#include "DeviceFactory.h"



constexpr Coach coach = EX4551_2022; // Default coach type"
// create the mutex
//std::mutex DeviceFactory::deviceMutex;
DeviceFactory* DeviceFactory::instance = nullptr;
boolean DeviceFactory::devicesCreated = false;
// std::map<uint8_t, GenericDevice*> DeviceFactory::iD2DeviceMap; // Map to hold devices by instance number
std::map<RVC_DGN, std::map<uint8_t, GenericDevice*>> DeviceFactory::DGN2DeviceMap; // Map of a map to hold devices by DGN number -> look up a devicd by DGN number and instance 


void DeviceFactory::createDevices(void) {
    // Create devices based on the coach type
    switch (coach) {
        case EX4551_2022:
            // Create devices for 2022 Essex 4551
            create2022Essex4551Devices();
            break;
        case DS4369_2019:
            // Create devices for 2019 Dutch Star 4369
            create2019DutchStar4369Devices();
            break;
        case KA4551_2022:
            // Create devices for 2022 King Aire 4551
            create2022KingAire4551Devices();
            break;
        default:
            throw std::runtime_error("Unknown coach type");

    }
    devicesCreated = true;
}

#include "DC_Switch.h"
#include "DC_DimmableSwitch.h"
#include "DC_LightSwitchView.h"
#include "DC_DimmableSwitchView.h"

#include "DoorLock.h"
#include "DoorLockView.h"


#include "ChassisMobility.h"
#include "WaterPump.h"
#include "WaterPumpView.h"
#include "Thermostat.h"
#include "ThermostatView.h"

void  DeviceFactory::create2022Essex4551Devices() {
    printf("DeviceFactory::create2022Essex4551Devices Start\n");
    // Create devices for 2022 Essex 4551
        // by id -> create the device and assign pointer in map by ID
        // by DGN - for each DGN to be used fo rthe device - assign a pointer to the DGN in the map -> not sure this will make sense yet
    // On/Off Lights
    // const uint8_t LIGHT_GROUP = 1;
    
    
    DC_DimmableSwitch* dimSwitch = new DC_DimmableSwitch(141, 82); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][82] = dimSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][82] = dimSwitch;
    DC_DimmableSwitchView::createDC_DimmableSwitchView(dimSwitch, "Living Room  82");

 
    dimSwitch = new DC_DimmableSwitch(141, 84); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][84] = dimSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][84] = dimSwitch;
    DC_DimmableSwitchView::createDC_DimmableSwitchView(dimSwitch, "Bedroom Slider 84");
    
    dimSwitch = new DC_DimmableSwitch(141, 85); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][85] = dimSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][85] = dimSwitch;
    DC_DimmableSwitchView::createDC_DimmableSwitchView(dimSwitch, "Master Bathroom 85");


    DC_Switch* dcSwitch = new DC_Switch(141, 98); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][98] = dcSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][98] = dcSwitch;
    DC_LightSwitchView::createDC_LightSwitchView(dcSwitch, "Driver Side Security Light 98");

    dcSwitch = new DC_Switch(141, 99);; // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][99] = dcSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][99] = dcSwitch;
    DC_LightSwitchView::createDC_LightSwitchView(dcSwitch, "Passenger Side Security Light 99");
    
    
    // doing only lights as first step

    // Air conditioners
    // Thermostats
    // std::list <RVC_DGN> thermostatDgns;
    // thermostatDgns.push_back(THERMOSTAT_COMMAND_1);
    // thermostatDgns.push_back(THERMOSTAT_COMMAND_2);
    // thermostatDgns.push_back(THERMOSTAT_STATUS_1);
    // thermostatDgns.push_back(THERMOSTAT_STATUS_2);
    // const uint8_t THERMOSTAT_GROUP = 4;

    // living room
    HVAC_Thermostat* thermostat = new HVAC_Thermostat(103, 1); // , THERMOSTAT_GROUP, thermostatDgns);
    DGN2DeviceMap[THERMOSTAT_COMMAND_1][1] = thermostat;
    DGN2DeviceMap[THERMOSTAT_COMMAND_2][1] = thermostat;
    DGN2DeviceMap[THERMOSTAT_STATUS_1][1] = thermostat;
    DGN2DeviceMap[THERMOSTAT_STATUS_2][1] = thermostat;
    ThermostatView::createThermostatView(thermostat, "Living Room Thermostat");

    thermostat = new HVAC_Thermostat(103, 2); // , THERMOSTAT_GROUP, thermostatDgns);
    DGN2DeviceMap[THERMOSTAT_COMMAND_1][2] = thermostat;
    DGN2DeviceMap[THERMOSTAT_COMMAND_2][2] = thermostat;
    DGN2DeviceMap[THERMOSTAT_STATUS_1][2] = thermostat;
    DGN2DeviceMap[THERMOSTAT_STATUS_2][2] = thermostat;
    ThermostatView::createThermostatView(thermostat, "Kitchen Thermostat");


    thermostat = new HVAC_Thermostat(103, 4); // , THERMOSTAT_GROUP, thermostatDgns);
    DGN2DeviceMap[THERMOSTAT_COMMAND_1][4] = thermostat;
    DGN2DeviceMap[THERMOSTAT_COMMAND_2][4] = thermostat;
    DGN2DeviceMap[THERMOSTAT_STATUS_1][4] = thermostat;
    DGN2DeviceMap[THERMOSTAT_STATUS_2][4] = thermostat;
    ThermostatView::createThermostatView(thermostat, "Bedroom Thermostat");
    // Furnace
    // HeatPump
    // Floor Heat
    // Awnings
    // shades
    // Door locks

    // std::list <RVC_DGN> lockDgns;
    // lockDgns.push_back(LOCK_COMMAND);
    // lockDgns.push_back(LOCK_STATUS);
    // const uint8_t LOCK_GROUP = 2;

    DoorLock* doorLock = new DoorLock(146, 1); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[LOCK_COMMAND][1] = doorLock;
    DGN2DeviceMap[LOCK_STATUS][1] = doorLock;
    DoorLockView::createDoorLockView(doorLock, "Front Door");

    doorLock = new DoorLock(146, 2); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[LOCK_COMMAND][2] = doorLock;
    DGN2DeviceMap[LOCK_STATUS][2] = doorLock;
    DoorLockView::createDoorLockView(doorLock, "Bay Door Grp 1");

    // Inverter
    // Generator
    // Batteries
    // water pump

    // std::list <RVC_DGN> waterPumpDgns;
    // waterPumpDgns.push_back(WATER_PUMP_COMMAND);
    // waterPumpDgns.push_back(WATER_PUMP_STATUS);
    // const uint8_t WATERPUMP_GROUP = 3;
    WaterPump* waterPump = new WaterPump(146); // , WATERPUMP_GROUP, waterPumpDgns);
    DGN2DeviceMap[WATER_PUMP_COMMAND][WATER_PUMP_INDEX] = waterPump;
    DGN2DeviceMap[WATER_PUMP_STATUS][WATER_PUMP_INDEX] = waterPump;
    WaterPumpView::createWaterPumpView(waterPump, "Water Pump");
    
    // Tanks
    // Roof Fans -- SilverLeaf does not expose the roof fans 
    // Power Devices

    //tmp = new DC_LightSwitchView(nullptr, "Essex 4551 Light Switch burned out");
    // DC_LightSwitchView::createDC_LightSwitchView(nullptr, "dummy switch");
    printf("2022 Essex 4552 devices created\n");
    printf("DeviceFactory::create2022Essex4551Devices End\n");
}

void DeviceFactory::create2019DutchStar4369Devices() {
    // Create devices for 2019 Dutch Star 4369
    // Implementation goes here
}   

void DeviceFactory::create2022KingAire4551Devices() {
    // Create devices for 2022 King Aire 4551
    // Implementation goes here
}   



DeviceFactory* DeviceFactory::getInstance() {
    // printf("DeviceFactory::getInstance Start\n");
    if (!DeviceFactory::instance) {
        DeviceFactory::devicesCreated = false;
        DeviceFactory::instance = new DeviceFactory();
        ChassisMobility* chassis = ChassisMobility::getInstance();
        const uint8_t defaultChassisIndex = DEFAULT_CHASSIS_INDEX;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_COMMAND][defaultChassisIndex] = chassis;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_STATUS][defaultChassisIndex] = chassis;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_STATUS_2][defaultChassisIndex] = chassis;
    }
    // printf("DeviceFactory::getInstance End\n");
    return DeviceFactory::instance;
}

ChassisMobility* DeviceFactory::getChassis(void) { 
    return ChassisMobility::getInstance();
}

std::map<uint8_t, GenericDevice*> DeviceFactory::getDevice(RVC_DGN dgnNumber) {
    std::map<uint8_t, GenericDevice* > result;
    std::map<uint8_t, GenericDevice* > it = DeviceFactory::DGN2DeviceMap[dgnNumber];
    if (!it.empty()) {
        result = it;
    }
    return result;
}


GenericDevice* DeviceFactory::getDeviceByData(RVC_DGN dgn, uint8_t* data) {
    GenericDevice* result = nullptr;
    if (data != nullptr) {
        if ((dgn != WATER_PUMP_COMMAND) && (dgn != WATER_PUMP_STATUS)) {
            uint8_t index = Packet::getIndex(data);
            result = DeviceFactory::DGN2DeviceMap[dgn][index]; 
        } else {
            result = DeviceFactory::DGN2DeviceMap[dgn][WATER_PUMP_INDEX]; 
        }
    }
    return result;
}
      
