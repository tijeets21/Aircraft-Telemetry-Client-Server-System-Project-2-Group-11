#include <iostream>
#include "ClientNetwork.h"
#include "FileReader.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: ClientProject.exe <Server_IP> <Port> <Telemetry_File.csv>\n";
        return 1;
    }

    std::string serverIp = argv[1];
    int port = std::stoi(argv[2]);
    std::string filePath = argv[3];

    // Initialize ID
    srand(static_cast<unsigned int>(time(0)) ^ GetCurrentProcessId());
    int aircraftID = rand() % 90000 + 10000;
    std::cout << "[INIT] Aircraft ID Assigned: " << aircraftID << "\n";

    ClientNetwork network;
    if (!network.Connect(serverIp, port)) {
        std::cerr << "[ERROR] Connection failed.\n";
        return 1;
    }
    std::cout << "[CONNECTED] Server joined.\n";

    FileReader reader;
    if (!reader.ProcessFile(filePath, aircraftID, network)) {
        std::cerr << "[ERROR] Failed to read file.\n";
    }

    std::cout << "[DISCONNECTING] Flight complete.\n";
    return 0;
}