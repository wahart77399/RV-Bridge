#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef AWNING_DEFINITIONS_H
#define AWNING_DEFINITIONS_H


#include "Arduino.h"
constexpr uint8_t AWNING_MAX_PERCENT = 100;
constexpr uint8_t AWNING_MIN_PERCENT = 0;
constexpr uint8_t AWNING_STEP = 5;
constexpr float_t AWNING_PERCENT_PRECISION = 0.5; // this is the value returned when no data is available
constexpr uint8_t AWNING_MAX_VALUE = 200; // 200 * 0.5 = 100%

constexpr uint8_t DOOR_AWNING = 1;
constexpr const char* DOOR_AWNING_NAME = "Door Awning";
constexpr uint8_t FRONT_AWNING = 2;
constexpr const char* FRONT_AWNING_NAME = "Front Awning";
constexpr uint8_t BACK_AWNING = 3;
constexpr const char* BACK_AWNING_NAME = "Back Awning";

typedef enum {
    // all indeces are zero based
    AWNING_INSTANCE_INDEX                     = 0,
    // STATUS
    AWNING_STATUS_MOTION_INDEX                = 1, 
    AWNING_STATUS_POSITION_INDEX              = 2,
    AWNING_STATUS_TRAVEL_LOCK_INDEX           = 3,
    AWNING_STATUS_PARK_INDEX                  = 4,
    AWNING_STATUS_LIGHT_INDEX                 = 5,
    AWNING_STATUS_LIGHT_DIM_INDEX             = 6,
    AWNING_STATUS_2ND_LIGNT_INDEX             = 7,
    // COMMAND
    AWNING_COMMAND_LOCK_INDEX                 = 1,
    AWNING_COMMAND_DIRECTION_INDEX            = 2,
    AWNING_COMMAND_POSITION_INDEX             = 3,
    AWNING_COMMAND_MOTION_INDEX               = 4,
    AWNING_COMMAND_LIGHT_INDEX                = 6,
    AWNING_COMMAND_2ND_LIGHT_INDEX            = 7,
    // STATUS 2
    AWNING_STATUS_2_MOTION_SENSITIVITY_INDEX  = 1,
    AWNING_STATUS_2_CALBRATION_INDEX          = 2,
    AWNING_STATUS_2_EXTEND_LOCKOUT_INDEX      = 3,
    AWNING_STATUS_2_RETRACT_LOCKOUT_INDEX     = 4,
    AWNING_STATUS_2_INPUT_STATE_INDEX         = 5,
    // COMMAND 2
    AWNING_COMMAND_2_MOTION_SENSITIVITY_INDEX = 1,
    AWNING_COMMAND_2_CALBRATION_INDEX         = 2,
    AWNING_COMMAND_2_EXTEND_LOCKOUT_INDEX     = 3,
    AWNING_COMMAND_2_RETRACT_LOCKOUT_INDEX    = 4,
    AWNING_COMMAND_2_INPUT_STATE_INDEX        = 5

} AWNING_INDECES;

typedef enum {
    INTEGRATED_LIGHT_INDEX = AWNING_COMMAND_LOCK_INDEX,
    INTEGRATED_LIGHT_MASK      = 0x30,   // 0011 0000 
    INTEGRATED_LIGHT_MASK_COMP = 0xcf,   // 1100 1111
    INTEGRATED_LIGHT_ON        = 0x10,   // 0001 0000
    INTEGRATED_LIGHT_OFF       = 0x00    // 0000 0000
} INTEGRATED_LIGHT;

// AWNING Command definitions
typedef enum {
    AWNING_STOP_COMMAND       = 0x00,
    AWNING_EXTEND_COMMAND     = 0x01,
    AWNING_RETRACT_COMMAND    = 0x02
} AWNING_COMMAND_DIRECTION;

// AWNING Command 2 definitions

// Awning Status definitions
typedef enum {
    NO_MOTION            = 0x00, // 0000 0000
    EXTENDING            = 0x01, // 0000 0001
    RETRACTING           = 0x02 // 0000 0010
} AWNING_MOTION;

constexpr uint8_t AWNING_LIGHT_MAX_PERCXENT = MAX_RVC_PERCENT * RVC_PERCENT_PRECISION;
typedef enum {
    AWNING_LIGHT_ON          = AWNING_LIGHT_MAX_PERCXENT, 
    AWNING_LIGHT_OFF         = 0x00 
} AWNING_LIGHT_STATUS;

// Awning Status 2 definitions



#endif  // AWNING_DEFINITIONS_H
#endif // HOME_KIT_2