#include "FuelCalculator.h"

void FuelCalculator::ProcessReading(int aircraftID, const char* timestamp, double fuelRemaining) {
    std::lock_guard<std::mutex> lock(dbMutex);
    FlightRecord& record = flightDatabase[aircraftID];

    if (record.initialFuel == -1.0) { record.initialFuel = fuelRemaining; }

    double usage = record.latestFuel != -1.0 ? (record.latestFuel - fuelRemaining) : 0;
    record.latestFuel = fuelRemaining;
    record.dataPoints++;

    std::cout << "[RECV] ID: " << aircraftID << " | Time: " << timestamp << " | Fuel: " << fuelRemaining << " | Used: " << usage << "\n";
}

void FuelCalculator::FinalizeFlight(int aircraftID) {
    std::lock_guard<std::mutex> lock(dbMutex);
    FlightRecord& record = flightDatabase[aircraftID];

    double fuelConsumed = record.initialFuel - record.latestFuel;
    double avgConsumption = record.dataPoints > 0 ? (fuelConsumed / record.dataPoints) : 0;

    std::cout << "\n========================================\n";
    std::cout << "[FLIGHT COMPLETE] Aircraft ID: " << aircraftID << "\n";
    std::cout << "Total Fuel Consumed: " << fuelConsumed << " gallons\n";
    std::cout << "Average Consumption/Tick: " << avgConsumption << " gallons\n";
    std::cout << "========================================\n\n";
}
