#include "Arduino.h"
#define CUSTOM_CHAR_HEADER
#include "HomeSpan.h"
#include "DeviceFactory.h"
#include "RVConstants.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "LittleFS.h"
#include "debug.h"


constexpr Coach coach = EX4551_2022; // Default coach type"
const char* DeviceFactory::fileName = "data/config.rv";

DeviceFactory* DeviceFactory::instance = nullptr;
boolean DeviceFactory::devicesCreated = false;
// std::map<uint8_t, GenericDevice*> DeviceFactory::iD2DeviceMap; // Map to hold devices by instance number
std::map<RVC_DGN, std::map<uint8_t, GenericDevice*>> DeviceFactory::DGN2DeviceMap; // Map of a map to hold devices by DGN number -> look up a devicd by DGN number and instance 

/**
void DeviceFactory::createDevices(void) {
    // Create devices based on the coach type
    devicesCreated = false;
    if (!createFromConfigFile()) {
        switch (coach) {
            case EX4551_2022:
                // Create devices for 2022 Essex 4551
                create2022Essex4551Devices();
                devicesCreated = true;
                break;
            case DS4369_2019:
                // Create devices for 2019 Dutch Star 4369
                create2019DutchStar4369Devices();
                devicesCreated = true;
                break;
            case KA4551_2022:
                // Create devices for 2022 King Aire 4551
                create2022KingAire4551Devices();

                break;
            default:
                throw std::runtime_error("Unknown coach type");
        }
    } else {
        RV_PRINTF("DeviceFactory::createDevices: Devices created from config file\n");
        devicesCreated = true;
    }
}
*/

#include "ChassisMobility.h"

#ifdef HOME_KIT_1
#include "LightDevice.h"
#include "LightDeviceView.h"

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
// #include "Awning.h" // can't implement - AWNING_STATUS does not report position properly so can't get position properly managed in loop
// #include "AwningView.h"
// #include "Shades.h"
// #include "ShadesView.h"
#include "CoverDevice.h"
#include "CoverView.h"
#include "Charger.h"
#endif

#ifdef HOME_KIT_3
void  DeviceFactory::create2022Essex4551Devices() {
    // all devices live live until reboot, the dgn maps do NOT own the memory.
    RV_PRINTF("DeviceFactory::create2022Essex4551Devices Start\n");
    // Create devices for 2022 Essex 4551
        // by id -> create the device and assign pointer in map by ID
        // by DGN - for each DGN to be used fo rthe device - assign a pointer to the DGN in the map -> not sure this will make sense yet
    // On/Off Lights
    // const uint8_t LIGHT_GROUP = 1;
#ifdef HOME_KIT_1
    LightDevice* dimSwitch = new LightDevice(141, 82); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][82] = dimSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][82] = dimSwitch;
    LightDeviceView::createLightDeviceView(dimSwitch, "Living Room  82");

 
    dimSwitch = new LightDevice(141, 84); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][84] = dimSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][84] = dimSwitch;
   LightDeviceView::createLightDeviceView(dimSwitch, "Bedroom Slider 84");
    
    dimSwitch = new LightDevice(141, 85); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][85] = dimSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][85] = dimSwitch;
    LightDeviceView::createLightDeviceView(dimSwitch, "Master Bathroom 85");


    LightDevice* dcSwitch = new LightDevice(141, 98, LightKind::OnOff); // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][98] = dcSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][98] = dcSwitch;
    LightDeviceView::createLightDeviceView(dcSwitch, "Driver Side Security Light 98");

    dcSwitch = new LightDevice(141, 99, LightKind::OnOff);; // , LIGHT_GROUP, windowDgns);
    DGN2DeviceMap[DC_DIMMER_COMMAND][99] = dcSwitch;
    DGN2DeviceMap[DC_DIMMER_STATUS_3][99] = dcSwitch;
    LightDeviceView::createLightDeviceView(dcSwitch, "Passenger Side Security Light 99");
    
    
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
    /* more work to debug and work on timing */
    CoverDevice* awning = new CoverDevice(136, FRONT_AWNING, CoverKind::Awning); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[AWNING_COMMAND][FRONT_AWNING] = awning;
    DGN2DeviceMap[AWNING_STATUS][FRONT_AWNING] = awning;
    CoverView::createCoverView(awning, FRONT_AWNING_NAME, FRONT_AWNING_EXTEND_CYCLE_TIME_SEC, FRONT_AWNING_RETRACT_CYCLE_TIME_SEC);
    awning = new CoverDevice(136, BACK_AWNING, CoverKind::Awning); // , LOCK_GROUP, lockDgns);
    DGN2DeviceMap[AWNING_COMMAND][BACK_AWNING] = awning;
    DGN2DeviceMap[AWNING_STATUS][BACK_AWNING] = awning;
    CoverView::createCoverView(awning, BACK_AWNING_NAME, BACK_AWNING_EXTEND_CYCLE_TIME_SEC, BACK_AWNING_RETRACT_CYCLE_TIME_SEC);
    /* */
    awning = new CoverDevice(136, DOOR_AWNING, CoverKind::Awning); 
    DGN2DeviceMap[AWNING_COMMAND][DOOR_AWNING] = awning;
    DGN2DeviceMap[AWNING_STATUS][DOOR_AWNING] = awning;
    CoverView::createCoverView(awning, DOOR_AWNING_NAME, DOOR_AWNING_EXTEND_CYCLE_TIME_SEC, DOOR_AWNING_RETRACT_CYCLE_TIME_SEC);
    /* */
    // shades
    CoverDevice* shades = new CoverDevice(static_cast<uint8_t>(146), static_cast<uint8_t>(LIVING_ROOM_DAY_SHADE), CoverKind::Shade);
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][LIVING_ROOM_DAY_SHADE] = shades;
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][LIVING_ROOM_DAY_SHADE] = shades;
    CoverView::createCoverView(shades, LIVING_ROOM_DAY_SHADE_NAME);
    shades = new CoverDevice(static_cast<uint8_t>(146), static_cast<uint8_t>(LIVING_ROOM_NIGHT_SHADE), CoverKind::Shade);
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][LIVING_ROOM_NIGHT_SHADE] = shades;
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][LIVING_ROOM_NIGHT_SHADE] = shades;
    CoverView::createCoverView(shades, LIVING_ROOM_NIGHT_SHADE_NAME);
    shades = new CoverDevice(static_cast<uint8_t>(146), static_cast<uint8_t>(BEDROOM_DAY_SHADE), CoverKind::Shade);
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][BEDROOM_DAY_SHADE] = shades;
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][BEDROOM_DAY_SHADE] = shades;
    CoverView::createCoverView(shades, BEDROOM_DAY_SHADE_NAME);
    shades = new CoverDevice(static_cast<uint8_t>(146), static_cast<uint8_t>(BEDROOM_NIGHT_SHADE), CoverKind::Shade);
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][BEDROOM_NIGHT_SHADE] = shades;
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][BEDROOM_NIGHT_SHADE] = shades;
    CoverView::createCoverView(shades, BEDROOM_NIGHT_SHADE_NAME);
    shades = new CoverDevice(static_cast<uint8_t>(146), static_cast<uint8_t>(BATHROOM_DAY_SHADE), CoverKind::Shade);
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][BATHROOM_DAY_SHADE] = shades;
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][BATHROOM_DAY_SHADE] = shades;
    CoverView::createCoverView(shades, BATHROOM_DAY_SHADE_NAME);
    shades = new CoverDevice(static_cast<uint8_t>(146), static_cast<uint8_t>(BATHROOM_NIGHT_SHADE), CoverKind::Shade);
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][BATHROOM_NIGHT_SHADE] = shades;
    DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][BATHROOM_NIGHT_SHADE] = shades;
    CoverView::createCoverView(shades, BATHROOM_NIGHT_SHADE_NAME);
    /* */
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
    inverter->attachView("Inv");
    // InverterView::createInverterView(inverter, "Inv");
    // Inverter 2 - no second inverter on 2022 Essex 4551 - but code is here for future use
    // inverter = new Inverter(250, 2);
    // DGN2DeviceMap[INVERTER_AC_STATUS_1][2] = inverter;
    // DGN2DeviceMap[INVERTER_STATUS][2] = inverter;
    // InverterView::createInverterView(inverter, "Inverter 2");

    // Generator
    Generator* generator = new Generator(250, 1); 
    DGN2DeviceMap[GENERATOR_AC_STATUS_1][static_cast<uint8_t>(GeneratorInstance::GENERATOR_INSTANCE_1)] = (PowerSensor* )generator;
    generator->attachView("Generator");
    // GeneratorView::createGeneratorView((GenericDevice* )generator, "Generator");

    // ATS
    AutomaticTransferSwitch* ats = new AutomaticTransferSwitch(250, 1);
    DGN2DeviceMap[ATS_AC_STATUS_1][1] = (AutomaticTransferSwitch* ) ats;
    DGN2DeviceMap[ATS_AC_STATUS_2][1] = (AutomaticTransferSwitch* ) ats;
    DGN2DeviceMap[ATS_AC_STATUS_3][1] = (AutomaticTransferSwitch* ) ats;
    DGN2DeviceMap[ATS_AC_STATUS_4][1] = (AutomaticTransferSwitch* ) ats;
    ats->attachView("ATS");
    // AutomaticTransferSwitchView::createAutomaticTransferSwitchView(ats, "ATS");

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
    Tanks* tank = new Tanks(250, Tanks::FRESH_WATER_INSTANCE, static_cast<uint16_t>(TANK_CAPACITIES::NEWMAR_ESSEX_2022_FRESH)); 
    DGN2DeviceMap[TANK_STATUS][Tanks::FRESH_WATER_INSTANCE] = tank;
    TanksView::createTanksView((GenericDevice* )tank, Tanks::tankNames.at(Tanks::FRESH_WATER_INSTANCE).c_str());  

    tank = new Tanks(250, Tanks::GRAY_WATER_INSTANCE, static_cast<uint16_t>(TANK_CAPACITIES::NEWMAR_ESSEX_2022_GRAY)); 
    DGN2DeviceMap[TANK_STATUS][Tanks::GRAY_WATER_INSTANCE] = tank;
    TanksView::createTanksView((GenericDevice* )tank, Tanks::tankNames.at(Tanks::GRAY_WATER_INSTANCE).c_str());  

    tank = new Tanks(250, Tanks::BLACK_WATER_INSTANCE, static_cast<uint16_t>(TANK_CAPACITIES::NEWMAR_ESSEX_2022_BLACK)); 
    DGN2DeviceMap[TANK_STATUS][Tanks::BLACK_WATER_INSTANCE] = tank;
    TanksView::createTanksView((GenericDevice* )tank, Tanks::tankNames.at(Tanks::BLACK_WATER_INSTANCE).c_str());  
#endif 

    // Roof Fans -- SilverLeaf does not expose the roof fans 
    // Power Devices

    //tmp = new DC_LightSwitchView(nullptr, "Essex 4551 Light Switch burned out");
    // DC_LightSwitchView::createDC_LightSwitchView(nullptr, "dummy switch");
    RV_PRINTF("2022 Essex 4552 devices created\n");
    RV_PRINTF("DeviceFactory::create2022Essex4551Devices End\n");
}


void DeviceFactory::create2019DutchStar4369Devices() {
    // Create devices for 2019 Dutch Star 4369
    // Implementation goes here
}   

void DeviceFactory::create2022KingAire4551Devices() {
    // Create devices for 2022 King Aire 4551
    // Implementation goes here
}   
#endif


#include "ChassisMobilityView.h"
DeviceFactory* DeviceFactory::getInstance() {
    // RV_PRINTF("DeviceFactory::getInstance Start\n");
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
    // RV_PRINTF("DeviceFactory::getInstance End\n");
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

            // RV_PRINTF("DeviceFactory::getDeviceByData: DGN=0x%08X, index=%d\n", dgn, index);
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
                //    RV_PRINTF("DeviceFactory::getDeviceByData INVERTER_AC_STATUS_1 index %d\n", index);
            }  else if (dgn == INVERTER_STATUS) {
                index = data[INVERTER_LINE_INDEX];
                // if (index > 1)
                //    RV_PRINTF("DeviceFactory::getDeviceByData INVERTER_STATUS index %d\n", index);
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
            uint8_t outputIndex = static_cast<uint8_t>(GeneratorInstance::GENERATOR_INSTANCE_0_INVALID);
            uint8_t tmp = data[Generator::GENERATOR_BYTE_0] & Generator::GENERATOR_OUTPUT_INDEX_MASK;
            if ((tmp > static_cast<uint8_t>(GeneratorInstance::GENERATOR_INSTANCE_0_INVALID)) && (tmp < static_cast<uint8_t>(GeneratorInstance::GENERATOR_INSTANCE_11_INVALID))) {
                outputIndex = tmp;
                result = DeviceFactory::DGN2DeviceMap[dgn][outputIndex];
                // if (dgn == GENERATOR_AC_STATUS_1) {
                    // RV_PRINTF("DeviceFactory::getDeviceByData: Generator DGN=0x%08X, index=%d\n", dgn, outputIndex);
                //}
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
     
#ifdef SMART_COACH_ESP32S3

#include <map>
std::map<String, DeviceFactory::DeviceCreator> DeviceFactory::creators;
CoachSpec DeviceFactory::coachSpec; 

void DeviceFactory::registerCreators() {

    // ============================================================
    // HOME_KIT_1
    // ============================================================
#ifdef HOME_KIT_1

    creators["DC_DimmableSwitch"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new LightDevice(c.sourceAddress, c.rvcIndex, LightKind::Dimmable);
        DGN2DeviceMap[DC_DIMMER_COMMAND][c.rvcIndex]  = d;
        DGN2DeviceMap[DC_DIMMER_STATUS_3][c.rvcIndex] = d;
        LightDeviceView::createLightDeviceView(d, c.name.c_str());
        return d;
    };

    creators["DC_Switch"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new LightDevice(c.sourceAddress, c.rvcIndex, LightKind::OnOff);   // adjust class name if different
        DGN2DeviceMap[DC_DIMMER_COMMAND][c.rvcIndex] = d;       // or whatever DGN your DC_Switch uses
        // … add STATUS DGNs as needed
        LightDeviceView::createLightDeviceView(d, c.name.c_str());
        return d;
    };

    creators["DoorLock"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new DoorLock(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[LOCK_COMMAND][c.rvcIndex] = d;
        DGN2DeviceMap[LOCK_STATUS][c.rvcIndex]  = d;
        DoorLockView::createDoorLockView(d, c.name.c_str());
        return d;
    };

    creators["WaterPump"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        // Note: your current code ignores index and uses a constant
        auto* d = new WaterPump(c.sourceAddress);
        DGN2DeviceMap[WATER_PUMP_COMMAND][WATER_PUMP_INDEX] = d;
        DGN2DeviceMap[WATER_PUMP_STATUS][WATER_PUMP_INDEX]  = d;
        WaterPumpView::createWaterPumpView(d, c.name.c_str());
        return d;
    };

    creators["Thermostat"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        // Your class is actually HVAC_Thermostat
        auto* d = new HVAC_Thermostat(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[THERMOSTAT_COMMAND_1][c.rvcIndex] = d;
        DGN2DeviceMap[THERMOSTAT_COMMAND_2][c.rvcIndex] = d;
        DGN2DeviceMap[THERMOSTAT_STATUS_1][c.rvcIndex]  = d;
        DGN2DeviceMap[THERMOSTAT_STATUS_2][c.rvcIndex]  = d;
        ThermostatView::createThermostatView(d, c.name.c_str());
        return d;
    };

#endif // HOME_KIT_1

    // ============================================================
    // HOME_KIT_2
    // ============================================================
#ifdef HOME_KIT_2

    creators["FloorHeat"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new FloorHeat(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[FLOOR_HEAT_COMMAND][c.rvcIndex] = d;
        DGN2DeviceMap[FLOOR_HEAT_STATUS][c.rvcIndex]  = d;
        FloorHeatView::createFloorHeatView(d, c.name.c_str());
        return d;
    };

    creators["Awning"] = [](const DeviceConfig& c, const CoachSpec& coach) -> GenericDevice* {
        GenericDevice* result = nullptr;
        if (c.enabled) {
            auto* d = new CoverDevice(c.sourceAddress, c.rvcIndex, CoverKind::Awning);

            uint16_t ext = static_cast<uint16_t>(c.extraUInt("extendSec", 40));
            uint16_t ret = static_cast<uint16_t>(c.extraUInt("retractSec", 45));

            CoverTiming out;

            // String key = c.extraString("awningKey");

            if (coach.findCoverTiming(c.name, out)) { // awningTimes.count(key)) {
                ext = out.extendSec;
                ret = out.retractSec;
            }
            d->configureTimings(ext, ret);

            DGN2DeviceMap[AWNING_COMMAND][c.rvcIndex] = d;
            DGN2DeviceMap[AWNING_STATUS][c.rvcIndex]  = d;

            // d->attachView(c.name.c_str());
            CoverView::createCoverView(d, c.name.c_str());
            result = d;
        }
        return result;
    };

    creators["Shades"] = [](const DeviceConfig& c, const CoachSpec& coach) -> GenericDevice* {
        GenericDevice* result = nullptr;
        if (c.enabled) {
            auto* d = new CoverDevice(c.sourceAddress, c.rvcIndex, CoverKind::Shade);

            uint16_t ext = static_cast<uint16_t>(c.extraUInt("extendSec", 40));
            uint16_t ret = static_cast<uint16_t>(c.extraUInt("retractSec", 45));

            CoverTiming out;

            // String key = c.extraString("awningKey");

            if (coach.findCoverTiming(c.name, out)) { // awningTimes.count(key)) {
                ext = out.extendSec;
                ret = out.retractSec;
            }
            d->configureTimings(ext, ret);

            DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][c.rvcIndex] = d;
            DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][c.rvcIndex]  = d;

            // d->attachView(c.name.c_str());
            CoverView::createCoverView(d, c.name.c_str());
            result = d;
        }
        return result;
    };

    creators["Tank"] = [](const DeviceConfig& c, const CoachSpec& coach) -> GenericDevice* {
        // Prefer the instance from the coach spec when possible
        GenericDevice* result = nullptr;
        if (c.enabled) {
            uint16_t tankSz = 0;
            if (const TankSpec* ts = coach.getTank(c.rvcIndex)) {
            tankSz = ts->capacityLiters;
            }
            auto* d = new Tanks(c.sourceAddress, c.rvcIndex, tankSz);
            DGN2DeviceMap[TANK_STATUS][c.rvcIndex] = d;
            TanksView::createTanksView(d, c.name.c_str());
            result = d;
        }
        return result;
    };

    creators["Generator"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new Generator(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[GENERATOR_AC_STATUS_1][c.rvcIndex] = (PowerSensor*)d;
        d->attachView(c.name.c_str());
        // GeneratorView::createGeneratorView((GenericDevice*)d, c.name.c_str());
        return d;
    };

    creators["Inverter"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new Inverter(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[INVERTER_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[INVERTER_STATUS][c.rvcIndex]      = d;
        d->attachView("Inv");  // c.name.c_str()); was too long for all the appenda
        // InverterView::createInverterView(d, c.name.c_str());
        return d;
    };

    creators["Battery"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        // Your current call uses extra constants – adapt as needed
        // Battery(source, MAIN_HOUSE_BATTERY, MULTI_METER)
        auto* d = new Battery(c.sourceAddress, c.rvcIndex, /* MULTI_METER or from extra */ 0);
        DGN2DeviceMap[DC_SOURCE_STATUS_1][c.rvcIndex] = (PowerSensor*)d;
        DGN2DeviceMap[DC_SOURCE_STATUS_2][c.rvcIndex] = (PowerSensor*)d;
        DGN2DeviceMap[DC_SOURCE_STATUS_3][c.rvcIndex] = (PowerSensor*)d;
        DGN2DeviceMap[DC_SOURCE_STATUS_4][c.rvcIndex] = (PowerSensor*)d;
        BatteryView::createBatteryView((GenericDevice*)d, c.name.c_str());
        return d;
    };

    creators["ATS"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new AutomaticTransferSwitch(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[ATS_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_4][c.rvcIndex] = d;
        d->attachView(c.name.c_str());
        // AutomaticTransferSwitchView::createAutomaticTransferSwitchView(d, c.name.c_str());
        return d;
    };
// #ifdef CHARGER_H
    creators["Charger"] = [](const DeviceConfig& c, const CoachSpec&) -> GenericDevice* {
        auto* d = new Charger(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[CHARGER_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_AC_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_AC_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_AC_STATUS_4][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_DC_STATUS][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_STATUS][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[CHARGER_STATUS_3][c.rvcIndex] = d;
        d->attachView(c.name.c_str());
        // AutomaticTransferSwitchView::createAutomaticTransferSwitchView(d, c.name.c_str());
        return d;
    };
// #endif

#endif // HOME_KIT_2

}
#endif
// DeviceFactory creators section



#ifdef SMART_COACH_ESP32
#ifdef HOME_KIT_1
GenericDevice* DeviceFactory::createDimmable(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) { // return nullptr;

        // Your merged LightDevice, or existing dimmer/switch type:
        auto* d = new LightDevice(c.sourceAddress, c.rvcIndex, LightKind::Dimmable);
        // Example extras: "dimmable": "1"
        // bool dimmable = c.getExtraUInt("dimmable", 1) != 0;

        DGN2DeviceMap[DC_DIMMER_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[DC_DIMMER_COMMAND][c.rvcIndex]  = d;
        // add any other light DGNs you already register

        // d->attachView(c.name.c_str());
        LightDeviceView::createLightDeviceView(d, c.name.c_str());
        result = d;
    }
    return result;
}
GenericDevice* DeviceFactory::createLight(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) { // return nullptr;

        // Your merged LightDevice, or existing dimmer/switch type:
        auto* d = new LightDevice(c.sourceAddress, c.rvcIndex, LightKind::OnOff);
        // Example extras: "dimmable": "1"
        // bool dimmable = c.getExtraUInt("dimmable", 1) != 0;

        DGN2DeviceMap[DC_DIMMER_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[DC_DIMMER_COMMAND][c.rvcIndex]  = d;
        // add any other light DGNs you already register

        // d->attachView(c.name.c_str());
        LightDeviceView::createLightDeviceView(d, c.name.c_str());
        result = d;
    }
    return result;
}
GenericDevice* DeviceFactory::createDoorLock(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        auto* d = new DoorLock(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[LOCK_COMMAND][c.rvcIndex] = d;
        DGN2DeviceMap[LOCK_STATUS][c.rvcIndex]  = d;
        DoorLockView::createDoorLockView(d, c.name.c_str());
        result = d;
    }
    return result;
}

GenericDevice* DeviceFactory::createThermostat(const DeviceConfig& c, const CoachSpec&) {
    if (!c.enabled) return nullptr;

    auto* d = new HVAC_Thermostat(c.sourceAddress, c.rvcIndex);
    // register the thermostat DGNs you already use, e.g.:
    DGN2DeviceMap[THERMOSTAT_STATUS_1][c.rvcIndex]  = d;
    DGN2DeviceMap[THERMOSTAT_COMMAND_1][c.rvcIndex] = d;
    // ...

    // d->attachView(c.name.c_str());  // or existing createThermostatView pattern
    ThermostatView::createThermostatView(d, c.name.c_str());
    return d;
}

GenericDevice* DeviceFactory::createWaterPump(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        auto* d = new WaterPump(c.sourceAddress); // your type name
        DGN2DeviceMap[WATER_PUMP_STATUS][c.rvcIndex]  = d;
        DGN2DeviceMap[WATER_PUMP_COMMAND][c.rvcIndex] = d;
        WaterPumpView::createWaterPumpView(d, c.name.c_str());
        result = d;
    }
    return result;
}

#endif
#ifdef HOME_KIT_2
GenericDevice* DeviceFactory::createCover(const DeviceConfig& c, const CoachSpec& coach) {
    GenericDevice* result = nullptr;
    if (c.enabled) {

        auto* d = new CoverDevice(c.sourceAddress, c.rvcIndex, CoverKind::Awning);

        uint16_t ext = static_cast<uint16_t>(c.extraUInt("extendSec", 40));
        uint16_t ret = static_cast<uint16_t>(c.extraUInt("retractSec", 45));

        CoverTiming out;

        // String key = c.extraString("awningKey");

        if (coach.findCoverTiming(c.name, out)) { // awningTimes.count(key)) {
            ext = out.extendSec;
            ret = out.retractSec;
        }
        d->configureTimings(ext, ret);

        DGN2DeviceMap[AWNING_COMMAND][c.rvcIndex] = d;
        DGN2DeviceMap[AWNING_STATUS][c.rvcIndex]  = d;

    // d->attachView(c.name.c_str());
        CoverView::createCoverView(d, c.name.c_str());
        result = d;
    }
    return result;
}

GenericDevice* DeviceFactory::createShades(const DeviceConfig& c, const CoachSpec& coach) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        auto* d = new CoverDevice(c.sourceAddress, c.rvcIndex, CoverKind::Shade);
        uint16_t ext = static_cast<uint16_t>(c.extraUInt("extendSec", 40));
        uint16_t ret = static_cast<uint16_t>(c.extraUInt("retractSec", 45));
        CoverTiming out;
        if (coach.findCoverTiming(c.name, out)) { // awningTimes.count(key)) {
            ext = out.extendSec;
            ret = out.retractSec;
        }
        DGN2DeviceMap[WINDOW_SHADE_CONTROL_COMMAND][c.rvcIndex] = d;
        DGN2DeviceMap[WINDOW_SHADE_CONTROL_STATUS][c.rvcIndex]  = d;
        CoverView::createCoverView(d, c.name.c_str());
        result = d;
    }
    return result;
}

GenericDevice* DeviceFactory::createTank(const DeviceConfig& c, const CoachSpec& coach) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        uint16_t tankSz = 0;
        if (const TankSpec* ts = coach.getTank(c.rvcIndex)) {
            tankSz = ts->capacityLiters;
        }
        auto* d = new Tanks(c.sourceAddress, c.rvcIndex, tankSz);
        DGN2DeviceMap[TANK_STATUS][c.rvcIndex] = d;
        TanksView::createTanksView(d, c.name.c_str());
        result = d;
    }
    return result;
}

GenericDevice* DeviceFactory::createBattery(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        uint16_t priority = c.order;
        auto* d = new Battery(c.sourceAddress, c.rvcIndex, priority);

        DGN2DeviceMap[DC_SOURCE_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[DC_SOURCE_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[DC_SOURCE_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[DC_SOURCE_STATUS_4][c.rvcIndex] = d;

        // Battery still uses existing view path
        BatteryView::createBatteryView(d, c.name.c_str());
        result = d;
    }
    return result;
}


static GenericDevice* createCharger(const DeviceConfig& config, const CoachSpec& /*coach*/ )  {
    GenericDevice* result = nullptr;

    if (config.enabled) {
        Charger* device = new Charger(config.sourceAddress, config.rvcIndex);
        DGN2DeviceMap[CHARGER_AC_STATUS_1][config.rvcIndex] = device;
        DGN2DeviceMap[CHARGER_STATUS][config.rvcIndex]       = device;
        DGN2DeviceMap[CHARGER_STATUS_2][config.rvcIndex]     = device;
        device->attachView(config.name.c_str());
        result = device;
    }
    return result;
}



GenericDevice* DeviceFactory::createInverter(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        auto* d = new Inverter(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[INVERTER_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[INVERTER_STATUS][c.rvcIndex]      = d;
        d->attachView(c.name.c_str());
        result = d;
    }
    return result;
}

GenericDevice* DeviceFactory::createGenerator(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        auto* d = new Generator(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[GENERATOR_AC_STATUS_1][c.rvcIndex] = d;
        d->attachView(c.name.c_str());
        result = d;
    }
    return result;
}

GenericDevice* DeviceFactory::createATS(const DeviceConfig& c, const CoachSpec&) {
    GenericDevice* result = nullptr;
    if (c.enabled) {
        auto* d = new AutomaticTransferSwitch(c.sourceAddress, c.rvcIndex);
        DGN2DeviceMap[ATS_AC_STATUS_1][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_2][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_3][c.rvcIndex] = d;
        DGN2DeviceMap[ATS_AC_STATUS_4][c.rvcIndex] = d;
        d->attachView(c.name.c_str());
        result = d;
    }
    return result;
}
#endif
#endif

bool DeviceFactory::loadCoachSpec(const char* path, CoachSpec& out) {
    bool success = false;
    File file;

    if (path != nullptr && LittleFS.begin()) {
        file = LittleFS.open(path, "r");
        if (file) {
            JsonDocument doc;   // or StaticJsonDocument<2048> if you fix the size
            DeserializationError err = deserializeJson(doc, file);
            file.close();

            if (!err) {
                out.year      = doc["year"]      | 0;
                out.make      = doc["make"]      | "";
                out.model     = doc["model"]     | "";
                out.floorplan = doc["floorplan"] | "";
                out.coachId   = doc["coachId"] | "";

                out.tanks.clear();
                JsonArray tanks = doc["tanks"].as<JsonArray>();
                for (JsonObject t : tanks) {
                    TankSpec tank;
                    tank.instance        = t["instance"]        | 0;
                    tank.capacityLiters  = t["capacityLiters"] | 0;
                    const char* n = t["name"] | "";
                    tank.name = n;
                    //tank.name            = t["name"]            | "";
                    RV_PRINTF("DeviceFactory::loadCoachSpec instance %d, capacityLiters %d, name %s", 
                        tank.instance, tank.capacityLiters, tank.name.c_str());
                    out.tanks.push_back(tank);
                }

                out.batteries.clear();
                JsonArray batts = doc["batteries"].as<JsonArray>();
                for(JsonObject o : batts) {
                    BatterySpec b;
                    b.instance = o["instance"] | 0;
                    b.nominalVoltage = o["nominalVoltage"] | 12.0f;
                    b.capacityAh = o["capacityAh"] | 0.0f;
                    const char* n = o["name"] | "";
                    b.name = n;
                    out.batteries.push_back(b);
                }

                out.coverTimings.clear();
                JsonObject cover = doc["coverTimes"].as<JsonObject>();
                for (JsonPair kv : cover) {
                    CoverTimingPair pair;
                    pair.key = kv.key().c_str();
                    pair.timing.extendSec  = kv.value()["extendSec"]  | 40;
                    pair.timing.retractSec = kv.value()["retractSec"] | 45;
                    out.coverTimings.push_back(pair);
                }
                success = true;
            }
        }
    }
    return success;
}

#ifdef SMART_COACH_ESP32
// Static table lives in flash (as const data)
const CreatorEntry DeviceFactory::kCreatorTable[] = {
#ifdef HOME_KIT_1
    { "Light",       DeviceFactory::createLight       },
    { "Thermostat",  DeviceFactory::createThermostat  },
    { "WaterPump",   DeviceFactory::createWaterPump   },
    { "DoorLock",    DeviceFactory::createDoorLock    },
#endif
#ifdef HOME_KIT_2
    { "Shades",      DeviceFactory::createShades      },
    { "Tank",        DeviceFactory::createTank        },
    { "Battery",     DeviceFactory::createBattery     },
    { "Inverter",    DeviceFactory::createInverter    },
//     { "Charger",     DeviceFactory::createCharger     },
    { "Generator",   DeviceFactory::createGenerator   },
    { "ATS",         DeviceFactory::createATS         }
#endif
};



DeviceCreatorFn DeviceFactory::findCreator(const char* typeName) {
    DeviceCreatorFn result = nullptr;
    const size_t kCreatorCount = sizeof(DeviceFactory::kCreatorTable) / sizeof(DeviceFactory::kCreatorTable[0]);
    if (typeName != nullptr) {
        for (size_t i = 0; i < kCreatorCount; ++i) {
            if (strcmp(DeviceFactory::kCreatorTable[i].typeName, typeName) == 0) {
                result = DeviceFactory::kCreatorTable[i].create;
                break;
            }
        }
    }
    return result;
}
#endif


bool DeviceFactory::loadDeviceConfigs(const char* path, std::vector<DeviceConfig>& out) {
    bool         success = false;
    File         file;
    bool         fileOk  = false;
    bool         pathOk  = false;
    JsonDocument doc;
    DeserializationError err = DeserializationError::Ok;
    JsonArray    devices;
    size_t       i = 0;
    size_t       n = 0;

    out.clear();

    if (path != nullptr && path[0] != '\0') {
        pathOk = true;
    }

    if (pathOk && LittleFS.begin(false)) {
        file = LittleFS.open(path, "r");
        if (file) {
            fileOk = true;
        }
    }

    if (fileOk) {
        err = deserializeJson(doc, file);
        file.close();
    }

    if (fileOk && err == DeserializationError::Ok) {
        // Root is an array: [ { ... }, { ... } ]
        devices = doc.as<JsonArray>();
        if (!devices.isNull()) {
            n = devices.size();
            i = 0;
            while (i < n) {
                JsonObject obj = devices[i].as<JsonObject>();
                DeviceConfig cfg;   // defaults from struct

                if (!obj.isNull()) {
                    if (obj["enabled"].is<bool>()) {
                        cfg.enabled = obj["enabled"].as<bool>();
                    }
                    if (obj["type"].is<const char*>()) {
                        cfg.type = obj["type"].as<const char*>();
                    }
                    if (obj["rvcIndex"].is<int>()) {
                        cfg.rvcIndex = static_cast<uint8_t>(obj["rvcIndex"].as<int>());
                    }
                    if (obj["sourceAddress"].is<int>()) {
                        cfg.sourceAddress = static_cast<uint8_t>(obj["sourceAddress"].as<int>());
                    }
                    if (obj["name"].is<const char*>()) {
                        cfg.name = obj["name"].as<const char*>();
                    }
                    if (obj["order"].is<int>()) {
                        cfg.order = static_cast<uint16_t>(obj["order"].as<int>());
                    }

                    // free-form "extra": { "awningKey": "front", "extendSec": "45", ... }
                    JsonObject extraObj = obj["extra"].as<JsonObject>();
                    if (!extraObj.isNull()) {
                        for (JsonPair kv : extraObj) {
                            ExtraPair pair;
                            pair.key = kv.key().c_str();
                            if (kv.value().is<const char*>()) {
                                pair.value = kv.value().as<const char*>();
                            } else if (kv.value().is<int>()) {
                                pair.value = String(kv.value().as<int>());
                            } else if (kv.value().is<bool>()) {
                                pair.value = kv.value().as<bool>() ? "1" : "0";
                            } else {
                                pair.value = "";
                            }
                            if (pair.key.length() > 0) {
                                cfg.extras.push_back(pair);
                            }
                        }
                    }

                    if (cfg.type.length() > 0) {
                        out.push_back(cfg);
                    }
                }
                i = i + 1;
            }
            success = (out.size() > 0);
        }
    }

    return success;
}

#include <algorithm>
void DeviceFactory::createFromDeviceConfig(const std::vector<DeviceConfig>& devices,
                                           const CoachSpec& coach) {

    // 1. Create a lightweight vector of pointers pointing to the original devices
    printf("DeviceFactory::createFromDeviceConfig\n");
    /*
    std::vector<const DeviceConfig*> sorted;
    sorted.reserve(devices.size());
    for (const auto& dev : devices) {
        sorted.push_back(&dev);
    }

    // 2. Sort the pointers safely based on the device's order field
    std::stable_sort(sorted.begin(), sorted.end(),
        [](const DeviceConfig* a, const DeviceConfig* b) {
            return a->order < b->order;
        });
    */
    // 3. To use the sorted items, access them via pointer syntax (->)
    // for (const DeviceConfig* dev : sorted) {
    for (const DeviceConfig dev : devices) {
        // dev->type, dev->name, dev->extraUInt(...), etc.
        RV_PRINTF("Device: %s, Order: %u\n", dev.name.c_str(), dev.order);
        if (dev.enabled) {
            #ifdef SMART_COACH_ESP32
            DeviceCreatorFn function = findCreator(dev->type.c_str());
            printf("DeviceFactory::createFromDeviceConfig name %s, type %s, index %d \n",
                    dev->name.c_str(), dev->type.c_str(), dev->rvcIndex);
            if (function != nullptr) {
                (void)function(*dev, coach);
            } else {
                RV_PRINTF("Unknown device type: %s\n", config.type.c_str());
            }
            #else
            printf("DeviceFactory::createFromDeviceConfig after sorting - this device is enabled type=%s, name=%s, index=%d\n",
                   dev.type.c_str(), dev.name.c_str(), dev.rvcIndex);
            auto it = creators.find(dev.type.c_str());
            if (it != creators.end()) {
                printf("DeviceFactory::createFromDeviceConfig found it!\n");
                /** */
                GenericDevice* md = it->second(dev, coach);
                if (md) {
                // any = true;
                    printf("Created %s (%s) idx=%u\n",dev.name.c_str(), dev.type.c_str(), dev.rvcIndex);
                }
                /* */
            } 
            #endif
        }
    } 
}


void DeviceFactory::createDevices(void) {
    CoachSpec coach;
    std::vector<DeviceConfig> devices;
    bool loaded = false;

    loaded = loadCoachSpec("/coach.json", coach);
    if (loaded) {
        /*
        RV_PRINTF("DeviceFactory::createDevices CoachSpec: year %d, make %s, model %s, floorplan %s, coachID %s\n", coach.year, coach.make, coach.model, coach.floorplan, coach.coachId);
        for (const auto& item : coach.tanks) {
            RV_PRINTF("DeviceFactory::createDevices CoachSpec tank: name %s, instance %d, liters %d \n", 
                item.name.c_str(), item.instance, item.capacityLiters);
        }
        for (const auto& item2 : coach.batteries) {
            RV_PRINTF("DeviceFactory::createDevices CoachSpec batteries: name %s, instance %d, voltage %f, capacity %f \n", 
                item2.name.c_str(), item2.instance, item2.nominalVoltage, item2.capacityAh);
        }
        for (const auto& item3 : coach.coverTimings) {
            RV_PRINTF("DeviceFactory::createDevices CoachSpec coverTimings: name: %s, extendSec %d, retractSec %d \n",
                item3.key.c_str(), item3.timing.extendSec, item3.timing.retractSec);
        }
                */
        RV_PRINTF("DeviceFactory::createDevices - loading devices\n");
        loaded = loadDeviceConfigs("/devices.json", devices);
    }
    if (loaded && !devices.empty()) {
        registerCreators();
        /*
        for (const auto& item : devices) {
            printf("DeviceFactory::createDevices Device type %s, enabled %d, index %d, name %s, order %d \n",
                item.type.c_str(), item.enabled, item.rvcIndex, item.name.c_str(), item.order);
            if (!item.extras.empty()) {
                for (const auto& itm : item.extras) {
                    printf("DeviceFactory::createDevices Extras : key %s, value %s \n", itm.key.c_str(), itm.value.c_str());
                }
            }
        } 
        */
        createFromDeviceConfig(devices, coach);
    } 
#ifdef HOME_KIT_3 
    else {
        create2022Essex4551Devices();
    }
#endif
}
