#ifndef DGN_H
#define DGN_H
#include <cstdint>
#include <stdint.h>
#include <mutex>
#include <map>
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "PacketKit.h" // Make sure this header defines the PacketKit class before this file is included

constexpr uint8_t RVCPercentMax = 250;
constexpr uint8_t homeKitTemperatureDisplayCelsius = 0;
constexpr uint8_t homeKitTemperatureDisplayFahrenheit = 1;

typedef enum {
    ERROR = 0x0000, // Error
    DATE_TIME_STATUS = 0x1FFFF, // Date and Time Status
    SET_DATE_TIME_COMMAND = 0x1FFFE, // Set Date and Time Command
    GPS_DATE_TIME_STATUS = 0x1FEA0, // GPS Date and Time Status

    // switches and dimmers
    DC_DIMMER_COMMAND = 0x1FFB9, // DC Dimmer Command 1
    DC_DIMMER_COMMAND_2 = 0x1FEDB,
    DC_DIMMER_STATUS_1 = 0x1FFBB, // DC Dimmer Status 1
    DC_DIMMER_STATUS_2 = 0x1FFBA, // DC Dimmer Status 2
    DC_DIMMER_STATUS_3 = 0x1FEDA,
    DC_LIGHTING_CONTROLLER_STATUS_1 = 0x1FDC2 , // DC Lighting Controller Status 1 
    DC_LIGHTING_CONTROLLER_STATUS_2 = 0x1FDC1, // DC Lighting Controller Status 2
    DC_LIGHTING_CONTROLLER_STATUS_3 = 0x1FDC0, // DC Lighting Controller Status 3 
    DC_LIGHTING_CONTROLLER_STATUS_4 = 0x1FDBF, // DC Lighting Controller Status 4
    DC_LIGHTING_CONTROLLER_STATUS_5 = 0x1FDBE, // DC Lighting Controller Status 5
    DC_LIGHTING_CONTROLLER_STATUS_6 = 0x1FDBD, // DC Lighting Controller Status 6
    DC_LIGHTING_CONTROLLER_COMMAND_1 = 0x1FDBC, // DC Lighting Controller Command 1
    DC_LIGHTING_CONTROLLER_COMMAND_2 = 0x1FDBB, // DC Lighting Controller Command 2
    DC_LIGHTING_CONTROLLER_COMMAND_3 = 0x1FDBA, // DC Lighting Controller Command 3
    DC_LIGHTING_CONTROLLER_COMMAND_4 = 0x1FDB9, // DC Lighting Controller Command 4
    DC_LIGHTING_CONTROLLER_COMMAND_5 = 0x1FDB8, // DC Lighting Controller Command 5
    DC_LIGHTING_CONTROLLER_COMMAND_6 = 0x1FDB7, // DC Lighting Controller Command 6


    DC_MOTOR_CONTROL_STATUS = 0x1FEE0, // DC Motor Control Status
    
    // generic
    GENERIC_INDICATOR_STATUS = 0x1FED7, // Generic Indicator Status
    GENERIC_INDICATOR_COMMAND = 0x1FED9,
    GENERIC_CONFIGURATION_STATUS = 0x1FED8,

    // window shade (day & night shades)
    WINDOW_SHADE_CONTROL_STATUS = 0x1FEDE,
    WINDOW_SHADE_CONTROL_COMMAND = 0x1FEDF,

    // chassis
    CHASSIS_MOBILITY_STATUS = 0x1FFF4,
    CHASSIS_MOBILITY_STATUS_2 = 0x1FEA8,
    CHASSIS_MOBILITY_COMMAND = 0x1FFF3,

    // water - black, gray, and fressh water tanks
    TANK_STATUS = 0x1FFB7,
    TANK_CALIBRATION_COMMAND = 0x1FFB6, 
    TANK_GEOMETRY_STATUS = 0x1FFB5,
    TANK_GEOMETRY_COMMAND = 0x1FFB4,
    AUTOFILL_STATUS = 0x1FFB1,
    WATER_PUMP_STATUS = 0x1FFB3,
    WATER_PUMP_COMMAND = 0x1FFB2,

    // HVAC (Heating, Ventilation, and Air Conditioning)
    AIR_CONDITIONER_STATUS = 0x1FFE1, 
    AIR_CONDITIONER_COMMAND = 0x1FFE0,
    THERMOSTAT_AMBIENT_STATUS = 0x1FF9C,
    THERMOSTAT_STATUS_1 = 0x1FFE2,
    THERMOSTAT_STATUS_2 = 0x1FEFA,
    THERMOSTAT_COMMAND_1 = 0x1FEF9,
    THERMOSTAT_COMMAND_2 = 0x1FEF8, 
    FLOOR_HEAT_STATUS = 0x1FEFC,
    FLOOR_HEAT_COMMAND = 0x1FEFB,
    FURNACE_STATUS = 0x1FFE4,
    FURNACE_COMMAND = 0x1FFE3,
    HEAT_PUMP_STATUS = 0x1FF9B,
    HEAT_PUMP_COMMAND = 0x1FF9A, 
    THERMOSTAT_SCHEDULE_STATUS_1 = 0x1FEF7, // Thermostat Schedule Status 1
    THERMOSTAT_SCHEDULE_STATUS_2 = 0x1FEF7, // Thermostat Schedule Status 2 
    THERMOSTAT_SCHEDULE_COMMAND_1 = 0x1FEF5, // Thermostat Schedule Command 1
    THERMOSTAT_SCHEDULE_COMMAND_2 = 0x1FEF4, // Thermostat Schedule Command 2
    ROOF_FAN_STATUS_1 = 0x1FEA7, // Roof Fan Status 1
    ROOF_FAN_COMMAND_1 = 0x1FEA6, // Roof Fan Command 1
    ROOF_FAN_STATUS_2 = 0x1FDE3, // Roof Fan Status 2
    ROOF_FAN_COMMAND_2 = 0x1FDE2,

	// ATS (Automatic Transfer Switch)
    ATS_STATUS = 0x1FFAA,
    ATS_COMMAND = 0x1FFA9,
    ATS_AC_STATUS_1 = 0x1FFAD,
    ATS_AC_STATUS_2 = 0x1FFAC,
    ATS_AC_STATUS_3 = 0x1FFAB,
    ATS_AC_STATUS_4 = 0x1FF85,	
    ATS_ACFAULT_CONFIGURATION_STATUS_1 = 0x1FF84,
    ATS_ACFAULT_CONFIGURATION_STATUS_2 = 0x1FF83,
    ATS_ACFAULT_CONFIGURATION_COMMAND_1 = 0x1FF82,
    ATS_ACFAULT_CONFIGURATION_COMMAND_2 = 0x1FF81,

    // AC & DC Load (Power) 
    DC_LOAD_COMMAND = 0x1FFBC,
    DC_LOAD_STATUS = 0x1FFBD,
    DC_LOAD_STATUS_2 = 0x1FEDC,
    DC_SOURCE_STATUS_1 = 0x1FFFD,
    DC_SOURCE_STATUS_2 = 0x1FFFC,
    DC_SOURCE_STATUS_3 = 0x1FFFB,
    DC_SOURCE_STATUS_4 = 0x1FEC9,
    DC_SOURCE_STATUS_5 = 0x1FEC8,
    DC_SOURCE_STATUS_6 = 0x1FEC7,
    DC_SOURCE_STATUS_7 = 0x1FEAC,
    DC_SOURCE_STATUS_8 = 0x1FEAB,
    DC_SOURCE_STATUS_9 = 0x1FEAA,
    DC_SOURCE_STATUS_10 = 0x1FEA9,
    DC_SOURCE_STATUS_11 = 0x1FEA5,
    DC_SOURCE_STATUS_12 = 0x1FDF8,
    DC_SOURCE_STATUS_13 = 0x1FDE6,

    DC_SOURCE_COMMAND = 0x1FEA4, 

    DC_SOURCE_CONFIGURATION_STATUS_1 = 0x1FDF7,
    DC_SOURCE_CONFIGURATION_COMMAND_1 = 0x1FDF6,
    DC_SOURCE_CONFIGURATION_STATUS_2 = 0x1FDF5,
    DC_SOURCE_CONFIGURATION_COMMAND_2 = 0x1FDF4, 
    DC_SOURCE_CONFIGURATION_COMMAND_3 = 0x1FDDE,

    DC_DISCONNECT_STATUS = 0x1FED0,
    DC_DISCONNECT_COMMAND = 0x1FECF,

    AC_LOAD_STATUS = 0x1FFBF,
    AC_LOAD_STATUS_2 = 0x1FEDD, 
    AC_LOAD_COMMAND = 0x1FFBE,

    // charger
    CHARGER_COMMAND = 0x1FFC5,
    CHARGER_CONFIGURATION_COMMAND = 0x1FFC4,
    CHARGER_CONFIGURATION_COMMAND_2 = 0x1FF95,
    CHARGER_CONFIGURATION_COMMAND_3 = 0x1FECB,
    CHARGER_CONFIGURATION_COMMAND_4 = 0x1FEBE,
    CHARGER_CONFIGURATION_STATUS = 0x1FFC6,
    CHARGER_CONFIGURATION_STATUS_2 = 0x1FF96,
    CHARGER_CONFIGURATION_STATUS_3 = 0x1FECC,
    CHARGER_CONFIGURATION_STATUS_4 = 0x1FEBF,
    CHARGER_STATUS = 0x1FFC7,
    CHARGER_STATUS_2 = 0x1FEA3,
    CHARGER_STATUS_3 = 0x1FDCA,
    CHARGER_AC_STATUS_1 = 0x1FFCA,
    CHARGER_AC_STATUS_2 = 0x1FFC9,
    CHARGER_AC_STATUS_3 = 0x1FFC8,
    CHARGER_AC_STATUS_4 = 0x1FF8A,
    CHARGER_DC_STATUS = 0x1FFCE,
    CHARGER_EQUALIZATION_CONFIGURATION_COMMAND = 0x1FF97,
    CHARGER_EQUALIZATION_STATUS = 0x1FF98,

    // generator
    GENERATOR_STATUS_1 = 0x1FFDC,
    GENERATOR_STATUS_2 = 0x1FFDB,
    GENERATOR_COMMAND = 0x1FFDA,
    GENERATOR_AC_STATUS_1 = 0x1FFDF,
    GENERATOR_AC_STATUS_2 = 0x1FFDE,
    GENERATOR_AC_STATUS_3 = 0x1FFDD,
    GENERATOR_AC_STATUS_4 = 0x1FF94,
    GENERATOR_ACFAULT_COFIGURATION_STATUS_1 = 0x1ff93,
    GENERATOR_ACFAULT_COFIGURATION_STATUS_2 = 0x1FF92,
    GENERATOR_ACFAULT_COFIGURATION_COMMAND_1 = 0x1FF91,
    GENERATOR_ACFAULT_COFIGURATION_COMMAND_2 = 0x1FF90,
    GENERATOR_DC_STATUS_1 = 0x1FEC6,
    GENERATOR_DC_STATUS_2 = 0x1FDDC,
    GENERATOR_DC_CONFIURATION_STATUS = 0x1FEC5,
    GENERATOR_DC_COMMAND = 0x1FEC4,
    GENERATOR_DC_CONFIGURATION_COMMAND = 0x1FEC3,
    GENERATOR_DC_CONFIGURATION_STATUS_2 = 0x1FDDB,
    GENERATOR_DC_CONFIGURATION_COMMAND_2 = 0x1FDDA,
    GENERATOR_DC_CONFIGURATION_STATUS_3 = 0x1FDD9,
    GENERATOR_DC_CONFIGURATION_COMMAND_3 = 0x1FDD8,
    GENERATOR_DC_CONFIGURATION_STATUS_4 = 0x1FDD7,
    GENERATOR_DC_CONFIGURATION_COMMAND_4 = 0x1FDD6,
    GENERATOR_DC_EQUALIZATION_CONFIGURATION_STATUS = 0x1FEC1,
    GENERATOR_DC_EQUALIZATION_CONFIGURATION_COMMAND = 0x1FEC0,
    GENERATOR_START_CONFIG_STATUS = 0x1FFD9,
    GENERATOR_START_CONFIG_COMMAND = 0x1FFD8,
    GENERATOR_DEMAND_STATUS = 0x1FF80,
    GENERATOR_DEMAND_COMMAND = 0x1FEFF,
    GENERATOR_DEMAND_CONFIGURATION_STATUS = 0x1FEE7,
    GENERATOR_DEMAND_CONFIGURATION_COMMAND = 0x1FEE6,
    
    // AGS
    AGS_CRITERION_STATUS = 0x1FEFE,
    AGS_CRITERION_STATUS_2 = 0x1FED2,
    AGS_CRITERION_COMMMAND = 0x1FEFD,
    AGS_DEMAND_CONFIGURATION_STATUS = 0x1FED5,
    AGS_DEMAND_CONFIGURATION_COMMAND = 0x1FED4,


    // inverter
    INVERTER_STATUS = 0x1FFD4,
    INVERTER_AC_STATUS_1 = 0x1FFD7,
    INVERTER_AC_STATUS_2 = 0x1FFD6,
    INVERTER_AC_STATUS_3 = 0x1FFD5,
    INVERTER_AC_STATUS_4 = 0x1FF8F,
    INVERTER_ACFAULT_CONFIGURATION_STATUS_1 = 0x1FF8E,
    INVERTER_ACFAULT_CONFIGURATION_STATUS_2 = 0x1FF8D,
    INVERTER_ACFAULT_CONFIGURATION_COMMAND_1 = 0x1FF8C,
    INVERTER_ACFAULT_CONFIGURATION_COMMAND_2 = 0x1FF8B,
    INVERTER_COMMAND = 0x1FFD3,
    INVERTER_CONFIGURATION_STATUS_1 = 0x1FFD2,
    INVERTER_CONFIGURATION_STATUS_2 = 0x1FFD1,
    INVERTER_CONFIGURATION_STATUS_3 = 0x1FECE,
    INVERTER_CONFIGURATION_STATUS_4 = 0x1FE9B,
    INVERTER_CONFIGURATION_COMMAND_1 = 0x1FFD0,
    INVERTER_CONFIGURATION_COMMAND_2 = 0x1FECF,
    INVERTER_CONFIGURATION_COMMAND_3 = 0x1FECD,
    INVERTER_CONFIGURATION_COMMAND_4 = 0x1FE9A,
    INVERTER_STATISTICS_STATUS = 0x1FFCE,
    INVERTER_APS_STATUS = 0x1FFCD,
    INVERTER_DCBUS_STATUS = 0x1FFCC,
    INVERTER_OPE_STATUS = 0x1FFCB,
    INVERTER_DC_STATUS = 0x1FEE8,
    INVERTER_TEMPERATURE_STATUS = 0x1FEBD,
    INVERTER_TEMPERATURE_STATUS_2 = 0x1FDCB,
    // INVERTER_DC_STATUS_1 = 0x1FEE9, according to the RVC spec, this is a tire_id_command
    
    // door locks (front and bay doors)
    LOCK_STATUS = 0x1FEE5,
    LOCK_COMMAND = 0x1FEE4,

    // Awnings
    AWNING_STATUS = 0x1FEF3,
    AWNING_STATUS_2 = 0x1FDCD,
    AWNING_COMMAND = 0x1FEF2,
    AWNING_COMMAND_2 = 0x1FDCC,

    // Battery - read the readme on the battery status 
    BATTERY_STATUS_1 = 0x1FE95, // Battery Status 1
    BATTERY_STATUS_2 = 0x1FE94, // Battery Status 2
    BATTERY_STATUS_3 = 0x1FE93, // Battery Status 3
    BATTERY_STATUS_4 = 0x1FE92, // Battery Status 4
    BATTERY_STATUS_5 = 0x1FE91, // Battery Status 5
    BATTERY_STATUS_6 = 0x1FE90, // Battery Status 6
    BATTERY_STATUS_7 = 0x1FE8F, // Battery Status 7
    BATTERY_STATUS_8 = 0x1FE8E, // Battery Status 8
    BATTERY_STATUS_9 = 0x1FE8D, // Battery Status 9
    BATTERY_STATUS_10 = 0x1FE8C, // Battery Status 10
    BATTERY_STATUS_11 = 0x1FE8B, // Battery Status 11
    BATTERY_STATUS_12 = 0x1FDF3, // Battery Status 12
    BATTERY_STATUS_13 = 0x1FDF2, // Battery Status 13
    BATTERY_COMMAND = 0x1FE8A, // Battery Command
    BATTERY_SUMMARY = 0x1FDF1 // Battery Summary
    // BATTERY_STATUS = 0x1AAFD;  WAS NOT IN THE RVC SPEC BUT WAS IN THE ORIGINAL CODE.
} RVC_DGN;

class DGN : public PacketKit {
    
    private: 
        // DGN specific constants
        static constexpr uint32_t DGN_INDICATOR_COMMAND = 0x1FED9;
        static constexpr uint32_t DGN_CONFIGURATION_STATUS = 0x1FED8;
        static constexpr int8_t DGN_START_BIT = 24; // Start bit for DGN extraction
        static constexpr int8_t DGN_NUM_BITS = 17; // Number of bits for DGN extraction
        static constexpr uint8_t SOURCE_ADDRESS_START_BIT = 7; // Start bit for source address extraction
        static constexpr uint8_t SOURCE_ADDRESS_NUM_BITS = 8; // Number of bits for source address extraction
        static std::mutex dgnMutex;
        static DGN* instance;

        DGN(void) : PacketKit() {}
        DGN(const DGN&) = delete; // Prevent copy
        DGN& operator=(const DGN&) = delete; // Prevent assignment

        // Private constructor to enforce singleton pattern

    
        static inline RVC_DGN processPacket(CAN_frame_t *packet, int8_t startBit, int8_t numBits) {
            // Process the packet and return the DGN value
            RVC_DGN dgn = ERROR;
            if ((packet != nullptr)) { // && (packet->FIR.B.FF == CAN_frame_std)) {
                dgn = (RVC_DGN) PacketKit::getMsgBits(packet->MsgID, startBit, numBits);
            }
            return dgn; // Return ERROR if not a standard frame
        }
        
    public:
    
        // Destructor
        virtual ~DGN() = default;

        static inline DGN& getInstance() {
            // std::lock_guard<std::mutex> lock(dgnMutex);
            if (!instance) {
                instance = new DGN();
            }
            // std::lock_guard<std::mutex> unlock(dgnMutex);
            return *instance;
        }
        
        // maybe void sendCommand(uint32_t command);

        static inline RVC_DGN getDGN(CAN_frame_t *packet) {
            return processPacket(packet, DGN_START_BIT, DGN_NUM_BITS);
        }

        static inline const uint8_t startBit(void) {return DGN_START_BIT; }
        static inline const uint8_t numBits(void) { return DGN_NUM_BITS; }

        
        static void sendCommand(uint32_t command);
};

#endif //DGN_H
