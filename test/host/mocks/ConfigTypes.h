#pragma once
#include "Arduino.h"
struct CoachSpec {
    uint16_t year = 0;
    String make;
    String model;
    String floorplan;
    String coachId;
};

struct DeviceConfig {
    bool enabled = false;
    String type;
    uint32_t aid = 0;
};