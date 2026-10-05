#ifndef DEVICE_FACTORY_H
#define DEVICE_FACTORY_H

#include "RVConstants.h"

#include "Arduino.h"
#define CUSTOM_CHAR_HEADER  // this must be done prior to #include of HomeSpan call anywhere. 
#include <cstdint>
#include <mutex>
#include <map>
#include "CanFrameTypes.h"
#include "DGN.h"
#include "GenericDevice.h"
#include "ConfigTypes.h"
#include <ArduinoJson.h>
#include "debug.h"

class DeviceFactory {
    private:
        using DeviceCreator = std::function<GenericDevice*(const DeviceConfig&, const CoachSpec&)>;

        static DeviceFactory* instance;
        static uint32_t chassisAccessoryAid;
        static std::map<String, DeviceCreator> creators;   // devices.json "type" -> creator
        static std::map<RVC_DGN, std::map<uint8_t, GenericDevice*>> DGN2DeviceMap;

        static void registerCreators();
        static bool loadDeviceConfigs(const char* path, std::vector<DeviceConfig>& out);
        static void createFromDeviceConfig(const std::vector<DeviceConfig>& devices, const CoachSpec& coach);
        static bool assignStableAids(std::vector<DeviceConfig>& devices, uint32_t& chassisAid);
        static bool saveDeviceMetadata(const char* path, const std::vector<DeviceConfig>& devices);
        static bool saveChassisAid(const char* path, uint32_t chassisAid);

        void createDevices(void);

        DeviceFactory(void) {
            RV_PRINTF("DeviceFactory::DeviceFactory() started\n");
            createDevices();
            RV_PRINTF("DeviceFactory::DeviceFactory() completed\n");
        }
        DeviceFactory(const DeviceFactory&) = delete;
        DeviceFactory& operator=(const DeviceFactory&) = delete;
        DeviceFactory(DeviceFactory&&) = delete;
        DeviceFactory& operator=(DeviceFactory&&) = delete;
        ~DeviceFactory() = default;

    public:
        static DeviceFactory* getInstance(void);

        GenericDevice* getDeviceByData(RVC_DGN, uint8_t* data, uint8_t sourceAddress);

        static bool instanceFromData(RVC_DGN dgn, uint8_t* data, uint8_t& index);
        static bool loadCoachSpec(const char* path, CoachSpec& out);
        static bool validateAndReserveConfiguration(JsonDocument& document);
};
#endif
