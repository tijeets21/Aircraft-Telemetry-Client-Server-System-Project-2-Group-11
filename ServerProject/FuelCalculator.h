#pragma once

/**
 * @file FuelCalculator.h
 * @brief Manages flight tracking, data storage, and fuel calculations.
 */
#pragma once
#include <map>
#include <mutex>
#include <iostream>

class FuelCalculator {
private:
    struct FlightRecord {
        double initialFuel = -1.0;
        double latestFuel = -1.0;
        int dataPoints = 0;
    };
    std::map<int, FlightRecord> flightDatabase;
    std::mutex dbMutex;

public:
    void ProcessReading(int aircraftID, const char* timestamp, double fuelRemaining);
    void FinalizeFlight(int aircraftID);
};
