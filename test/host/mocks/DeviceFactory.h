#pragma once
#include "ConfigTypes.h"
#include "LittleFS.h"
#include <ArduinoJson.h>
#include "DGN.h"
#include <vector>
#include <map>
#include "SupportedDeviceTypes.h"

class DeviceFactory {
public:
    static bool assignStableAids(std::vector<DeviceConfig>& devices, uint32_t& chassisAid);
    static bool validateAndReserveConfiguration(JsonDocument& document);
    static bool saveChassisAid(const char* path, uint32_t aid);
    static bool loadCoachSpec(const char* path, CoachSpec& coach) {
        bool result = false;
        File input = LittleFS.open(path, "r");
        if (input) {
            JsonDocument document;
            if (!deserializeJson(document, input) && document.is<JsonObject>()) {
                coach.year = document["year"] | 0;
                coach.make = document["make"] | "";
                coach.model = document["model"] | "";
                coach.floorplan = document["floorplan"] | "";
                coach.coachId = document["coachId"] | "";
                coach.chassisAid = document["chassisAid"] | 0U;
                result = true;
            }
        }
        return result;
    }
    static bool instanceFromData(RVC_DGN, uint8_t* data, uint8_t& instance) {
        bool valid = data != nullptr;
        if (valid) instance = data[0];
        return valid;
    }
private:
    inline static std::map<String, int> creators;
    static void registerCreators() {
        for (const char* type : supportedDeviceTypes) creators[type] = 1;
    }
};