#pragma once
#include "Arduino.h"
struct CoachSpec {
    uint16_t year = 0;
    String make;
    String model;
    String floorplan;
    String coachId;
    uint32_t chassisAid = 0;
};

struct DeviceConfig {
    bool enabled = false;
    String type;
    uint32_t aid = 0;
    uint8_t rvcIndex = 0;
    uint8_t sourceAddress = 0;
};