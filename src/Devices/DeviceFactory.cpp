#include "Arduino.h"
#define CUSTOM_CHAR_HEADER
#include "HomeSpan.h"
#include "DeviceFactory.h"
#include "RVConstants.h"


constexpr Coach coach = EX4551_2022; // Default coach type"
const char* DeviceFactory::fileName = "data/config.rv";

DeviceFactory* DeviceFactory::instance = nullptr;
boolean DeviceFactory::devicesCreated = false;
// std::map<uint8_t, GenericDevice*> DeviceFactory::iD2DeviceMap; // Map to hold devices by instance number
std::map<RVC_DGN, std::map<uint8_t, GenericDevice*>> DeviceFactory::DGN2DeviceMap; // Map of a map to hold devices by DGN number -> look up a devicd by DGN number and instance 


void DeviceFactory::createDevices(void) {
    // Create devices based on the coach type
    // if (!createFromConfigFile()) {
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
    // }
    devicesCreated = true;
}

#include "ChassisMobility.h"

#ifdef HOME_KIT_1
#include "DC_Switch.h"
#include "DC_DimmableSwitch.h"
#include "DC_LightSwitchView.h"
#include "DC_DimmableSwitchView.h"

#include "DoorLock.h"
#include "DoorLockView.h"


#include "WaterPump.h"
#include "WaterPumpView.h"
#include "Thermostat.h"
#include "ThermostatView.h"
#endif


#ifdef HOME_KIT_2
#include "Generator.h"
#include "GeneratorView.h"
#include "Tanks.h"
#include "TanksView.h"
#include "AutomaticTransferSwitch.h"
#include "AutomaticTransferSwitchView.h"
#include "Battery.h"
#include "BatteryView.h"
#include "Inverter.h"
#include "InverterView.h"
#include "FloorHeat.h"
#include "FloorHeatView.h"
// #include "PowerSensorView.h"
#include "Awning.h" // can't implement - AWNING_STATUS does not report position properly so can't get position properly managed in loop
#include "AwningView.h"
#endif

void  DeviceFactory::create2022Essex4551Devices() {
    printf("DeviceFactory::create2022Essex4551Devices Start\n");
    // Create devices for 2022 Essex 4551
        // by id -> create the device and assign pointer in map by ID
        // by DGN - for each DGN to be used fo rthe device - assign a pointer to the DGN in the map -> not sure this will make sense yet
    // On/Off Lights
    // const uint8_t LIGHT_GROUP = 1;
#ifdef HOME_KIT_1
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

#endif

#ifdef HOME_KIT_2
    // Floor Heat
    
    FloorHeat* floorHeat = new FloorHeat(97, 1);
    DGN2DeviceMap[FLOOR_HEAT_COMMAND][1] = floorHeat;
    DGN2DeviceMap[FLOOR_HEAT_STATUS][1] = floorHeat;
    FloorHeatView::createFloorHeatView(floorHeat, "Living Room Floor Heat");

    floorHeat = new FloorHeat(97, 2);
    DGN2DeviceMap[FLOOR_HEAT_COMMAND][2] = floorHeat;
    DGN2DeviceMap[FLOOR_HEAT_STATUS][2] = floorHeat;
    FloorHeatView::createFloorHeatView(floorHeat, "Kitchen Floor Heat");
  
    floorHeat = new FloorHeat(97, 3);
    DGN2DeviceMap[FLOOR_HEAT_COMMAND][3] = floorHeat;
    DGN2DeviceMap[FLOOR_HEAT_STATUS][3] = floorHeat;
    FloorHeatView::createFloorHeatView(floorHeat, "Bed n Bath Floor Heat");
    // Awnings 
    /* more work to debug and work on timing
    Awning* awning = new Awning(136, FRONT_AWNING); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[AWNING_COMMAND][FRONT_AWNING] = awning;
    DGN2DeviceMap[AWNING_STATUS][FRONT_AWNING] = awning;
    AwningView::createAwningView(awning, FRONT_AWNING_NAME, FRONT_AWNING_EXTEND_CYCLE_TIME_SEC, FRONT_AWNING_RETRACT_CYCLE_TIME_SEC);
    awning = new Awning(136, BACK_AWNING); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[AWNING_COMMAND][BACK_AWNING] = awning;
    DGN2DeviceMap[AWNING_STATUS][BACK_AWNING] = awning;
    AwningView::createAwningView(awning, BACK_AWNING_NAME, BACK_AWNING_EXTEND_CYCLE_TIME_SEC, BACK_AWNING_RETRACT_CYCLE_TIME_SEC);
    awning = new Awning(136, DOOR_AWNING); 
    DGN2DeviceMap[AWNING_COMMAND][DOOR_AWNING] = awning;
    DGN2DeviceMap[AWNING_STATUS][DOOR_AWNING] = awning;
    AwningView::createAwningView(awning, DOOR_AWNING_NAME, DOOR_AWNING_EXTEND_CYCLE_TIME_SEC, DOOR_AWNING_RETRACT_CYCLE_TIME_SEC);
    */
    // shades
#endif
    // Door locks

    // std::list <RVC_DGN> lockDgns;
    // lockDgns.push_back(LOCK_COMMAND);
    // lockDgns.push_back(LOCK_STATUS);
    // const uint8_t LOCK_GROUP = 2;
#ifdef HOME_KIT_1
    DoorLock* doorLock = new DoorLock(146, 1); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[LOCK_COMMAND][1] = doorLock;
    DGN2DeviceMap[LOCK_STATUS][1] = doorLock;
    DoorLockView::createDoorLockView(doorLock, "Front Door");

    doorLock = new DoorLock(146, 2); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[LOCK_COMMAND][2] = doorLock;
    DGN2DeviceMap[LOCK_STATUS][2] = doorLock;
    DoorLockView::createDoorLockView(doorLock, "Bay Door Grp 1");
#endif

#ifdef HOME_KIT_2
    // Inverter 1
    Inverter* inverter = new Inverter(250, 1);
    DGN2DeviceMap[INVERTER_AC_STATUS_1][1] = inverter;
    DGN2DeviceMap[INVERTER_STATUS][1] = inverter;
    InverterView::createInverterView(inverter, "Inverter 1");
    // Inverter 2
    inverter = new Inverter(250, 2);
    DGN2DeviceMap[INVERTER_AC_STATUS_1][2] = inverter;
    DGN2DeviceMap[INVERTER_STATUS][2] = inverter;
    InverterView::createInverterView(inverter, "Inverter 2");

    // Generator
    Generator* generator = new Generator(250, 1); 
    DGN2DeviceMap[GENERATOR_AC_STATUS_1][1] = (PowerSensor* )generator;
    GeneratorView::createGeneratorView((GenericDevice* )generator, "Generator");

    // ATS
    AutomaticTransferSwitch* ats = new AutomaticTransferSwitch(250, 1);
    DGN2DeviceMap[ATS_AC_STATUS_1][1] = (AutomaticTransferSwitch* ) ats;
    DGN2DeviceMap[ATS_AC_STATUS_2][1] = (AutomaticTransferSwitch* ) ats;
    DGN2DeviceMap[ATS_AC_STATUS_3][1] = (AutomaticTransferSwitch* ) ats;
    DGN2DeviceMap[ATS_AC_STATUS_4][1] = (AutomaticTransferSwitch* ) ats;
    AutomaticTransferSwitchView::createAutomaticTransferSwitchView(ats, "ATS");

    // Batteries
    Battery* battery = new Battery(250, MAIN_HOUSE_BATTERY, MULTI_METER); 
    DGN2DeviceMap[DC_SOURCE_STATUS_1][MAIN_HOUSE_BATTERY] = (PowerSensor* )battery;
    DGN2DeviceMap[DC_SOURCE_STATUS_2][MAIN_HOUSE_BATTERY] = (PowerSensor* )battery;
    DGN2DeviceMap[DC_SOURCE_STATUS_3][MAIN_HOUSE_BATTERY] = (PowerSensor* )battery;
    DGN2DeviceMap[DC_SOURCE_STATUS_4][MAIN_HOUSE_BATTERY] = (PowerSensor* )battery;
    BatteryView::createBatteryView((GenericDevice* )battery, "House Battery Bank");

#endif
    // water pump

    // std::list <RVC_DGN> waterPumpDgns;
    // waterPumpDgns.push_back(WATER_PUMP_COMMAND);
    // waterPumpDgns.push_back(WATER_PUMP_STATUS);
    // const uint8_t WATERPUMP_GROUP = 3;
#ifdef HOME_KIT_1
    WaterPump* waterPump = new WaterPump(146); // , WATERPUMP_GROUP, waterPumpDgns);
    DGN2DeviceMap[WATER_PUMP_COMMAND][WATER_PUMP_INDEX] = waterPump;
    DGN2DeviceMap[WATER_PUMP_STATUS][WATER_PUMP_INDEX] = waterPump;
    WaterPumpView::createWaterPumpView(waterPump, "Water Pump");
#endif
    // Tanks
#ifdef HOME_KIT_2
    Tanks* tank = new Tanks(250, Tanks::FRESH_WATER_INSTANCE); 
    DGN2DeviceMap[TANK_STATUS][Tanks::FRESH_WATER_INSTANCE] = tank;
    TanksView::createTanksView((GenericDevice* )tank, "Fresh Water");  

    tank = new Tanks(250, Tanks::GRAY_WATER_INSTANCE); 
    DGN2DeviceMap[TANK_STATUS][Tanks::GRAY_WATER_INSTANCE] = tank;
    TanksView::createTanksView((GenericDevice* )tank, "Gray Water");  

    tank = new Tanks(250, Tanks::BLACK_WATER_INSTANCE); 
    DGN2DeviceMap[TANK_STATUS][Tanks::BLACK_WATER_INSTANCE] = tank;
    TanksView::createTanksView((GenericDevice* )tank, "Black Water");  
#endif 

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


#include "ChassisMobilityView.h"
DeviceFactory* DeviceFactory::getInstance() {
    // printf("DeviceFactory::getInstance Start\n");
    if (!DeviceFactory::instance) {
        DeviceFactory::devicesCreated = false;
        DeviceFactory::instance = new DeviceFactory();
#ifdef HOME_KIT_1
        ChassisMobility* chassis = ChassisMobility::getInstance();
        const uint8_t defaultChassisIndex = DEFAULT_CHASSIS_INDEX;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_COMMAND][defaultChassisIndex] = chassis;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_STATUS][defaultChassisIndex] = chassis;
        instance->DGN2DeviceMap[CHASSIS_MOBILITY_STATUS_2][defaultChassisIndex] = chassis;
        ChassisMobilityView::createChassisMobilityView((GenericDevice* )chassis, "Chassis Mobility Sensor");
#endif
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

#include "AutomaticTransferSwitch.h"
#include "Generator.h"

GenericDevice* DeviceFactory::getDeviceByData(RVC_DGN dgn, uint8_t* data) {
    GenericDevice* result = nullptr;
    if (data != nullptr) {
        if ((dgn != WATER_PUMP_COMMAND) && (dgn != WATER_PUMP_STATUS) && 
            (dgn != ATS_AC_STATUS_1) && (dgn != ATS_AC_STATUS_2) && (dgn != ATS_AC_STATUS_3) && (dgn != ATS_AC_STATUS_4) &&
            (dgn != GENERATOR_AC_STATUS_1) && (dgn != GENERATOR_AC_STATUS_2) && (dgn != GENERATOR_AC_STATUS_3) && (dgn != GENERATOR_AC_STATUS_4) &&
            (dgn != INVERTER_AC_STATUS_1) && (dgn != INVERTER_STATUS)) {
            uint8_t index = Packet::getIndex(data);
            result = DeviceFactory::DGN2DeviceMap[dgn][index];  
        } else if ((dgn == ATS_AC_STATUS_1) || (dgn == ATS_AC_STATUS_2) || (dgn == ATS_AC_STATUS_3) || (dgn == ATS_AC_STATUS_4)) { // need to get the index differently for ATS_AC_STATUS data
#ifdef HOME_KIT_2
            // need to deal with this differently per the RVC Spec on Byte 0
            uint8_t index = ATS_INSTANCE_0_INVALID; // invalid
            uint8_t tmp = data[AutomaticTransferSwitch::ATS_BYTE_0] & AutomaticTransferSwitch::ATS_STATUS_INDEX_MASK;
            if ((tmp > ATS_INSTANCE_0_INVALID) && (tmp < ATS_INSTANCE_7_INVALID)) {
                index = tmp;
                result = DeviceFactory::DGN2DeviceMap[dgn][index];
            }
#endif  
            ; // do nothing
        } else if ((dgn == INVERTER_AC_STATUS_1) || (dgn == INVERTER_STATUS)) {
#ifdef HOME_KIT_2
            // need to deal with this differently per the RVC Spec on Byte 0
            uint8_t index = INVERTER_INVALID; // invalid
            if (dgn == INVERTER_AC_STATUS_1) {
                index = data[INVERTER_LINE_INDEX] & INVERTER_INSTANCE_MASK;
                // if (index > 1) // line 2 is invalid for this DGN
                //    printf("DeviceFactory::getDeviceByData INVERTER_AC_STATUS_1 index %d\n", index);
            }  else if (dgn == INVERTER_STATUS) {
                index = data[INVERTER_LINE_INDEX];
                // if (index > 1)
                //    printf("DeviceFactory::getDeviceByData INVERTER_STATUS index %d\n", index);
            }
            // uint8_t tmp = data[Inverter::] & Inverter::INVERTER_STATUS_INDEX_MASK;
            if ((index != INVERTER_INVALID)) { //  && (tmp < INVERTER_INSTANCE_4_INVALID)) {
                // index = tmp;
                result = DeviceFactory::DGN2DeviceMap[dgn][index];
            }
#endif  
            ; // do nothing
        } else if ((dgn == GENERATOR_AC_STATUS_1) || (dgn == GENERATOR_AC_STATUS_2) || (dgn == GENERATOR_AC_STATUS_3) || (dgn == GENERATOR_AC_STATUS_4)) {
            // another special case of BYTE 0 for the instance
#ifdef HOME_KIT_2
            uint8_t outputIndex = GENERATOR_INSTANCE_0_INVALID;
            uint8_t tmp = data[Generator::GENERATOR_BYTE_0] & Generator::GENERATOR_OUTPUT_INDEX_MASK;
            if ((tmp > GENERATOR_INSTANCE_0_INVALID) && (tmp < GENERATOR_INSTANCE_11_INVALID)) {
                outputIndex = tmp;
                result = DeviceFactory::DGN2DeviceMap[dgn][outputIndex];
            }   
#endif         
            ; // do nothing
        } else {        
#ifdef HOME_KIT_1
                result = DeviceFactory::DGN2DeviceMap[dgn][WATER_PUMP_INDEX];   
#endif
#ifdef HOME_KIT_2
                ; // do nothing
#endif
        }        

    }
    return result;
}
      
#include <map>
typedef enum {
    DIMMABLE_SWITCH = 0,
    SWITCH,
    DOOR_LOCK,
    THERMOSTAT,
    HEATED_FLOOR,
    WATER_PUMP,
    ATS,
    BATTERY,
    CHARGER_DEVICE, 
    GENERATOR,
    TANKS,
    INVERTER_DEVICE,
    NULLDT
} DeviceType;

#include <fstream>
#include <string>
#include <sstream> // For parsing lines
constexpr uint8_t YES_NO_COLUMN = 0;
constexpr uint8_t DATA_TYPE_COLUMN = 1;
constexpr uint8_t RVC_INDEX_COLUMN = 2;
constexpr uint8_t SOURCE_ADDRESS_COLUMN = 3;
constexpr uint8_t DEVICE_NAME_COLUMN = 4;
constexpr uint8_t BAD_INDEX = 255;

// Function to remove all whitespace (leading, trailing, and internal)
std::string removeAllWhitespace(const std::string& str) {
    std::string s = str;
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char ch) {
        return std::isspace(ch);
    }), s.end());
    return s;
}


/*
boolean DeviceFactory::createFromConfigFile(void) {
    printf("DeviceFactory::createFromConfigFile Start\n");
    std::map<std::string, DeviceType> deviceType;
    boolean success = false;
    if (fileName != nullptr) {
        std::ifstream file(fileName);
        if (file.is_open()) {
            std::string line;
            int row = 0;
            boolean ignore = false;
            while (std::getline(file, line)) {
                if (row != 0) {
                    std::stringstream ss(line);
                    std::string field;
                    std::string cleanField;
                    int column = 0;
                    DeviceType dataType = NULLDT;
                    std::string deviceName;
                    uint8_t sourceAddress = SOURCE_ADDRESS;
                    uint8_t index = BAD_INDEX;
                    while (std::getline(ss, field, ',')) {
                        // Process each field (e.g., store in a data structure, convert to appropriate types)
                        // Example: std::cout << field << std::endl;
                        cleanField = removeAllWhitespace(field);
                        if ((column == YES_NO_COLUMN) && (cleanField == "Y")) {
                            if (column == DATA_TYPE_COLUMN) {
                                dataType = deviceType[field];
                            }
                            if (column == RVC_INDEX_COLUMN) {
                                // convert index to integer
                                index = atoi(cleanField.c_str());
                            }
                            if (column == SOURCE_ADDRESS_COLUMN) {
                                sourceAddress = atoi(cleanField.c_str());
                            }
                            if (column == DEVICE_NAME_COLUMN) {
                                deviceName = cleanField;
                            }
                        } else if ((column == YES_NO_COLUMN) && (cleanField == "N")) 
                            ignore=true;
                        column++;
                    }
                    // create device from data type, index and device Name
                    if ((!ignore) && (dataType != NULLDT)) {
                        DC_DimmableSwitch* dimSwitch = nullptr;
                        DC_Switch* dcSwitch = nullptr;
                        DoorLock* doorLock = nullptr;
                        HVAC_Thermostat* thermostat = nullptr;
                        WaterPump* waterPump = nullptr;

                        Generator* generator = nullptr;
                        Tanks* tank = nullptr;

                        switch (dataType) {

                            case (DIMMABLE_SWITCH): 
#ifdef HOME_KIT_1
                                {

                                    dimSwitch = new DC_DimmableSwitch(sourceAddress, index); // , LIGHT_GROUP, windowDgns);
                                    DGN2DeviceMap[DC_DIMMER_COMMAND][index] = dimSwitch;
                                    DGN2DeviceMap[DC_DIMMER_STATUS_3][index] = dimSwitch;
                                    DC_DimmableSwitchView::createDC_DimmableSwitchView(dimSwitch, deviceName.c_str());
                                    success = true;
                                }
#endif
                                break;
                            case (SWITCH):
#ifdef HOME_KIT_1
                                {
                                    dcSwitch = new DC_Switch(sourceAddress, index); // , LIGHT_GROUP, windowDgns);
                                    DGN2DeviceMap[DC_DIMMER_COMMAND][index] = dcSwitch;
                                    DGN2DeviceMap[DC_DIMMER_STATUS_3][index] = dcSwitch;
                                    DC_LightSwitchView::createDC_LightSwitchView(dcSwitch, deviceName.c_str());
                                    success = true;
                                }
#endif
                                break;
                            case (DOOR_LOCK):
#ifdef HOME_KIT_1
                                {   
                                    doorLock = new DoorLock(sourceAddress, index); // , LOCK_GROUP, lockDgns);
                                    DGN2DeviceMap[LOCK_COMMAND][index] = doorLock;
                                    DGN2DeviceMap[LOCK_STATUS][index] = doorLock;
                                    DoorLockView::createDoorLockView(doorLock, deviceName.c_str());
                                    success = true;
                                }
#endif
                                break;
                            case (THERMOSTAT):
#ifdef HOME_KIT_1
                                {
                                    thermostat = new HVAC_Thermostat(sourceAddress, index); // , THERMOSTAT_GROUP, thermostatDgns);
                                    DGN2DeviceMap[THERMOSTAT_COMMAND_1][index] = thermostat;
                                    DGN2DeviceMap[THERMOSTAT_COMMAND_2][index] = thermostat;
                                    DGN2DeviceMap[THERMOSTAT_STATUS_1][index] = thermostat;
                                    DGN2DeviceMap[THERMOSTAT_STATUS_2][index] = thermostat;
                                    ThermostatView::createThermostatView(thermostat, deviceName.c_str());
                                    success = true;
                                }
#endif
                                break;
                            case (HEATED_FLOOR):
#ifdef HOME_KIT_2
                                {
                                        printf("DeviceFactory::createFromConfigFile: HEATED FLOOR not supported yet\n");
                                }
#endif
                                break;
                            case (WATER_PUMP):
#ifdef HOME_KIT_1
                                {
                                    waterPump = new WaterPump(sourceAddress); // , WATERPUMP_GROUP, waterPumpDgns);
                                    DGN2DeviceMap[WATER_PUMP_COMMAND][WATER_PUMP_INDEX] = waterPump;
                                    DGN2DeviceMap[WATER_PUMP_STATUS][WATER_PUMP_INDEX] = waterPump;
                                    WaterPumpView::createWaterPumpView(waterPump, deviceName.c_str());
                                    success = true;
                                }
#endif
                                break;
                            case (ATS):
                                    printf("DeviceFactory::createFromConfigFile: ATS not supported yet\n");
                                break;
                            case (BATTERY):
                                    printf("DeviceFactory::createFromConfigFile: Battery not supported yet\n");
                                break;
                            case (CHARGER_DEVICE):
                                    printf("DeviceFactory::createFromConfigFile: Charger not supported yet\n");
                                break;
                            case (GENERATOR):
#ifdef HOME_KIT_2
                                {
                                    generator = new Generator(sourceAddress, index); 
                                    DGN2DeviceMap[GENERATOR_AC_STATUS_1][index] = (PowerSensor* )generator;
                                    GeneratorView::createGeneratorView((GenericDevice* )generator, deviceName.c_str());
                                    success = true;
                                }
#endif
                            case (TANKS):
#ifdef HOME_KIT_2
                                {
                                    tank = new Tanks(sourceAddress, Tanks::FRESH_WATER_INSTANCE); 
                                    DGN2DeviceMap[TANK_STATUS][Tanks::FRESH_WATER_INSTANCE] = tank;
                                    TanksView::createTanksView((GenericDevice* )tank, deviceName.c_str());  
                                    success = true;
                                } 
#endif
                                break;
                            case (INVERTER_DEVICE):
                                printf("DeviceFactory::createFromConfigFile: Inverter not supported yet\n");
                            default:
                                break;
                        }
                    // clear column, dataType, deviceName and index
                    }
                    column = 0;
                    ignore = false;
                    dataType = NULLDT;
                    deviceName.clear();
                    index = BAD_INDEX;
                }
                row++;
            }   
        }
    }
    return success;
} 
    */