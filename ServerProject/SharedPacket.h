#pragma once
#pragma once
#pragma pack(push, 1)

struct TelemetryPacket {
    int aircraftID;
    char timestamp[32];
    double fuelRemaining;
    bool isEOF;
};

#pragma pack(pop)
