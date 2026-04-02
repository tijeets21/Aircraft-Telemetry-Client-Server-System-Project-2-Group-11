#include "ClientNetwork.h"
#include <iostream>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

ClientNetwork::ClientNetwork() : clientSocket(INVALID_SOCKET) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

ClientNetwork::~ClientNetwork() { Disconnect(); WSACleanup(); }

bool ClientNetwork::Connect(const std::string& ip, int port) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool ClientNetwork::SendPacket(const TelemetryPacket& packet) {
    if (send(clientSocket, (char*)&packet, sizeof(packet), 0) == SOCKET_ERROR) return false;
    return true;
}

void ClientNetwork::Disconnect() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
}