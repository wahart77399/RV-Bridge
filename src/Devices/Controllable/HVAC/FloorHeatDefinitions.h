#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef FLOOR_HEAT_DEFINITIONS_H
#define FLOOR_HEAT_DEFINITIONS_H    


#include "Arduino.h"
constexpr uint8_t FLOOR_HEAT_INSTANCE_INDEX                         = 0;
constexpr uint8_t FLOOR_HEAT_GEN_STATUS_INDEX                       = 1;
constexpr uint8_t FLOOR_HEAT_STATUS_MEASURED_TEMPERATURE_MSB_INDEX  = 2;
constexpr uint8_t FLOOR_HEAT_STATUS_MEASURED_TEMPERATURE_LSB_INDEX  = 3;
constexpr uint8_t FLOOR_HEAT_STATUS_SET_TEMPERATURE_MSB_INDEX       = 4;
constexpr uint8_t FLOOR_HEAT_STATUS_SET_TEMPERATURE_LSB_INDEX       = 5;
constexpr uint8_t FLOOR_HEAT_COMMAND_SET_TEMPERATURE_MSB_INDEX      = 2;
constexpr uint8_t FLOOR_HEAT_COMMAND_SET_TEMPERATURE_LSB_INDEX      = 3;
constexpr uint8_t FLOOR_HEAT_INVALID_BYTE                           = 0xff;
constexpr uint8_t FLOOR_HEAT_COMMAND_OFF                            = 0xf3;  // 1111 0011
constexpr uint8_t FLOOR_HEAT_COMMAND_ON                             = 0xf7;  // 1111 0111
constexpr uint8_t FLOOR_HEAT_STATUS_OFF                             = 0x00;
constexpr uint8_t FLOOR_HEAT_STATUS_ON                              = 0x04;  // xxxx 01xx

typedef enum {
    FLOOR_HEAT_MODE_AUTO                                            = 0x00, // xxxx xx00
    FLOOR_HEAT_MODE_MANUAL                                          = 0x01, // xxxx xx01 our coach only runs in MANUAL mode
    FLOOR_HEAT_MODE_MASK                                            = 0x03  // xxxx xx11
} FLOOR_HEAT_OPERATING_MODE;

typedef enum {
    FLOOR_HEAT_GEN_STATUS_HEATING_OFF                               = 0x00, // xxxx 00xx
    FLOOR_HEAT_GEN_STATUS_HEATING_ON                                = 0x04, // xxxx 01xx
    FLOOR_HEAT_GEN_STATUS_MASK                                      = 0x0c  // xxxx 11xx
} FLOOR_HEAT_OPERATING_STATUS;

typedef enum {
    FLOOR_HEAT_ELEMENT_STATUS_OFF                                   = 0x00, // xx00 xxxx
    FLOOR_HEAT_ELEMENT_STATUS_ON                                    = 0x10, // xx01 xxxx
    FLOOR_HEAT_ELEMENT_STATUS_MASK                                  = 0x30  // xx11 xxxx
} FLOOR_HEAT_ELEMENT_STATUS;

typedef enum {
    FLOOR_HEAT_SCHEDULE_MODE_DISABLED                               = 0x00, // 00xx xxxx
    FLOOR_HEAT_SCHEDULE_MODE_ENABLED                                = 0x40, // 01xx xxxx
    FLOOR_HEAT_SCHEDULE_MODE_MASK                                   = 0xc0  // 11xx xxxx
} FLOOR_HEAT_SCHEDULE_MODE;

typedef enum {
    FLOOR_HEAT_OFF                                                  = 0x00,
    FLOOR_HEAT_ON                                                   = 0x01
} FLOOR_HEAT_TIME_INTERPRETATION;

// these may need to be adjusted based on RVC output values 12 (store), 22.0, 24.5, 25.75, 27.0, 28.25, 29.5, 30.75, 32.0, 33.25, 34.5 (C)
// these are in degrees Celsius                                                          (decimal/hexidicimal)
constexpr float_t FLOOR_HEAT_TEMP_OFF                               = -273.5F; // 0.0 C.
constexpr float_t FLOOR_HEAT_TEMP_STORE                             = 12.00F; //12.0 C. (9120/23a0)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_1                           = 22.00F; // 22.5 C. (9440/24E0)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_2                           = 23.25F; // 23.25 C (9480/2508)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_3                           = 24.50F; // 24.5 C  (9520/2530)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_4                           = 25.75F; // 25.7f C (9560/2558)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_5                           = 27.00F; // 27 C.   (9600/2580)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_6                           = 28.25F; // 28.25 C (9640/25a8)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_7                           = 29.50F; // 29.5 C. (9680/25d0)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_8                           = 30.75F; // 30.75 C (9720/25F8)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_9                           = 32.00F; // 32 C    (9760/2620)
constexpr float_t FLOOR_HEAT_TEMP_LEVEL_10                          = 33.25F; // 33.25 C (9800/2648)      


#endif  // ifndef FLOOR_HEAT_DEFINITIONS_H
#endif // ifdef HOME_KIT_2