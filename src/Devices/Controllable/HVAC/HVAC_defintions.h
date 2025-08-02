#ifndef HVAC_DEFINITIONS_H
#define HVAC_DEFINITIONS_H

// RVC Definitions
 typedef enum {
            AUTO_FAN_MODE = 0x00,
            ON_FAN_MODE = 0x10
} RVCFanMode;

typedef enum {
            OFF = 0x00,
            COOL = 0x01,
            HEAT = 0x02,
            AUTO = 0x03
} RVCMode;

typedef enum {
            AUTO_FAN = 0x00, // per the spec - need to validate with Packet::displayPacket
            LOW_FAN = 50,    // will need to watch what this setting is in Packet::displayPacket
            HIGH_FAN = 100   // assuming, but need validate with Packet::displayPacket
} RVCForcedFan;

typedef enum {
	currentFanStateInactive = 0U,
	currentFanStateIdle = 1U,
	currentFanStateBlowing = 2U,

	targetFanStateManual = 0U,
	targetFanStateAuto = 1U,

	heatingCoolingStateOff = 0,
	heatingCoolingStateHeat = 1U,
	heatingCoolingStateCool = 2U
} HomeKitHVACStates;

constexpr uint8_t INITIAL_ROTATION_SPEED = 0;
constexpr uint8_t DEFAULT_TEMP = 72;
#endif