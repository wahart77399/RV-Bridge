#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef BATTERY_DEFINITIONS_H
#define BATTERY_DEFINITIONS_H
#include "Arduino.h"

// Battery monitoring is performed by reading the DGN of the DC_SOURCE_STATUS_1, 2, 3, and 4, since the raw data from these packets are considerably different, 
// we will add 3 more data vectors. Thus setData will be more complex and based on the DGN. There are 12 DC_SOURCE_STATUS, but Newmar only seems to use the first
// few, so I limited the count to just the first 4.
typedef enum {
    DC_SOURCE_INSTANCE_INDEX             = 0, // all DC_SOURCE_STATUS
    DC_SOURCE_PRIORITY_INDEX             = 1, // all DC_SOURCE_STAtua
    DC_SOURCE_VOLTAGE_MSB_INDEX          = 2, // from DC_SOURCE_STATUS_1
    DC_SOURCE_VOLTAGE_LSB_INDEX          = 3, // from DC_SOURCE_STATUS_1
    DC_SOURCE_CURRENT_MSB_1_INDEX        = 4, // from DC_SOURCE_STATUS_1
    DC_SOURCE_CURRENT_MSB_2_INDEX        = 5, // from DC_SOURCE_STATUS_1
    DC_SOURCE_CURRENT_LSB_1_INDEX        = 6, // from DC_SOURCE_STATUS_1
    DC_SOURCE_CURRENT_LSB_2_INDEX        = 7, // from DC_SOURCE_STATUS_1
    DC_SOURCE_TEMPERATURE_MSB_INDEX      = 2, // from DC_SOURCE_STATUS_2
    DC_SOURCE_TEMPERATURE_LSB_INDEX      = 3, // from DC_SOURCE_STATUS_2
    DC_SOURCE_STATE_OF_CHARGE_INDEX      = 4, // from DC_SOURCE_STATUS_2
    DC_SOURCE_TIME_REMAINING_MSB_INDEX   = 5, // from DC_SOURCE_STATUS_2
    DC_SOURCE_TIME_REMAINING_LSB_INDEX   = 6, // from DC_SOURCE_STATUS_2
    DC_SOURCE_TIME_INTERPRETATION        = 7, // from DC_SOURCE_STATUS_2
    DC_SOURCE_HEALTH_STATE_INDEX         = 2, // from DC_SOURCE_STATUS_3
    DC_SOURCE_CAP_REMAINS_MSB_INDEX      = 3, // from DC_SOURCE_STATUS_3
    DC_SOURCE_CAP_REMAINS_LSB_INDEX      = 4, // from DC_SOURCE_STATUS_3
    DC_SOURCE_RELATIVE_CAP_INDEX         = 5, // from DC_SOURCE_STATUS_3
    DC_SOURCE_AC_RIPPLE_MSB_INDEX        = 6, // from DC_SOURCE_STATUS_3
    DC_SOURCE_AC_RIPPLE_LSB_INDEX        = 7, // from DC_SOURCE_STATUS_3
    DC_SOURCE_DESORED_CHARGE_STATE_INDEX = 2, // from DC_SOURCE_STATUS_4
    DC_SOURCE_DESIRED_VOLT_MSB_INDEX     = 3, // from DC_SOURCE_STATUS_4
    DC_SOURCE_DESIRED_VOLT_LSB_INDEX     = 4, // from DC_SOURCE_STATUS_4
    DC_SOURCE_DESIRED_AMP_MSB_INDEX      = 5, // from DC_SOURCE_STATUS_4
    DC_SOURCE_DESIRED_AMP_LSB_INDEX      = 6, // from DC_SOURCE_STATUS_4
    DC_SOURCE_BATTERY_TYPE_INDEX         = 7, // from DC_SOURCE_STATUS_4
    DC_SOURCE_FAULT_INDEX                = 0xf
} DC_SOURCE_DATA_INDECES;

typedef enum {
    INVALID_SOURCE            = 0,
    MAIN_HOUSE_BATTERY        = 1,
    CHASSIS_START_BATTERY     = 2,
    SECONDARY_HOUSE_BATTERY   = 3,
    GENERATOR_STARTER_BATTERY = 4,
    MFG_OTHER                 
} DC_SOURCE_INSTANCE_DEFINITION;

typedef enum {
    BATTERY_SOC_BMS_DEVICE = 120,
    INVERTER_CHARGER       = 100,
    CHARGER                = 80,
    INVERTER               = 60,
    MULTI_METER            = 40,
    VOLT_METER             = 20,
    NO_PRIORITY_ALWAYS_RPT = 0
} DC_SOURCE_PRIORITY_DEFINTION;

typedef enum {
    UNDEFINED_CHARGE_STATE = 0,
    DO_NOT_CHARGE_STATE    = 1,
    BULK_CHARGE_STATE      = 2,
    ABSORPTION_STATE       = 3,
    OVERCHARGE_STATE       = 4,
    EQUALIZE_STATE         = 5,
    FLOAT_STATE            = 6,
    CONSTANT_VOLTAGE_STATE = 7
} CHARGE_STATE;

typedef enum {
    FLOODED_BATTERY = 0,
    GEL_BATTERY     = 1,
    AGM_BATTERY     = 2,
    LITHIUM_BATTERY = 3,
    VENDOR_DEFINED
} BATTERY_TYPE;

const float_t VDC_PRECISION = 0.05;
const float_t ADC_PRECISION = 0.001;
const uint16_t VDC_MAX = 3213;
const uint32_t ADC_MAX = 2221082;
const uint16_t VDC_OFFSET = 0;
const float_t ADC_OFFSET = 2000000;
const uint32_t ADC_ZERO = 0x77359400;

#endif
#endif // ifdef HOME_KIT_2