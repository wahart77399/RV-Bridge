/*
#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef AWNING_DEFINITIONS_H
#define AWNING_DEFINITIONS_H


#include "Arduino.h"


enum class CoverKind : uint8_t {Awning = 0, Shade = 1};
enum class CoverMotion : uint8_t { Stopped = 0x00, Opening = 0x01, Closing = 0x02 };

constexpr uint8_t AWNING_MAX_PERCENT = 100;
constexpr uint8_t AWNING_MIN_PERCENT = 0;
constexpr uint8_t AWNING_STEP = 5;
constexpr float_t AWNING_PERCENT_PRECISION = 0.5; // this is the value returned when no data is available
constexpr uint8_t AWNING_MAX_VALUE = 200; // 200 * 0.5 = 100%
constexpr uint8_t AWNING_PCT_FUDGE_FACTOR = 1;

constexpr uint8_t DOOR_AWNING = 1;
constexpr const char* DOOR_AWNING_NAME = "Door Awning";
constexpr uint8_t FRONT_AWNING = 2;
constexpr const char* FRONT_AWNING_NAME = "Front Awning";
constexpr uint8_t BACK_AWNING = 3;
constexpr const char* BACK_AWNING_NAME = "Back Awning";


constexpr float    DOOR_AWNING_EXTEND_CYCLE_TIME_SEC    = 15000.0F; // time to fully extend door awning 14 seconds
constexpr float    DOOR_AWNING_RETRACT_CYCLE_TIME_SEC   = 15000.0F; // time to fully retract door awning 14 seconds
constexpr float    FRONT_AWNING_EXTEND_CYCLE_TIME_SEC   = 44000.0F; // time to fully extend front awning 44 seconds
constexpr float    FRONT_AWNING_RETRACT_CYCLE_TIME_SEC  = 46000.0F; // time to fully retract front awning 46 seconds
constexpr float    BACK_AWNING_EXTEND_CYCLE_TIME_SEC    = 44000.0F; // time to fully extend back awning 44 seconds
constexpr float    BACK_AWNING_RETRACT_CYCLE_TIME_SEC   = 46000.0F; // time to fully retract back awning 46 seconds
constexpr uint8_t  AWNING_FULLY_RETRACTED_PCT           = 100;        // fully retracted awning percentage
constexpr uint8_t  AWNING_FULLY_EXTENDED_PCT            = 0;      // fully extended awning percentage
constexpr uint8_t  AWNING_STEP_PCT                      = 1;        // awning step percentage
constexpr float    DEFAULT_EXTEND_TIME_SEC              = FRONT_AWNING_EXTEND_CYCLE_TIME_SEC;
constexpr float    DEFAULT_RETRACT_TIME_SEC             = FRONT_AWNING_RETRACT_CYCLE_TIME_SEC;

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
    NO_MOTION            = 0x00, // CoverMotion::Stopped, // 0000 0000
    EXTENDING            = 0x01, // CoverMotion::Opening, // 0000 0001
    RETRACTING           = 0x02  // CoverMotion::Closing  // 0000 0010
}AWNING_MOTION;

constexpr uint8_t AWNING_LIGHT_MAX_PERCXENT = MAX_RVC_PERCENT * RVC_PERCENT_PRECISION;
typedef enum {
    AWNING_LIGHT_ON          = AWNING_LIGHT_MAX_PERCXENT, 
    AWNING_LIGHT_OFF         = 0x00 
} AWNING_LIGHT_STATUS;

// Awning Status 2 definitions
constexpr float homeKitShadeOpenValue = 100.0;
constexpr float homeKitShadeClosedValue = 0.0;
typedef uint16_t   ShadeState;
constexpr uint16_t shadeStateClosing = 1 << 3;
constexpr uint16_t shadeStateOpening = 1 << 4;
constexpr uint16_t shadeStateMoving = shadeStateClosing | shadeStateOpening;

constexpr uint16_t shadeStateUserAction = 1 < 6;
constexpr uint16_t shadeStateHomeKitAction = 1 << 7;

constexpr float    DOOR_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC            = 15000.0F;  // time to fully close door night shade 14 seconds
constexpr float    DOOR_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC             = 15000.0F;  // time to fully open door night shade 14 seconds
constexpr float    FRONT_WINDOW_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC    = 44000.0F;  // time to fully close front window night shade 44 seconds
constexpr float    FRONT_WINDOW_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC     = 46000.0F;  // time to fully open front window night shade 46 seconds
constexpr float    DRIVERS_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC         = 44000.0F;  // time to fully close drivers night shade 44 seconds
constexpr float    DRIVERS_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC          = 46000.0F;  // time to fully open drivers night shade 46 seconds
constexpr uint8_t  SHADES_FULLY_CLOSED_PCT                          = 100;       // fully close shade percentage
constexpr uint8_t  SHADES_FULLY_OPEN_PCT                            = 0;         // fully open shade percentage
constexpr uint8_t  SHADES_STEP_PCT                                  = 1;         // Shade step percentage


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
    STOPPED              = 0x00, // CoverMotion::Stopped, // 0000 0000
    CLOSING              = 0x01, // CoverMotion::Opening, // 0000 0001
    OPENING              = 0x02  // CoverMotion::Closing  // 0000 0010
} SHADE_MOTION;

#endif  // AWNING_DEFINITIONS_H
#endif // HOME_KIT_2
*/

#include "RVConstants.h"
#ifndef AWNING_DEFINITIONS_H
#define AWNING_DEFINITIONS_H

#include "Arduino.h"

enum class CoverKind : uint8_t { Awning = 0, Shade = 1 };

enum class CoverMotion : uint8_t {
    Stopped  = 0x00,
    Opening  = 0x01,   // shade open / awning retract
    Closing  = 0x02    // shade close / awning extend
};

// ---------------------------------------------------------------------------
// Common percent constants
// ---------------------------------------------------------------------------
constexpr uint8_t COVER_MAX_PERCENT     = 100;
constexpr uint8_t COVER_MIN_PERCENT     = 0;
constexpr uint8_t COVER_STEP_PCT        = 1;
constexpr float   COVER_PERCENT_PRECISION = 0.5f;
constexpr uint8_t COVER_PCT_FUDGE       = 1;

// ---------------------------------------------------------------------------
// Awning-specific
// ---------------------------------------------------------------------------
constexpr uint8_t AWNING_MAX_PERCENT            = COVER_MAX_PERCENT;
constexpr uint8_t AWNING_MIN_PERCENT            = COVER_MIN_PERCENT;
constexpr uint8_t AWNING_FULLY_RETRACTED_PCT    = 0;   // 100;   // HomeKit
constexpr uint8_t AWNING_FULLY_EXTENDED_PCT     = 100; // 0;     // HomeKit
constexpr uint8_t AWNING_STEP_PCT               = COVER_STEP_PCT;

constexpr float DOOR_AWNING_EXTEND_CYCLE_TIME_SEC   = 15000.0f;
constexpr float DOOR_AWNING_RETRACT_CYCLE_TIME_SEC  = 15000.0f;
constexpr float FRONT_AWNING_EXTEND_CYCLE_TIME_SEC  = 44000.0f;
constexpr float FRONT_AWNING_RETRACT_CYCLE_TIME_SEC = 46000.0f;
constexpr float BACK_AWNING_EXTEND_CYCLE_TIME_SEC   = 44000.0f;
constexpr float BACK_AWNING_RETRACT_CYCLE_TIME_SEC  = 46000.0f;

constexpr float DEFAULT_EXTEND_TIME_SEC  = FRONT_AWNING_EXTEND_CYCLE_TIME_SEC;
constexpr float DEFAULT_RETRACT_TIME_SEC = FRONT_AWNING_RETRACT_CYCLE_TIME_SEC;

constexpr uint8_t DOOR_AWNING  = 1;
constexpr uint8_t FRONT_AWNING = 2;
constexpr uint8_t BACK_AWNING  = 3;

constexpr const char* DOOR_AWNING_NAME  = "Door Awning";
constexpr const char* FRONT_AWNING_NAME = "Front Awning";
constexpr const char* BACK_AWNING_NAME  = "Back Awning";

// ---------------------------------------------------------------------------
// Shade-specific
// ---------------------------------------------------------------------------
constexpr uint8_t SHADES_MAX_PERCENT         = COVER_MAX_PERCENT;
constexpr uint8_t SHADES_MIN_PERCENT         = COVER_MIN_PERCENT;
constexpr uint8_t SHADES_FULLY_CLOSED_PCT    = 100;   // HomeKit
constexpr uint8_t SHADES_FULLY_OPEN_PCT      = 0;     // HomeKit
constexpr uint8_t SHADES_STEP_PCT            = COVER_STEP_PCT;
constexpr uint8_t SHADE_GROUP_NONE           = 0U;

constexpr float DOOR_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC         = 15000.0f;
constexpr float DOOR_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC          = 15000.0f;
constexpr float FRONT_WINDOW_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC = 44000.0f;
constexpr float FRONT_WINDOW_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC  = 46000.0f;
constexpr float DRIVERS_NIGHT_SHADE_CLOSE_CYCLE_TIME_SEC      = 44000.0f;
constexpr float DRIVERS_NIGHT_SHADE_OPEN_CYCLE_TIME_SEC       = 46000.0f;

constexpr uint8_t DOOR_NIGHT_SHADE         = 1;
constexpr uint8_t FRONT_WINDOW_NIGHT_SHADE = 2;
constexpr uint8_t DRIVERS_NIGHT_SHADE      = 3;

constexpr const char* DOOR_NIGHT_SHADE_NAME         = "Door Night Shade";
constexpr const char* FRONT_WINDOW_NIGHT_SHADE_NAME = "Front Window Night Shade";
constexpr const char* DRIVERS_NIGHT_SHADE_NAME      = "Drivers Night Shade";

// Additional shade instance IDs kept for compatibility
constexpr uint8_t LIVING_ROOM_DAY_SHADE   = 88;
constexpr uint8_t LIVING_ROOM_NIGHT_SHADE = 89;
constexpr uint8_t BEDROOM_DAY_SHADE       = 90;
constexpr uint8_t BEDROOM_NIGHT_SHADE     = 91;
constexpr uint8_t BATHROOM_DAY_SHADE      = 92;
constexpr uint8_t BATHROOM_NIGHT_SHADE    = 93;

// ---------------------------------------------------------------------------
// Index & command enums (unchanged numeric values)
// ---------------------------------------------------------------------------
typedef enum {
    AWNING_INSTANCE_INDEX                    = 0,
    AWNING_STATUS_MOTION_INDEX               = 1,
    AWNING_STATUS_POSITION_INDEX             = 2,
    AWNING_STATUS_TRAVEL_LOCK_INDEX          = 3,
    AWNING_STATUS_PARK_INDEX                 = 4,
    AWNING_STATUS_LIGHT_INDEX                = 5,
    AWNING_STATUS_LIGHT_DIM_INDEX            = 6,
    AWNING_STATUS_2ND_LIGNT_INDEX            = 7,
    AWNING_COMMAND_LOCK_INDEX                = 1,
    AWNING_COMMAND_DIRECTION_INDEX           = 2,
    AWNING_COMMAND_POSITION_INDEX            = 3,
    AWNING_COMMAND_MOTION_INDEX              = 4,
    AWNING_COMMAND_LIGHT_INDEX               = 6,
    AWNING_COMMAND_2ND_LIGHT_INDEX           = 7,
    AWNING_STATUS_2_MOTION_SENSITIVITY_INDEX = 1,
    AWNING_STATUS_2_CALBRATION_INDEX         = 2,
    AWNING_STATUS_2_EXTEND_LOCKOUT_INDEX     = 3,
    AWNING_STATUS_2_RETRACT_LOCKOUT_INDEX    = 4,
    AWNING_STATUS_2_INPUT_STATE_INDEX        = 5,
    AWNING_COMMAND_2_MOTION_SENSITIVITY_INDEX= 1,
    AWNING_COMMAND_2_CALBRATION_INDEX        = 2,
    AWNING_COMMAND_2_EXTEND_LOCKOUT_INDEX    = 3,
    AWNING_COMMAND_2_RETRACT_LOCKOUT_INDEX   = 4,
    AWNING_COMMAND_2_INPUT_STATE_INDEX       = 5
} AWNING_INDECES;

typedef enum {
    AWNING_STOP_COMMAND    = 0x00,
    AWNING_EXTEND_COMMAND  = 0x01,
    AWNING_RETRACT_COMMAND = 0x02
} AWNING_COMMAND_DIRECTION;

typedef enum {
    NO_MOTION  = 0x00,
    EXTENDING  = 0x01,
    RETRACTING = 0x02
} AWNING_MOTION;

typedef enum {
    SHADE_INSTANCE_INDEX   = 0,
    SHADE_GROUP_INDEX      = 1,
    SHADE_MOTOR_DUTY_INDEX = 2,
    SHADE_COMMAND_INDEX    = 3,
    SHADE_DURATION_INDEX   = 4,
    SHADE_INTERLOCK_INDEX  = 5,
    SHADE_STATUS_INDEX     = 3,
    SHADE_LAST_COMMAND_INDEX = 5,
    SHADE_OVER_INDEX       = 6
} SHADE_INDECES;

typedef enum {
    SHADE_COMMAND_REVERSE        = 0x41,
    SHADE_COMMAND_FORWARD        = 0x81,
    SHADE_COMMAND_STOP           = 0x04,
    SHADE_COMMAND_LOCK           = 0x21,
    SHADE_COMMAND_UNLOCK         = 0x22,
    SHADE_COMMAND_TILT           = 0x10,
    SHADE_COMMAND_TOGGLE_FORWARD = 0x85,
    SHADE_COMMAND_TOGGLE_REVERSE = 0x45
} SHADES_COMMANDS;

typedef enum {
    STOPPED  = 0x00,
    CLOSING  = 0x01,
    OPENING  = 0x02
} SHADE_MOTION;

#endif // AWNING_DEFINITIONS_H
