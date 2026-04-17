/**
 * @file ServerNetwork.cpp
 * @brief Implements the ServerNetwork class for accepting and handling client connections.
 *
 * Contains the TCP server lifecycle (bind, listen, accept) and the per-client
 * handler thread that receives TelemetryPacket structs and forwards them to
 * the FuelCalculator. The ws2_32.lib library is linked automatically via
 * the #pragma comment below.
 *
 * @see ServerNetwork.h for the class interface and member documentation.
 */

#include "ServerNetwork.h"
#include <thread>
#include <ws2tcpip.h>
#include "SharedPacket.h"

 /// @brief Instructs the MSVC linker to automatically link the Winsock2 library.
#pragma comment(lib, "ws2_32.lib")

/**
 * @brief Constructs a ServerNetwork and initialises the Winsock2 library.
 *
 * Calls WSAStartup(MAKEWORD(2, 2), &wsaData) to prepare the Winsock DLL
 * for use. Sets listenSocket to INVALID_SOCKET so the destructor can
 * safely detect whether a socket was ever created.
 *
 * @note WSAStartup errors are not currently checked. A production system
 *       should validate the return value and signal failure to the caller.
 */
ServerNetwork::ServerNetwork() : listenSocket(INVALID_SOCKET) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

/**
 * @brief Destroys the ServerNetwork, releasing the listening socket and Winsock.
 *
 * Closes listenSocket if valid. Does not close any active client sockets,
 * as ownership of those was transferred to detached handler threads in
 * AcceptClients(). Calls WSACleanup() to release the Winsock DLL.
 */
ServerNetwork::~ServerNetwork() {
    if (listenSocket != INVALID_SOCKET) closesocket(listenSocket);
    WSACleanup();
}

/**
 * @brief Creates, binds, and starts listening on the specified TCP port.
 *
 * Follows the standard passive-socket setup sequence:
 * - socket(AF_INET, SOCK_STREAM, 0) — creates an IPv4 TCP socket.
 * - bind() with INADDR_ANY — accepts connections on all local network interfaces.
 * - listen(SOMAXCONN) — places the socket in the listening state with the
 *   system's maximum allowed backlog of pending connections.
 *
 * @param port The TCP port to bind to, in host byte order. Converted internally
 *             to network byte order via htons().
 *
 * @return @c true  if socket(), bind(), and listen() all succeed.
 * @return @c false if any of the three calls returns SOCKET_ERROR.
 */
bool ServerNetwork::Start(int port) {
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) return false;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept on all available interfaces
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) return false;
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) return false;

    std::cout << "[SERVER] Listening on port " << port << "...\n";
    return true;
}

/**
 * @brief Enters an infinite loop accepting clients and spawning handler threads.
 *
 * For each successful accept() call, a new std::thread is created with
 * ClientHandlerThread as its entry point, receiving the new client socket
 * and a pointer to the shared FuelCalculator. The thread is immediately
 * detached so the accept loop is never blocked waiting for a client to finish.
 *
 * Invalid sockets (i.e., INVALID_SOCKET returned by accept() on transient
 * errors) are silently skipped; the loop continues to the next iteration.
 *
 * @note This function runs indefinitely. The server process must be terminated
 *       externally (e.g., SIGINT / Ctrl+C) to stop it.
 */
void ServerNetwork::AcceptClients() {
    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);

        if (clientSocket != INVALID_SOCKET) {
            std::cout << "[INFO] New client connected. Spawning thread..." << std::endl;
            // Create a handler thread for this client and detach it immediately.
            // The thread owns the clientSocket and is responsible for closing it.
            std::thread clientThread(ClientHandlerThread, clientSocket, &fuelCalc);
            clientThread.detach();
        }
    }
}

/**
 * @brief Static thread function that manages a single client connection from start to finish.
 *
 * This is the entry point for each per-client std::thread. It handles the
 * full receive loop for one connected aircraft client:
 *
 * ### Setup:
 * - Sets a 10-second receive timeout (SO_RCVTIMEO) on @p clientSocket using
 *   setsockopt. If no data arrives within 10 seconds, recv() will return
 *   with error code WSAETIMEDOUT (10060), triggering the cleanup path.
 *
 * ### Receive loop (while (true)):
 * Each iteration calls recv() attempting to read exactly sizeof(TelemetryPacket)
 * bytes into a local TelemetryPacket buffer.
 *
 * - *bytes > 0*: A packet was received.
 *   - Records the aircraft ID from the first packet seen (used for cleanup
 *     if the client later crashes).
 *   - If packet.isEOF == true: calls FuelCalculator::FinalizeFlight() and
 *     exits the loop (normal termination path).
 *   - If packet.isEOF == false: calls FuelCalculator::ProcessReading() to
 *     record the telemetry data point.
 *
 * - *bytes == 0*: The client closed its TCP connection gracefully without
 *   sending an EOF packet (unusual but possible). Logs a warning and invokes
 *   CleanupCrashedFlight() if an aircraft ID was previously seen.
 *
 * - *bytes < 0* (i.e., SOCKET_ERROR): A Winsock error occurred, most
 *   commonly WSAETIMEDOUT (10060) due to the 10-second inactivity timeout.
 *   Logs the WSA error code and invokes CleanupCrashedFlight().
 *
 * ### Teardown:
 * After breaking from the loop, the function closes @p clientSocket and exits,
 * terminating the thread.
 *
 * @param clientSocket The accepted TCP socket for the client. This thread takes
 *                     full ownership; it must call closesocket() before returning.
 * @param calculator   Non-owning pointer to the shared FuelCalculator. Guaranteed
 *                     to be valid for the server's lifetime (which outlasts all threads).
 */
void ServerNetwork::ClientHandlerThread(SOCKET clientSocket, FuelCalculator* calculator) {
    // --- DEBUG: Confirm the thread has started ---
    std::cout << "--- NEW CLIENT THREAD HAS STARTED ---" << std::endl;

    // Set a 10-second receive timeout. If recv() blocks for longer than this
    // without receiving data, it will return SOCKET_ERROR with WSAGetLastError()
    // == WSAETIMEDOUT (10060), allowing the server to clean up stalled clients.
    DWORD timeout = 10000; // milliseconds
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    TelemetryPacket packet;
    int aircraftIdForThisThread = -1; ///< Sentinel: -1 means no packet received yet.

    // --- DEBUG: Confirm we are about to block on recv() ---
    std::cout << "--- THREAD IS ENTERING RECV LOOP, WAITING FOR DATA... ---" << std::endl;

    while (true) {
        // Block until sizeof(TelemetryPacket) bytes are available, or until the
        // timeout/error condition above is triggered.
        int bytes = recv(clientSocket, (char*)&packet, sizeof(packet), 0);

        // --- DEBUG: Log the raw return value of recv() for diagnostics ---
        std::cout << "--- RECV() RETURNED A VALUE: " << bytes << " ---" << std::endl;
        if (bytes == -1) {
            // Print the Winsock error code to identify the exact failure reason.
            // Common codes: 10060 = WSAETIMEDOUT, 10054 = WSAECONNRESET.
            std::cout << "--- WINSOCK ERROR CODE: " << WSAGetLastError() << " ---" << std::endl;
        }

        if (bytes <= 0) {
            // bytes == 0: client closed the connection (TCP FIN) without an EOF packet.
            // bytes  < 0: recv() returned SOCKET_ERROR (timeout, reset, etc.).
            std::cout << "[WARN] Client timed out or disconnected. ID: " << aircraftIdForThisThread << std::endl;
            if (aircraftIdForThisThread != -1) {
                // Only clean up if we received at least one packet (and therefore
                // created a FlightRecord). If no packet was seen, there is nothing to erase.
                calculator->CleanupCrashedFlight(aircraftIdForThisThread);
            }
            break;
        }

        // Record the aircraft ID from the first packet to enable cleanup on crash.
        if (aircraftIdForThisThread == -1) {
            aircraftIdForThisThread = packet.aircraftID;
        }

        if (packet.isEOF) {
            // Normal termination path: client has finished sending all data.
            // FinalizeFlight prints the summary and erases the record.
            calculator->FinalizeFlight(packet.aircraftID);
            break;
        }
        else {
            // Normal data packet: update the flight record and log the reading.
            calculator->ProcessReading(packet.aircraftID, packet.timestamp, packet.fuelRemaining);
        }
    }

    std::cout << "--- CLIENT THREAD IS EXITING ---" << std::endl;
    closesocket(clientSocket); // Release the socket back to the OS.
}
