/**
 * @file ClientNetwork.h
 * @brief Handles connection and data transmission to the server.
 */
#pragma once
#include <string>
#include <winsock2.h>
#include "SharedPacket.h"

class ClientNetwork {
private:
    SOCKET clientSocket;
public:
    ClientNetwork();
    ~ClientNetwork();
    bool Connect(const std::string& ip, int port);
    bool SendPacket(const TelemetryPacket& packet);
    void Disconnect();
};