#ifndef DEVICE_FACTORY_H
#define DEVICE_FACTORY_H
#include "Arduino.h"
#include <cstdint>
#include <mutex>
#include <map>
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "DGN.h"
#include "GenericDevice.h"


// Forward declaration of device types
class DeviceCommand;
// controllables

// HVAC
class AirConditioner;
class Furnace;
class HeatPump;
class Thermostat;

// Switches
class BinarySwitch;
class SlideSwitch;
class Fan;
class HeatedFloor;
class Awaning;
class Generator;

// Sensors
class Battery;
class Inverter;
class Charger;
class Tanks;  // gray, black, fresh
class ATS;  // automatic transfer switch
class ChassisMobility;



typedef enum {
    EX4551_2022 = 0, // 2022 Essex 4551
    DS4369_2019, // 2019 Dutch Star 4369
    KA4551_2022 // 2022 King Aire 4551    
} Coach;

class DeviceFactory {
    private:

        //static std::mutex deviceMutex;
        static DeviceFactory* instance;
        // static ChassisMobility* chassis;
        static boolean devicesCreated;

        // static std::map<uint8_t, GenericDevice*> iD2DeviceMap; // Map to hold devices by instance number
        static std::map<RVC_DGN, std::map<uint8_t, GenericDevice*>> DGN2DeviceMap; // Map of a map to hold devices by DGN number -> look up a devicd by DGN number and instance 
        
        DeviceFactory(const DeviceFactory&) = delete; // Prevent copy
        DeviceFactory& operator=(const DeviceFactory&) = delete; // Prevent assignment


        void createDevices(void);

        // Private constructor to enforce singleton pattern
        inline DeviceFactory(void) {
            // Initialization code if needed
            //std::lock_guard<std::mutex> lock(deviceMutex);
            printf("DeviceFactory::DeviceFactory() started\n");
            createDevices();
            printf("DeviceFactory::DeviceFactory() completed\n");
            //std::lock_guard<std::mutex> unlock(deviceMutex);
        }
        
        // Device creation methods for specific coaches
        // These methods will create devices based on the coach type
        // and populate the deviceMaps with the created devices. 
        void create2022Essex4551Devices(void);
        void create2019DutchStar4369Devices(void);
        void create2022KingAire4551Devices(void);
    public:
        // Singleton instance
        static DeviceFactory* getInstance(void); 

        static ChassisMobility* getChassis(void);

        static std::map<uint8_t, GenericDevice*> getDevice(RVC_DGN dgnNumber);

        GenericDevice* getDeviceByData(RVC_DGN, uint8_t* data);
        /**
            // Create a new device based on the DGN number
        DeviceCommand* device = createDeviceCommand(dgnNumber) {
            if (device) {
                    deviceMap[dgnNumber] = device;
                return *device;
            }
            
                // If no matching device found, return a null reference or throw an exception
            throw std::runtime_error("Device not found for DGN number: " + std::to_string(dgnNumber));
        }   
            */

};
#endif