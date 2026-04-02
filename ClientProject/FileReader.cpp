#include "FileReader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>

bool FileReader::ProcessFile(const std::string& filepath, int aircraftID, ClientNetwork& network) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string timeStr, fuelStr;

        if (std::getline(ss, timeStr, ',') && std::getline(ss, fuelStr, ',')) {
            TelemetryPacket packet;
            packet.aircraftID = aircraftID;
            strncpy_s(packet.timestamp, timeStr.c_str(), sizeof(packet.timestamp) - 1);
            try { packet.fuelRemaining = std::stod(fuelStr); }
            catch (...) { continue; } // Skip bad data
            packet.isEOF = false;

            network.SendPacket(packet);
            std::cout << "[SENT] Time: " << packet.timestamp << " | Fuel: " << packet.fuelRemaining << "\n";

            // CLT-FUNC-016: 1 second delay
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    file.close();

    // Send EOF
    TelemetryPacket eofPacket = { aircraftID, "", 0.0, true };
    network.SendPacket(eofPacket);
    return true;
}