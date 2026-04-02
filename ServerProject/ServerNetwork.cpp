
#include "ServerNetwork.h"
#include <thread>
#include <ws2tcpip.h>
#include "SharedPacket.h"

#pragma comment(lib, "ws2_32.lib")

ServerNetwork::ServerNetwork() : listenSocket(INVALID_SOCKET) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

ServerNetwork::~ServerNetwork() {
    if (listenSocket != INVALID_SOCKET) closesocket(listenSocket);
    WSACleanup();
}

bool ServerNetwork::Start(int port) {
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);
    std::cout << "[SERVER] Listening on port " << port << "...\n";
    return true;
}

void ServerNetwork::AcceptClients() {
    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket != INVALID_SOCKET) {
            std::thread clientThread(ClientHandlerThread, clientSocket, &fuelCalc);
            clientThread.detach(); // SYS-001: Parallel execution
        }
    }
}

void ServerNetwork::ClientHandlerThread(SOCKET clientSocket, FuelCalculator* calculator) {
    TelemetryPacket packet;
    while (true) {
        int bytes = recv(clientSocket, (char*)&packet, sizeof(packet), 0);
        if (bytes <= 0) break;

        if (packet.isEOF) {
            calculator->FinalizeFlight(packet.aircraftID);
            break;
        }
        else {
            calculator->ProcessReading(packet.aircraftID, packet.timestamp, packet.fuelRemaining);
        }
    }
    closesocket(clientSocket);
}
