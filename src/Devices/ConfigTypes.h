#pragma once
#include <Arduino.h>
#include <vector>
#include <map>
#include <functional>
#include "DGN.h"
#include "GenericDevice.h"

// ------------ Coach / Floorplan specific configuration types ---------------
struct TankSpec {
    uint8_t instance = 0;
    uint16_t capacityLiters = 0;
    String name;
};

struct BatterySpec {
    uint8_t instance = 0;
    float nominalVoltage = 12.0f; // default to 12V
    float capacityAh = 0.0f;
    String name;
};

struct CoverTiming {
    uint16_t extendSec  = 40;
    uint16_t retractSec = 45;

    CoverTiming() = default;
    CoverTiming(const CoverTiming&) = default;
    CoverTiming& operator=(const CoverTiming&) = default;
    ~CoverTiming() = default;
};

struct CoverTimingPair {
    String       key;
    CoverTiming  timing;

    CoverTimingPair() = default;
    CoverTimingPair(const CoverTimingPair&) = default;
    CoverTimingPair& operator=(const CoverTimingPair&) = default;
    ~CoverTimingPair() = default;
};

struct CoachSpec {
    uint16_t year = 2022;
    String   make;
    String   model;
    String   floorplan;
    String   coachId;
    uint32_t chassisAid = 0;
    // std::map<String, TankSpec>     tanks; // key: tank type (fresh, gray, black)
    std::vector<TankSpec>       tanks;
    //std::map<String, BatterySpec>  batteries; // key: battery type (house, chassis)
    std::vector<BatterySpec>    batteries;
    // std::map<String, CoverTiming> awningTimes; // key: awning position (front, rear)
    std::vector<CoverTimingPair> coverTimings;

    // convenience methods to get tank, battery, and awning timing by type
    // const TankSpec* getTank(const String& tankType) const {
    const TankSpec* getTank(uint8_t instance) const {
        // auto it = tanks.find(tankType);
        // return it != tanks.end() ? &it->second : nullptr; // return default if not found
        const TankSpec* result = nullptr;
        for (const auto& t : tanks) {
            if (t.instance == instance)  {
                result = &t;
                break;
            }
        }
        return result;
    }

    const BatterySpec* getBattery(uint8_t instance) const {
        const BatterySpec* result = nullptr;
        for (const auto& t : batteries) {
            if (t.instance == instance) {
                result = &t;
                break;
            }
        }
        return result;
    }

    bool findCoverTiming(const String& key, CoverTiming& out) const {
        bool found = false;

        for (size_t i = 0; i < coverTimings.size(); ++i) {
            if (coverTimings[i].key == key) {
                out = coverTimings[i].timing;
                found = true;
                break;
            }
        }
        return found;
    }

    // const BatterySpec* getBattery(const String& batteryType) const {
    //     auto it = batteries.find(batteryType);
    //     return it != batteries.end() ? &it->second : nullptr; // return default if not found
    // }

    // const CoverTiming* getCoverTiming(const String& position) const {
    //     auto it = awningTimes.find(position);
    //    return it != awningTimes.end() ? &it->second : nullptr; // return default if not found
    // }

};

struct ExtraPair {
    String key;
    String value;

    ExtraPair() = default;
    ExtraPair(const ExtraPair&) = default;
    ExtraPair& operator=(const ExtraPair&) = default;
    ~ExtraPair() = default;
};

// ------------- End of Coach / Floorplan specific configuration types ---------------
// -------------- Device configuration types ---------------
struct DeviceConfig {
    bool        enabled = false;
    String      type;               // e.g., "Thermostat", "WaterPump", "Cover", etc.
    uint8_t     rvcIndex = 0;       // instance index for the device
    uint8_t     sourceAddress = 0;  // RVC source address for the device
    String      name;               // Optional name for the device
    uint32_t    aid = 0;             // Stable starting AID; multi-accessory views use following IDs
    String      room;                // Suggested HomeKit room; Apple Home owns actual room assignment
    uint16_t    order         = 100;
    // free form extras, (awning timers, tank kind, etc)
    // std::map<String, String> extra; // key-value pairs for additional configuration
    std::vector<ExtraPair> extras;
    DeviceConfig() = default;
    DeviceConfig(const DeviceConfig&) = default;
    DeviceConfig& operator=(const DeviceConfig&) = default;
    ~DeviceConfig() = default;

    // domain behaviour for reading extras (not public attribute exposure of the vector)
    uint32_t extraUInt(const char* key, uint32_t defaultValue) const {
        uint32_t result = defaultValue;
        for (size_t i = 0; i < extras.size(); ++i) {
            if (extras[i].key == key) {
                result = (uint32_t)extras[i].value.toInt();
                break;
            }
        }
        return result;
    }

    String extraString(const char* key, const String& defaultValue) const {
        String result = defaultValue;
        for (size_t i = 0; i < extras.size(); ++i) {
            if (extras[i].key == key) {
                result = extras[i].value;
                break;
            }
        }
        return result;
    }
    // helpers
    /**
    uint32_t getExtraUInt(const char* key, uint32_t def = 0) const {
        auto it = extra.find(key);
        uint32_t result = def;
        if (it != extra.end()) 
            result = static_cast<uint32_t>(it->second.toInt());
        return result;
    }

    String getExtraString(const char* key, const String& def = "") const {
        auto it = extra.find(key);
        String result = def;
        if (it != extra.end()) 
            result = it->second;
        return result;
    }
    
    uint16_t getExtraUInt(const String& key, uint16_t defaultValue = 0) const {
        auto it = extra.find(key);
        return it != extra.end() ? static_cast<uint16_t>(atoi(it->second.c_str())) : defaultValue;
    }

    String getExtraString(const String& key, const String& defaultValue = "") const {
        auto it = extra.find(key);
        return it != extra.end() ? it->second : defaultValue;
    }
    */
};
