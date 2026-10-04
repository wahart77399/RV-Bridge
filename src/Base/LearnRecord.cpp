#include "LearnRecord.h"
#include <string.h>

namespace {
    // DC_DIMMER_STATUS_3 byte 2 is brightness in 0.5% steps (200 = 100%)
    constexpr uint8_t DIMMER_LEVEL_INDEX   = 2;
    constexpr uint8_t DIMMER_PARTIAL_LOW   = 10;
    constexpr uint8_t DIMMER_PARTIAL_HIGH  = 190;
    constexpr uint8_t UNKNOWN_INSTANCE     = 0xFF;
}

LearnTable::LearnTable() : count_(0) {
    reset();
}

void LearnTable::reset() {
    memset(rows_, 0, sizeof(rows_));
    count_ = 0;
}

LearnedKind LearnTable::classify(RVC_DGN dgn) {
    LearnedKind kind = LearnedKind::Unknown;
    switch (dgn) {
        case DC_DIMMER_STATUS_3:          kind = LearnedKind::Light;      break;
        case AWNING_STATUS:               kind = LearnedKind::Awning;     break;
        case WINDOW_SHADE_CONTROL_STATUS: kind = LearnedKind::Shade;      break;
        case TANK_STATUS:                 kind = LearnedKind::Tank;       break;
        case THERMOSTAT_STATUS_1:         kind = LearnedKind::Thermostat; break;
        case FLOOR_HEAT_STATUS:           kind = LearnedKind::FloorHeat;  break;
        case WATER_PUMP_STATUS:           kind = LearnedKind::WaterPump;  break;
        case LOCK_STATUS:                 kind = LearnedKind::DoorLock;   break;
        case GENERATOR_AC_STATUS_1:       kind = LearnedKind::Generator;  break;
        case INVERTER_AC_STATUS_1:
        case INVERTER_STATUS:             kind = LearnedKind::Inverter;   break;
        case CHARGER_STATUS:              kind = LearnedKind::Charger;    break;
        case ATS_AC_STATUS_1:             kind = LearnedKind::Ats;        break;
        case DC_SOURCE_STATUS_1:          kind = LearnedKind::Battery;    break;
        default:                                                          break;
    }
    return kind;
}

namespace {
    struct RelatedDgn {
        uint32_t dgn;
        const char* name;
        const char* family;
    };

    constexpr RelatedDgn relatedDgns[] = {
        {DC_DIMMER_STATUS_3, "DC_DIMMER_STATUS_3", "Switches"},
        {AWNING_STATUS, "AWNING_STATUS", "Awning"},
        {WINDOW_SHADE_CONTROL_STATUS, "WINDOW_SHADE_CONTROL_STATUS", "Shades"},
        {TANK_STATUS, "TANK_STATUS", "Tanks"},
        {FLOOR_HEAT_STATUS, "FLOOR_HEAT_STATUS", "FloorHeat"},
        {WATER_PUMP_STATUS, "WATER_PUMP_STATUS", "WaterPump"},
        {LOCK_STATUS, "LOCK_STATUS", "DoorLock"},
        {GENERATOR_AC_STATUS_1, "GENERATOR_AC_STATUS_1", "Generator"},
        {INVERTER_AC_STATUS_1, "INVERTER_AC_STATUS_1", "Inverter"},
        {INVERTER_STATUS, "INVERTER_STATUS", "Inverter"},
        {CHARGER_STATUS, "CHARGER_STATUS", "Charger"},
        {DC_SOURCE_STATUS_1, "DC_SOURCE_STATUS_1", "Battery"},
        {ATS_STATUS, "ATS_STATUS", "ATS"},
        {ATS_COMMAND, "ATS_COMMAND", "ATS"},
        {ATS_AC_STATUS_1, "ATS_AC_STATUS_1", "ATS"},
        {ATS_AC_STATUS_2, "ATS_AC_STATUS_2", "ATS"},
        {ATS_AC_STATUS_3, "ATS_AC_STATUS_3", "ATS"},
        {ATS_AC_STATUS_4, "ATS_AC_STATUS_4", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_STATUS_1, "ATS_ACFAULT_CONFIGURATION_STATUS_1", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_STATUS_2, "ATS_ACFAULT_CONFIGURATION_STATUS_2", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_COMMAND_1, "ATS_ACFAULT_CONFIGURATION_COMMAND_1", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_COMMAND_2, "ATS_ACFAULT_CONFIGURATION_COMMAND_2", "ATS"},
        {THERMOSTAT_AMBIENT_STATUS, "THERMOSTAT_AMBIENT_STATUS", "Thermostat"},
        {THERMOSTAT_STATUS_1, "THERMOSTAT_STATUS_1", "Thermostat"},
        {THERMOSTAT_STATUS_2, "THERMOSTAT_STATUS_2", "Thermostat"},
        {THERMOSTAT_COMMAND_1, "THERMOSTAT_COMMAND_1", "Thermostat"},
        {THERMOSTAT_COMMAND_2, "THERMOSTAT_COMMAND_2", "Thermostat"},
        {THERMOSTAT_SCHEDULE_STATUS_1, "THERMOSTAT_SCHEDULE_STATUS_1", "Thermostat"},
        {THERMOSTAT_SCHEDULE_COMMAND_1, "THERMOSTAT_SCHEDULE_COMMAND_1", "Thermostat"},
        {THERMOSTAT_SCHEDULE_COMMAND_2, "THERMOSTAT_SCHEDULE_COMMAND_2", "Thermostat"},
        {0x1FFFF, "DATE_TIME_STATUS", "Clock"},
        {0x1FFFE, "SET_DATE_TIME_COMMAND", "Clock"},
        {0x1FFFC, "DC_SOURCE_STATUS_2", "Electrical"},
        {0x1FFFB, "DC_SOURCE_STATUS_3", "Electrical"},
        {0x1FFFA, "COMMUNICATION_STATUS_1", "Communication"},
        {0x1FFF9, "COMMUNICATION_STATUS_2", "Communication"},
        {0x1FFF8, "COMMUNICATION_STATUS_3", "Communication"},
        {0x1FFF7, "WATERHEATER_STATUS", "Plumbing"},
        {0x1FFF6, "WATERHEATER_COMMAND", "Plumbing"},
        {0x1FFF5, "GAS_SENSOR_STATUS", "Safety"},
        {0x1FFF4, "CHASSIS_MOBILITY_STATUS", "Chassis"},
        {0x1FFF3, "CHASSIS_MOBILITY_COMMAND", "Chassis"},
        {0x1FFF1, "AAS_COMMAND", "Moving Devices"},
        {0x1FFF0, "AAS_STATUS", "Moving Devices"},
        {0x1FFEF, "AAS_SENSOR_STATUS", "Moving Devices"},
        {0x1FFEE, "LEVELING_CONTROL_COMMAND", "Moving Devices"},
        {0x1FFED, "LEVELING_CONTROL_STATUS", "Moving Devices"},
        {0x1FFEC, "LEVELING_JACK_STATUS", "Moving Devices"},
        {0x1FFEB, "LEVELING_SENSOR_STATUS", "Moving Devices"},
        {0x1FFEA, "HYDRAULIC_PUMP_STATUS", "Moving Devices"},
        {0x1FFE9, "LEVELING_AIR_STATUS", "Moving Devices"},
        {0x1FFE8, "SLIDE_STATUS", "Moving Devices"},
        {0x1FFE7, "SLIDE_COMMAND", "Moving Devices"},
        {0x1FFE6, "SLIDE_SENSOR_STATUS", "Moving Devices"},
        {0x1FFE5, "SLIDE_MOTOR_STATUS", "Moving Devices"},
        {0x1FFE4, "FURNACE_STATUS", "Climate"},
        {0x1FFE3, "FURNACE_COMMAND", "Climate"},
        {0x1FFE1, "AIR_CONDITIONER_STATUS", "Climate"},
        {0x1FFE0, "AIR_CONDITIONER_COMMAND", "Climate"},
        {0x1FFDE, "GENERATOR_AC_STATUS_2", "Generator"},
        {0x1FFDD, "GENERATOR_AC_STATUS_3", "Generator"},
        {0x1FFDC, "GENERATOR_STATUS_1", "Generator"},
        {0x1FFDB, "GENERATOR_STATUS_2", "Generator"},
        {0x1FFDA, "GENERATOR_COMMAND", "Generator"},
        {0x1FFD9, "GENERATOR_START_CONFIG_STATUS", "Generator"},
        {0x1FFD8, "GENERATOR_START_CONFIG_COMMAND", "Generator"},
        {0x1FFD6, "INVERTER_AC_STATUS_2", "Electrical"},
        {0x1FFD5, "INVERTER_AC_STATUS_3", "Electrical"},
        {0x1FFD3, "INVERTER_COMMAND", "Electrical"},
        {0x1FFD2, "INVERTER_CONFIGURATION_STATUS_1", "Electrical"},
        {0x1FFD1, "INVERTER_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FFD0, "INVERTER_CONFIGURATION_COMMAND_1", "Electrical"},
        {0x1FFCF, "INVERTER_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FFCE, "INVERTER_STATISTICS_STATUS", "Electrical"},
        {0x1FFCD, "INVERTER_APS_STATUS", "Electrical"},
        {0x1FFCC, "INVERTER_DCBUS_STATUS", "Electrical"},
        {0x1FFCB, "INVERTER_OPS_STATUS", "Electrical"},
        {0x1FFCA, "CHARGER_AC_STATUS_1", "Electrical"},
        {0x1FFC9, "CHARGER_AC_STATUS_2", "Electrical"},
        {0x1FFC8, "CHARGER_AC_STATUS_3", "Electrical"},
        {0x1FFC6, "CHARGER_CONFIGURATION_STATUS", "Electrical"},
        {0x1FFC5, "CHARGER_COMMAND", "Electrical"},
        {0x1FFC4, "CHARGER_CONFIGURATION_COMMAND", "Electrical"},
        {0x1FFC2, "CHARGER_APS_STATUS", "Electrical"},
        {0x1FFC1, "CHARGER_DCBUS_STATUS", "Electrical"},
        {0x1FFC0, "CHARGER_OPS_STATUS", "Electrical"},
        {0x1FFBF, "AC_LOAD_STATUS", "Electrical"},
        {0x1FFBE, "AC_LOAD_COMMAND", "Electrical"},
        {0x1FFBD, "DC_LOAD_STATUS", "Electrical"},
        {0x1FFBC, "DC_LOAD_COMMAND", "Electrical"},
        {0x1FFBB, "DC_DIMMER_STATUS_1", "Lighting"},
        {0x1FFBA, "DC_DIMMER_STATUS_2", "Lighting"},
        {0x1FFB9, "DC_DIMMER_COMMAND", "Lighting"},
        {0x1FFB8, "DIGITAL_INPUT_STATUS", "Lighting"},
        {0x1FFB6, "TANK_CALIBRATION_COMMAND", "Plumbing"},
        {0x1FFB5, "TANK_GEOMETRY_STATUS", "Plumbing"},
        {0x1FFB4, "TANK_GEOMETRY_COMMAND", "Plumbing"},
        {0x1FFB2, "WATER_PUMP_COMMAND", "WaterPump"},
        {0x1FFB1, "AUTOFILL_STATUS", "Plumbing"},
        {0x1FFB0, "AUTOFILL_COMMAND", "Plumbing"},
        {0x1FFAF, "WASTEDUMP_STATUS", "Plumbing"},
        {0x1FFAE, "WASTEDUMP_COMMAND", "Plumbing"},
        {0x1FFA5, "WEATHER_STATUS_1", "Weather"},
        {0x1FFA4, "WEATHER_STATUS_2", "Weather"},
        {0x1FFA3, "ALTIMETER_STATUS", "Weather"},
        {0x1FFA2, "ALTIMETER_COMMAND", "Weather"},
        {0x1FFA1, "WEATHER_CALIBRATE_COMMAND", "Weather"},
        {0x1FFA0, "COMPASS_BEARING_STATUS", "Weather"},
        {0x1FF9F, "COMPASS_CALIBRATE_COMMAND", "Weather"},
        {0x1FF9B, "HEAT_PUMP_STATUS", "Climate"},
        {0x1FF9A, "HEAT_PUMP_COMMAND", "Climate"},
        {0x1FF99, "CHARGER_EQUALIZATION_STATUS", "Electrical"},
        {0x1FF98, "CHARGER_EQUALIZATION_CONFIGURATION_STATUS", "Electrical"},
        {0x1FF97, "CHARGER_EQUALIZATION_CONFIGURATION_COMMAND", "Electrical"},
        {0x1FF96, "CHARGER_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FF95, "CHARGER_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FF94, "GENERATOR_AC_STATUS_4", "Generator"},
        {0x1FF93, "GENERATOR_ACFAULT_CONFIGURATION_STATUS_1", "Generator"},
        {0x1FF92, "GENERATOR_ACFAULT_CONFIGURATION_STATUS_2", "Generator"},
        {0x1FF91, "GENERATOR_ACFAULT_CONFIGURATION_COMMAND_1", "Generator"},
        {0x1FF90, "GENERATOR_ACFAULT_CONFIGURATION_COMMAND_2", "Generator"},
        {0x1FF8F, "INVERTER_AC_STATUS_4", "Electrical"},
        {0x1FF8E, "INVERTER_ACFAULT_CONFIGURATION_STATUS_1", "Electrical"},
        {0x1FF8D, "INVERTER_ACFAULT_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FF8C, "INVERTER_ACFAULT_CONFIGURATION_COMMAND_1", "Electrical"},
        {0x1FF8B, "INVERTER_ACFAULT_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FF8A, "CHARGER_AC_STATUS_4", "Electrical"},
        {0x1FF89, "CHARGER_ACFAULT_CONFIGURATION_STATUS_1", "Electrical"},
        {0x1FF88, "CHARGER_ACFAULT_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FF87, "CHARGER_ACFAULT_CONFIGURATION_COMMAND_1", "Electrical"},
        {0x1FF86, "CHARGER_ACFAULT_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FF80, "GENERATOR_DEMAND_STATUS", "Generator"},
        {0x1FEFF, "GENERATOR_DEMAND_COMMAND", "Generator"},
        {0x1FEFE, "AGS_CRITERION_STATUS", "Generator"},
        {0x1FEFD, "AGS_CRITERION_COMMAND", "Generator"},
        {0x1FEFB, "FLOOR_HEAT_COMMAND", "Climate"},
        {0x1FEF6, "THERMOSTAT_SCHEDULE_STATUS_2", "Thermostat"},
        {0x1FEF3, "AWNING_STATUS", "Awning"},
        {0x1FEF2, "AWNING_COMMAND", "Awning"},
        {0x1FEF1, "TIRE_RAW_STATUS", "Chassis"},
        {0x1FEF0, "TIRE_STATUS", "Chassis"},
        {0x1FEE9, "TIRE_ID_COMMAND", "Chassis"},
        {0x1FEE8, "INVERTER_DC_STATUS", "Electrical"},
        {0x1FEE7, "GENERATOR_DEMAND_CONFIGURATION_STATUS", "Generator"},
        {0x1FEE6, "GENERATOR_DEMAND_CONFIGURATION_COMMAND", "Generator"},
        {0x1FEEF, "TIRE_SLOW_LEAK_ALARM", "Chassis"},
        {0x1FEEE, "TIRE_TEMPERATURE_CONFIGURATION_STATUS", "Chassis"},
        {0x1FEED, "TIRE_PRESSURE_CONFIGURATION_STATUS", "Chassis"},
        {0x1FEEC, "TIRE_PRESSURE_CONFIGURATION_COMMAND", "Chassis"},
        {0x1FEEB, "TIRE_TEMPERATURE_CONFIGURATION_COMMAND", "Chassis"},
        {0x1FEEA, "TIRE_ID_STATUS", "Chassis"},
        {0x1FEE9, "TIRE_ID_COMMAND", "Chassis"},
        {0x1FEE4, "LOCK_COMMAND", "DoorLock"},
        {0x1FEE3, "WINDOW_STATUS", "Moving Devices"},
        {0x1FEE2, "WINDOW_COMMAND", "Moving Devices"},
        {0x1FEE1, "DC_MOTOR_CONTROL_COMMAND", "Moving Devices"},
        {0x1FEE0, "DC_MOTOR_CONTROL_STATUS", "Moving Devices"},
        {0x1FEDF, "WINDOW_SHADE_CONTROL_COMMAND", "Shades"},
        {0x1FEDD, "AC_LOAD_STATUS_2", "Electrical"},
        {0x1FEDC, "DC_LOAD_STATUS_2", "Electrical"},
        {0x1FEDB, "DC_DIMMER_COMMAND_2", "Lighting"},
        {0x1FED9, "GENERIC_INDICATOR_COMMAND", "Lighting"},
        {0x1FED8, "GENERIC_CONFIGURATION_STATUS", "RV-C"},
        {0x1FED7, "GENERIC_INDICATOR_STATUS", "Lighting"},
        {0x1FED6, "MFG_SPECIFIC_CLAIM_REQUEST", "RV-C"},
        {0x1FED5, "AGS_DEMAND_CONFIGURATION_STATUS", "Generator"},
        {0x1FED4, "AGS_DEMAND_CONFIGURATION_COMMAND", "Generator"},
        {0x1FED3, "DEPRECATED_GPS_STATUS", "Clock"},
        {0x1FED2, "AGS_CRITERION_STATUS_2", "Generator"},
        {0x1FED1, "SUSPENSION_AIR_PRESSURE_STATUS", "Chassis"},
        {0x1FED0, "DC_DISCONNECT_STATUS", "Electrical"},
        {0x1FECF, "DC_DISCONNECT_COMMAND", "Electrical"},
        {0x1FECE, "INVERTER_CONFIGURATION_STATUS_3", "Electrical"},
        {0x1FECD, "INVERTER_CONFIGURATION_COMMAND_3", "Electrical"},
        {0x1FECC, "CHARGER_CONFIGURATION_STATUS_3", "Electrical"},
        {0x1FECB, "CHARGER_CONFIGURATION_COMMAND_3", "Electrical"},
        {0x1FECA, "DM-RV", "RV-C"},
        {0x1FEC9, "DC_SOURCE_STATUS_4", "Electrical"},
        {0x1FEC8, "DC_SOURCE_STATUS_5", "Electrical"},
        {0x1FEC7, "DC_SOURCE_STATUS_6", "Electrical"},
        {0x1FEC6, "GENERATOR_DC_STATUS_1", "Generator"},
        {0x1FEC5, "GENERATOR_DC_CONFIGURATION_STATUS", "Generator"},
        {0x1FEC4, "GENERATOR_DC_COMMAND", "Generator"},
        {0x1FEC3, "GENERATOR_DC_CONFIGURATION_COMMAND", "Generator"},
        {0x1FEC2, "GENERATOR_DC_EQUALIZATION_STATUS", "Generator"},
        {0x1FEC1, "GENERATOR_DC_EQUALIZATION_CONFIGURATION_STATUS", "Generator"},
        {0x1FEC0, "GENERATOR_DC_EQUALIZATION_CONFIGURATION_COMMAND", "Generator"},
        {0x1FEBF, "CHARGER_CONFIGURATION_STATUS_4", "Electrical"},
        {0x1FEBE, "CHARGER_CONFIGURATION_COMMAND_4", "Electrical"},
        {0x1FEBD, "INVERTER_TEMPERATURE_STATUS", "Electrical"},
        {0x1FEBC, "HYDRAULIC_PUMP_COMMAND", "Moving Devices"},
        {0x1FEBB, "GENERIC_AC_STATUS_1", "Electrical"},
        {0x1FEBA, "GENERIC_AC_STATUS_2", "Electrical"},
        {0x1FEB9, "GENERIC_AC_STATUS_3", "Electrical"},
        {0x1FEB8, "GENERIC_AC_STATUS_4", "Electrical"},
        {0x1FEB7, "GENERIC_ACFAULT_CONFIGURATION_STATUS_1", "Electrical"},
        {0x1FEB6, "GENERIC_ACFAULT_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FEB5, "GENERIC_ACFAULT_CONFIGURATION_COMMAND_1", "Electrical"},
        {0x1FEB4, "GENERIC_ACFAULT_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FEB3, "SOLAR_CONTROLLER_STATUS_1", "Electrical"},
        {0x1FEB2, "SOLAR_CONTROLLER_CONFIGURATION_STATUS", "Electrical"},
        {0x1FEB1, "SOLAR_CONTROLLER_COMMAND", "Electrical"},
        {0x1FEB0, "SOLAR_CONTROLLER_CONFIGURATION_COMMAND", "Electrical"},
        {0x1FEAF, "SOLAR_EQUALIZATION_STATUS", "Electrical"},
        {0x1FEAE, "SOLAR_EQUALIZATION_CONFIGURATION_STATUS", "Electrical"},
        {0x1FEAD, "SOLAR_EQUALIZATION_CONFIGURATION_COMMAND", "Electrical"},
        {0x1FEAC, "DC_SOURCE_STATUS_7", "Electrical"},
        {0x1FEAB, "DC_SOURCE_STATUS_8", "Electrical"},
        {0x1FEAA, "DC_SOURCE_STATUS_9", "Electrical"},
        {0x1FEA9, "DC_SOURCE_STATUS_10", "Electrical"},
        {0x1FEA8, "CHASSIS_MOBILITY_STATUS_2", "Chassis"},
        {0x1FEA7, "ROOF_FAN_STATUS_1", "Climate"},
        {0x1FEA6, "ROOF_FAN_COMMAND_1", "Climate"},
        {0x1FEA5, "DC_SOURCE_STATUS_11", "Electrical"},
        {0x1FEA4, "DC_SOURCE_COMMAND", "Electrical"},
        {0x1FEA3, "CHARGER_STATUS_2", "Electrical"},
        {0x1FEA2, "CHARGER_CONFIGURATION_COMMAND_5", "Electrical"},
        {0x1FEA1, "CHARGER_CONFIGURATION_STATUS_5", "Electrical"},
        {0x1FEA0, "GPS_DATE_TIME_STATUS", "Clock"},
        {0x1FE9F, "GENERIC_ALARM_STATUS", "Safety"},
        {0x1FE9E, "GENERIC_ALARM_COMMAND", "Safety"},
        {0x1FE9B, "INVERTER_CONFIGURATION_STATUS_4", "Electrical"},
        {0x1FE9A, "INVERTER_CONFIGURATION_COMMAND_4", "Electrical"},
        {0x1FE99, "WATERHEATER_STATUS_2", "Plumbing"},
        {0x1FE98, "WATERHEATER_COMMAND_2", "Plumbing"},
        {0x1FE97, "CIRCULATION_PUMP_STATUS", "Plumbing"},
        {0x1FE96, "CIRCULATION_PUMP_COMMAND", "Plumbing"},
        {0x1FE95, "BATTERY_STATUS_1", "Electrical"},
        {0x1FE94, "BATTERY_STATUS_2", "Electrical"},
        {0x1FE93, "BATTERY_STATUS_3", "Electrical"},
        {0x1FE92, "BATTERY_STATUS_4", "Electrical"},
        {0x1FE91, "BATTERY_STATUS_5", "Electrical"},
        {0x1FE90, "BATTERY_STATUS_6", "Electrical"},
        {0x1FE8F, "BATTERY_STATUS_7", "Electrical"},
        {0x1FE8E, "BATTERY_STATUS_8", "Electrical"},
        {0x1FE8D, "BATTERY_STATUS_9", "Electrical"},
        {0x1FE8C, "BATTERY_STATUS_10", "Electrical"},
        {0x1FE8B, "BATTERY_STATUS_11", "Electrical"},
        {0x1FE8A, "BATTERY_COMMAND", "Electrical"},
        {0x1FE89, "STEP_STATUS", "Moving Devices"},
        {0x1FE88, "STEP_COMMAND", "Moving Devices"},
        {0x1FE87, "VEHICLE_ENVIRONMENT_STATUS", "Chassis"},
        {0x1FE86, "VEHICLE_ENVIRONMENT_COMMAND", "Chassis"},
        {0x1FE85, "SOLAR_CONTROLLER_STATUS_2", "Electrical"},
        {0x1FE84, "SOLAR_CONTROLLER_STATUS_3", "Electrical"},
        {0x1FE83, "SOLAR_CONTROLLER_STATUS_4", "Electrical"},
        {0x1FE82, "SOLAR_CONTROLLER_STATUS_5", "Electrical"},
        {0x1FE81, "SOLAR_CONTROLLER_STATUS_6", "Electrical"},
        {0x1FE80, "SOLAR_CONTROLLER_BATTERY_STATUS", "Electrical"},
        {0x1FDFF, "SOLAR_CONTROLLER_SOLAR_ARRAY_STATUS", "Electrical"},
        {0x1FDFE, "SOLAR_CONTROLLER_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FDFD, "SOLAR_CONTROLLER_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FDFC, "SOLAR_CONTROLLER_CONFIGURATION_STATUS_3", "Electrical"},
        {0x1FDFB, "SOLAR_CONTROLLER_CONFIGURATION_COMMAND_3", "Electrical"},
        {0x1FDFA, "SOLAR_CONTROLLER_CONFIGURATION_STATUS_4", "Electrical"},
        {0x1FDF9, "SOLAR_CONTROLLER_CONFIGURATION_COMMAND_4", "Electrical"},
        {0x1FDF8, "DC_SOURCE_STATUS_12", "Electrical"},
        {0x1FDF7, "DC_SOURCE_CONFIGURATION_STATUS_1", "Electrical"},
        {0x1FDF6, "DC_SOURCE_CONFIGURATION_COMMAND_1", "Electrical"},
        {0x1FDF5, "DC_SOURCE_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FDF4, "DC_SOURCE_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FDF3, "BATTERY_STATUS_12", "Electrical"},
        {0x1FDF2, "BATTERY_STATUS_13", "Electrical"},
        {0x1FDF1, "BATTERY_SUMMARY", "Electrical"},
        {0x1FDF0, "DEPRECATED_BATTERY_CONFIGURATION_COMMAND_1", "Electrical"},
        {0x1FDEF, "DEPRECATED_BATTERY_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FDEE, "DEPRECATED_BATTERY_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FDED, "TIRE_HIGH_PRESSURE_CONFIGURATION_STATUS", "Chassis"},
        {0x1FDEC, "TIRE_HIGH_PRESSURE_CONFIGURATION_COMMAND", "Chassis"},
        {0x1FDEB, "LEVELING_SENSOR_ROLL_CONFIG_STATUS", "Moving Devices"},
        {0x1FDEA, "LEVELING_SENSOR_ROLL_CONFIG_COMMAND", "Moving Devices"},
        {0x1FDE9, "LEVELING_SENSOR_PITCH_CONFIG_STATUS", "Moving Devices"},
        {0x1FDE8, "LEVELING_SENSOR_PITCH_CONFIG_COMMAND", "Moving Devices"},
        {0x1FDE7, "DC_SOURCE_STATUS_13", "Electrical"},
        {0x1FDE6, "CAN_BUS_STATUS", "Communication"},
        {0x1FDE5, "VALVE_STATUS", "Plumbing"},
        {0x1FDE4, "VALVE_COMMAND", "Plumbing"},
        {0x1FDE3, "ROOF_FAN_STATUS_2", "Climate"},
        {0x1FDE2, "ROOF_FAN_COMMAND_2", "Climate"},
        {0x1FDE1, "WEATHER_ALARM_STATUS", "Weather"},
        {0x1FDE0, "WEATHER_ALARM_COMMAND", "Weather"},
        {0x1FDDF, "GPS_TIME_STATUS", "Clock"},
        {0x1FDDE, "DC_SOURCE_CONFIGURATION_COMMAND_3", "Electrical"},
        {0x1FDDD, "CELL_DETAIL", "Electrical"},
        {0x1FDDC, "GENERATOR_DC_STATUS_2", "Generator"},
        {0x1FDD5, "DEPRECATED_GENERATOR_DC_CONFIGURATION_STATUS_5", "Generator"},
        {0x1FDD4, "DEPRECATED_GENERATOR_DC_CONFIGURATION_COMMAND_5", "Generator"},
        {0x1FDDB, "GENERATOR_DC_CONFIGURATION_STATUS_2", "Generator"},
        {0x1FDDA, "GENERATOR_DC_CONFIGURATION_COMMAND_2", "Generator"},
        {0x1FDD9, "GENERATOR_DC_CONFIGURATION_STATUS_3", "Generator"},
        {0x1FDD8, "GENERATOR_DC_CONFIGURATION_COMMAND_3", "Generator"},
        {0x1FDD7, "GENERATOR_DC_CONFIGURATION_STATUS_4", "Generator"},
        {0x1FDD6, "GENERATOR_DC_CONFIGURATION_COMMAND_4", "Generator"},
        {0x1FDD3, "REFRIGERATOR_STATUS", "Appliances"},
        {0x1FDD2, "REFRIGERATOR_COMMAND", "Appliances"},
        {0x1FDD1, "DEVICE_STATE_SYNCHRONIZATION", "RV-C"},
        {0x1FDD0, "DC_SOURCE_CONNECTION_STATUS", "Electrical"},
        {0x1FDCF, "SOLAR_CONTROLLER_CONFIGURATION_STATUS_5", "Electrical"},
        {0x1FDCE, "SOLAR_CONTROLLER_CONFIGURATION_COMMAND_5", "Electrical"},
        {0x1FDCD, "AWNING_STATUS_2", "Awning"},
        {0x1FDCC, "AWNING_COMMAND_2", "Awning"},
        {0x1FDCA, "CHARGER_STATUS_3", "Electrical"},
        {0x1FDCB, "INVERTER_TEMPERATURE_STATUS_2", "Electrical"},
        {0x1FDC9, "AIR_CONDITIONING_STATUS_2", "Climate"},
        {0x1FDC8, "VEHICLE_SEAT_COMMAND", "Moving Devices"},
        {0x1FDC7, "VEHICLE_SEAT_STATUS", "Moving Devices"},
        {0x1FDC6, "VEHICLE_SEAT_LIGHTING_COMMAND", "Moving Devices"},
        {0x1FDC5, "VEHICLE_SEAT_LIGHTING_STATUS", "Moving Devices"},
        {0x1FDC4, "TV_LIFT_STATUS", "Moving Devices"},
        {0x1FDC3, "TV_LIFT_COMMAND", "Moving Devices"},
        {0x1FDC2, "DC_LIGHTING_CONTROLLER_STATUS_1", "Lighting"},
        {0x1FDC1, "DC_LIGHTING_CONTROLLER_STATUS_2", "Lighting"},
        {0x1FDC0, "DC_LIGHTING_CONTROLLER_STATUS_3", "Lighting"},
        {0x1FDBF, "DC_LIGHTING_CONTROLLER_STATUS_4", "Lighting"},
        {0x1FDBE, "DC_LIGHTING_CONTROLLER_STATUS_5", "Lighting"},
        {0x1FDBD, "DC_LIGHTING_CONTROLLER_STATUS_6", "Lighting"},
        {0x1FDBC, "DC_LIGHTING_CONTROLLER_COMMAND_1", "Lighting"},
        {0x1FDBB, "DC_LIGHTING_CONTROLLER_COMMAND_2", "Lighting"},
        {0x1FDBA, "DC_LIGHTING_CONTROLLER_COMMAND_3", "Lighting"},
        {0x1FDB9, "DC_LIGHTING_CONTROLLER_COMMAND_4", "Lighting"},
        {0x1FDB8, "DC_LIGHTING_CONTROLLER_COMMAND_5", "Lighting"},
        {0x1FDB7, "DC_LIGHTING_CONTROLLER_COMMAND_6", "Lighting"},
        {0x1FDF0, "DEPRECATED_BATTERY_CONFIGURATION_COMMAND_1", "Electrical"},
        {0x1FDEF, "DEPRECATED_BATTERY_CONFIGURATION_STATUS_2", "Electrical"},
        {0x1FDEE, "DEPRECATED_BATTERY_CONFIGURATION_COMMAND_2", "Electrical"},
        {0x1FDD5, "DEPRECATED_GENERATOR_DC_CONFIGURATION_STATUS_5", "Generator"},
        {0x1FDD4, "DEPRECATED_GENERATOR_DC_CONFIGURATION_COMMAND_5", "Generator"},
        {0x1FED3, "DEPRECATED_GPS_STATUS", "Clock"},
        {0x0FEF3, "DEPRECATED_GPS_POSITION", "Clock"}
    };
}

const char* LearnTable::relatedFamily(RVC_DGN dgn) {
    LearnedKind kind = classify(dgn);
    const char* family = kind == LearnedKind::Unknown ? "Unknown" : label(kind);
    for (const RelatedDgn& entry : relatedDgns) {
        if (entry.dgn == dgn) {
            family = entry.family;
            break;
        }
    }
    return family;
}

const char* LearnTable::dgnName(RVC_DGN dgn) {
    const char* name = "UNKNOWN_DGN";
    for (const RelatedDgn& entry : relatedDgns) {
        if (entry.dgn == dgn) {
            name = entry.name;
            break;
        }
    }
    return name;
}

const char* LearnTable::typeName(const LearnRecord& r) {
    const char* name = "";
    switch (r.kind) {
        case LearnedKind::Light:
            name = (r.flags & LearnRecord::FLAG_PARTIAL_DIM) ? "DC_DimmableSwitch" : "DC_Switch";
            break;
        case LearnedKind::Awning:     name = "Awning";     break;
        case LearnedKind::Shade:      name = "Shades";     break;
        case LearnedKind::Tank:       name = "Tank";       break;
        case LearnedKind::Thermostat: name = "Thermostat"; break;
        case LearnedKind::FloorHeat:  name = "FloorHeat";  break;
        case LearnedKind::WaterPump:  name = "WaterPump";  break;
        case LearnedKind::DoorLock:   name = "DoorLock";   break;
        case LearnedKind::Generator:  name = "Generator";  break;
        case LearnedKind::Inverter:   name = "Inverter";   break;
        case LearnedKind::Charger:    name = "Charger";    break;
        case LearnedKind::Ats:        name = "ATS";        break;
        case LearnedKind::Battery:    name = "Battery";    break;
        default:                                           break;
    }
    return name;
}

const char* LearnTable::label(LearnedKind kind) {
    const char* text = "Device";
    switch (kind) {
        case LearnedKind::Light:      text = "Light";      break;
        case LearnedKind::Awning:     text = "Awning";     break;
        case LearnedKind::Shade:      text = "Shade";      break;
        case LearnedKind::Tank:       text = "Tank";       break;
        case LearnedKind::Thermostat: text = "Thermostat"; break;
        case LearnedKind::FloorHeat:  text = "Floor Heat"; break;
        case LearnedKind::WaterPump:  text = "Water Pump"; break;
        case LearnedKind::DoorLock:   text = "Lock";       break;
        case LearnedKind::Generator:  text = "Generator";  break;
        case LearnedKind::Inverter:   text = "Inverter";   break;
        case LearnedKind::Charger:    text = "Charger";    break;
        case LearnedKind::Ats:        text = "ATS";        break;
        case LearnedKind::Battery:    text = "Battery";    break;
        default:                                           break;
    }
    return text;
}

LearnRecord* LearnTable::findOrAlloc(RVC_DGN dgn, LearnedKind kind, uint8_t instance, uint8_t sourceAddress) {
    LearnRecord* result = nullptr;
    bool known = (kind != LearnedKind::Unknown);

    // known devices are keyed by kind+instance; unknown traffic by DGN+source
    for (size_t i = 0; (i < count_) && (result == nullptr); ++i) {
        LearnRecord& r = rows_[i];
        if (known) {
            if ((r.kind == kind) && (r.instance == instance)) {
                result = &r;
            }
        } else if ((r.kind == LearnedKind::Unknown) && (r.dgn == static_cast<uint32_t>(dgn)) &&
                   (r.sourceAddress == sourceAddress)) {
            result = &r;
        }
    }

    size_t limit = known ? MAX_RECORDS : (MAX_RECORDS - RESERVED_KNOWN);
    if ((result == nullptr) && (count_ < limit)) {
        result = &rows_[count_];
        memset(result, 0, sizeof(LearnRecord));
        result->dgn           = static_cast<uint32_t>(dgn);
        result->kind          = kind;
        result->instance      = known ? instance : UNKNOWN_INSTANCE;
        result->sourceAddress = sourceAddress;
        count_ = count_ + 1;
    }
    return result;
}

void LearnTable::observe(RVC_DGN dgn, uint8_t instance, uint8_t sourceAddress, const uint8_t* data, uint32_t nowSec) {
    if (data != nullptr) {
        LearnedKind kind = classify(dgn);
        LearnRecord* r = findOrAlloc(dgn, kind, instance, sourceAddress);
        if (r != nullptr) {
            if (r->hitCount == 0) {
                r->firstSeenSec = nowSec;
            }
            r->lastSeenSec   = nowSec;
            r->sourceAddress = sourceAddress;
            if (r->hitCount < UINT32_MAX) {
                r->hitCount = r->hitCount + 1;
            }
            memcpy(r->sample, data, sizeof(r->sample));
            r->flags |= LearnRecord::FLAG_HAS_SAMPLE;

            if (kind == LearnedKind::Light) {
                uint8_t level = data[DIMMER_LEVEL_INDEX];
                if ((level >= DIMMER_PARTIAL_LOW) && (level <= DIMMER_PARTIAL_HIGH)) {
                    r->flags |= LearnRecord::FLAG_PARTIAL_DIM;
                }
            }
        }
    }
}

bool LearnTable::load(const void* src, size_t rowCount) {
    bool ok = false;
    reset();
    if ((src != nullptr) && (rowCount <= MAX_RECORDS)) {
        memcpy(rows_, src, rowCount * sizeof(LearnRecord));
        count_ = rowCount;
        ok = true;
    }
    return ok;
}