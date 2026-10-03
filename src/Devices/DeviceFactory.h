#ifndef DEVICE_FACTORY_H
#define DEVICE_FACTORY_H

#include "RVConstants.h"

#include "Arduino.h"
#define CUSTOM_CHAR_HEADER  // this must be done prior to #include of HomeSpan call anywhere. 
#include <cstdint>
#include <mutex>
#include <map>
#include "CanFrameTypes.h"
// #include "ESP32CAN.h"
// #include "CAN_config.h"
#include "DGN.h"
#include "GenericDevice.h"
#include "ConfigTypes.h"
#include "debug.h"



// Forward declaration of device types
//class DeviceCommand;
// controllables

// HVAC
//class AirConditioner;
//class Furnace;
//class HeatPump;
class Thermostat;

// Switches
//class HeatedFloor;
// class Awning;
class Generator;

// Sensors
class Battery;
class Inverter;
class Charger;
class Tanks;  // gray, black, fresh
class AutomaticTransferSwitch;  // automatic transfer switch
class ChassisMobility;



typedef enum {
    EX4551_2022 = 0, // 2022 Essex 4551
    DS4369_2019, // 2019 Dutch Star 4369
    KA4551_2022 // 2022 King Aire 4551    
} Coach;

using DeviceCreatorFn = GenericDevice* (*)(const DeviceConfig& cfg, const CoachSpec& coach);
struct CreatorEntry {
    const char*     typeName;   // must match devices.json "type"
    DeviceCreatorFn create;
};



class DeviceFactory {
    private:

        static DeviceFactory* instance;
        static boolean devicesCreated;
        static const char*   fileName;
        static CoachSpec     coachSpec; // Store the loaded coach specification

#ifdef SMART_COACH_ESP32S3
        using DeviceCreator = std::function<GenericDevice*(const DeviceConfig&, const CoachSpec&)>;

        static std::map<String, DeviceCreator> creators; // Map of device type to creation function
        static void registerCreators(); // Function to register device creators
#endif
        static bool loadCoachSpec(const char* path, CoachSpec& out);        
        static bool loadDeviceConfigs(const char* path, std::vector<DeviceConfig>& out);
        static void createFromDeviceConfig(const std::vector<DeviceConfig>& devices, const CoachSpec& coach);

#ifdef SMART_COACH_ESP32
        // ---- forward declarations of creators ----
        static GenericDevice* createDimmable(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createLight(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createDoorLock(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createThermostat(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createCover(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createShades(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createTank(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createWaterPump(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createBattery(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createInverter(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createGenerator(const DeviceConfig&, const CoachSpec&);
        // static GenericDevice* createCharger(const DeviceConfig&, const CoachSpec&);
        static GenericDevice* createATS(const DeviceConfig&, const CoachSpec&);
        static const CreatorEntry kCreatorTable[];
        static DeviceCreatorFn findCreator(const char* typeName);
#endif

        static std::map<RVC_DGN, std::map<uint8_t, GenericDevice*>> DGN2DeviceMap; // Map of a map to hold devices by DGN number -> look up a devicd by DGN number and instance 
        
        DeviceFactory(const DeviceFactory&) = delete; // Prevent copy
        DeviceFactory& operator=(const DeviceFactory&) = delete; // Prevent assignment


        void createDevices(void);

        // Private constructor to enforce singleton pattern
        inline DeviceFactory(void) {
            // Initialization code if needed
            RV_PRINTF("DeviceFactory::DeviceFactory() started\n");
            createDevices();
            RV_PRINTF("DeviceFactory::DeviceFactory() completed\n");
        }
        
        // Device creation methods for specific coaches
        // These methods will create devices based on the coach type
        // and populate the deviceMaps with the created devices. 
        #ifdef HOME_KIT_3
        void create2022Essex4551Devices(void);
        void create2019DutchStar4369Devices(void);
        void create2022KingAire4551Devices(void);
        #endif
        boolean createFromConfigFile(void);
    public:
        // Singleton instance
        static DeviceFactory* getInstance(void); 

        static ChassisMobility* getChassis(void);

        static std::map<uint8_t, GenericDevice*> getDevice(RVC_DGN dgnNumber);

        GenericDevice* getDeviceByData(RVC_DGN, uint8_t* data);

};
#endif
