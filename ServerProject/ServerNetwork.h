
#pragma once
/**
 * @file ServerNetwork.h
 * @brief Handles listening for clients and spawning handler threads.
 */
#pragma once
#include <winsock2.h>
#include "FuelCalculator.h"

class ServerNetwork {
private:
    SOCKET listenSocket;
    FuelCalculator fuelCalc;
    static void ClientHandlerThread(SOCKET clientSocket, FuelCalculator* calculator);

public:
    ServerNetwork();
    ~ServerNetwork();
    bool Start(int port);
    void AcceptClients();
};
