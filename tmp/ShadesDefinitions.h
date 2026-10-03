#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef SHADES_DEFINITIONS_H
#define SHADES_DEFINITIONS_H


#include "Arduino.h"

constexpr uint8_t LIVING_ROOM_DAY_SHADE = 88;
constexpr const char* LIVING_ROOM_DAY_SHADE_NAME = "Living Room Day Shade";
constexpr uint8_t LIVING_ROOM_NIGHT_SHADE = 89;
constexpr const char* LIVING_ROOM_NIGHT_SHADE_NAME = "Living Room Night Shade";
constexpr uint8_t BEDROOM_DAY_SHADE = 90;
constexpr const char* BEDROOM_DAY_SHADE_NAME = "Bedroom Day Shade";
constexpr uint8_t BEDROOM_NIGHT_SHADE = 91;
constexpr const char* BEDROOM_NIGHT_SHADE_NAME = "Bedroom Night Shade";
constexpr uint8_t BATHROOM_DAY_SHADE = 92;
constexpr const char* BATHROOM_DAY_SHADE_NAME = "Bathroom Day Shade";
constexpr uint8_t BATHROOM_NIGHT_SHADE = 93;
constexpr const char* BATHROOM_NIGHT_SHADE_NAME = "Bathroom Night Shade";

constexpr uint8_t SHADES_MAX_PERCENT = 100;
constexpr uint8_t SHADES_MIN_PERCENT = 0;
constexpr uint8_t SHADES_STEP = 5;
constexpr float_t SHADES_PERCENT_PRECISION = 0.5; // this is the value returned when no data is available
constexpr uint8_t SHADES_MAX_VALUE = 200; // 200 * 0.5 = 100%
constexpr uint8_t SHADES_PCT_FUDGE_FACTOR = 1;

constexpr uint8_t DOOR_NIGHT_SHADE = 1;
constexpr const char* DOOR_NIGHT_SHADE_NAME = "Door Night Shade";
constexpr uint8_t FRONT_WINDOW_NIGHT_SHADE = 2;
constexpr const char* FRONT_WINDOW_NIGHT_SHADE_NAME = "Front Window Night Shade";
constexpr uint8_t DRIVERS_NIGHT_SHADE = 3;
constexpr const char* DRIVERS_NIGHT_SHADE_NAME = "Drivers Night Shade";

typedef enum {
    SHADE_INSTANCE_INDEX     = 0,
    SHADE_GROUP_INDEX        = 1,
    SHADE_OP_STATUS_INDEX    = 2, // motor duty percent
    SHADE_STATUS_INDEX       = 3,
    SHADE_DURATION_INDEX     = 4,
    SHADE_LAST_COMMAND_INDEX = 5,
    SHADE_OVER_INDEX         = 6,
    SHADE_MOTOR_DUTY_INDEX   = 2,
    SHADE_COMMAND_INDEX      = 3,
    SHADE_INTERLOCK_INDEX    = 5 
} SHADE_INDECES;

typedef enum {
    SHADE_GROUP_1 = 0x7e,  // 0111 1110
    SHADE_GROUP_2 = 0x7d,  // 0111 1101
    SHADE_GROUP_3 = 0x7b,  // 0111 1011
    SHADE_GROUP_4 = 0x77,  // 0111 0111
    SHADE_GROUP_5 = 0x6f,  // 0110 1111
    SHADE_GROUP_6 = 0x5f,  // 0101 1111
    SHADE_GROUP_7 = 0x3f,  // 0011 1111
    SHADE_GROUP_ALL = 0x00, // 0000 0000
    SHADE_GROUP_NONE = 0xFF  // 1111 1111
} SHADE_GROUP_BITMAP;

typedef enum {
    SHADE_LOCK_UNLOCKED         = 0x00,   // xxxx xx00
    SHADE_LOCK_LOCKED           = 0x01,   // xxxx xx01
    SHADE_LOCK_NA               = 0x03,   // xxxx xx11
    SHADE_LOCK_MASK             = 0x03,    // xxxx xx11
    SHADE_MOTOR_NEITHER         = 0x00,   // xxxx 00xx
    SHADE_MOTOR_EITHER          = 0x04,   // xxxx 01xx
    SHADE_MOTOR_MASK            = 0x0c,   // xxxx 11xx
    SHADE_FORWARD_NOT_ON        = 0x00,   // xx00 xxxx
    SHADE_FORWARD_ON            = 0x10,   // xx01 xxxx
    SHADE_FORWARD_MASK          = 0x30,   // xx11 xxxx
    SHADE_REVERSE_NOT_ON        = 0x00,   // 00xx xxxx
    SHADE_REVERSE_ON            = 0x40,   // 01xx xxxx
    SHADE_REVERSE_MASK          = 0xc0    // 11xx xxxx
} SHADE_STATUS;

typedef enum {
    NOT_IN_OVER_CURRENT       = 0x00,  // xxxx xx00
    IN_OVER_CURRENT           = 0x01,  // xxxx xx01
    OVER_CURRENT_NA           = 0x03,  // xxxx 0011
    OVERCURRENT_MASK          = 0x03   // xxxx xx11
} OVER_CURRENT_STATUS;

typedef enum {
    OVER_RIDE_INACTIVE = 0x00,  // xxxx 00xx
    OVER_RIDE_ACTIVE   = 0x04,  // xxxx 01xx
    OVER_RIDE_NA       = 0x0c,  // xxxx 11xx
    OVER_RIDE_MASK     = 0x0c   // xxxx 11xx
} OVER_RIDE_STATUS;

typedef enum {
    DISABLE_1_DISABLED = 0x00,  // xx00 xxxx
    DISABLE_1_ENABLED  = 0x10,  // xx01 xxxx
    DISABLE_1_NA       = 0x30,  // xx11 xxxx
    DISABLE_1_MASK     = 0x30   // xx11 xxxx
} DISABLE_1_STATUS;

typedef enum {
    DISABLE_2_DISABLED = 0x00,  // 00xx xxxx
    DISABLE_2_ENABLED  = 0x40,  // 01xx xxxx
    DISABLE_2_NA       = 0xc0,  // 11xx xxxx
    DISABLE_2_MASK     = 0xc0   // 11xx xxxx
} DISABLE_2_STATUS;


typedef enum {
    SHADE_COMMAND_REVERSE         = 0x41,
    SHADE_COMMAND_FORWARD         = 0x81,
    SHADE_COMMAND_STOP            = 0x04,
    SHADE_COMMAND_LOCK            = 0x21,
    SHADE_COMMAND_UNLOCK          = 0x22,
    SHADE_COMMAND_TILT            = 0x10, // not supported yet
    SHADE_COMMAND_TOGGLE_FORWARD  = 0x85,
    SHADE_COMMAND_TOGGLE_REVERSE  = 0x45
} SHADES_COMMANDS;

// Shade Status definitions
typedef enum {
    STOPPED              = 0x00, // 0000 0000
    CLOSING              = 0x01, // 0000 0001
    OPENING              = 0x02 // 0000 0010
} SHADE_MOTION;

#endif // SHADES_DEFINITIONS_H
#endif // HOME_KIT