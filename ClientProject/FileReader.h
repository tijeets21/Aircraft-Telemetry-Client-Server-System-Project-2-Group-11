/**
 * @file FileReader.h
 * @brief Reads telemetry files and packetizes the data.
 */
#pragma once
#include <string>
#include "ClientNetwork.h"

class FileReader {
public:
    /**
     * @brief Processes the CSV file and sends data over the network.
     */
    bool ProcessFile(const std::string& filepath, int aircraftID, ClientNetwork& network);
};