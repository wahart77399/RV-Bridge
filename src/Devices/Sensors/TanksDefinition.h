#ifndef TANKS_DEFINITION_H
#define TANKS_DEFINITION_H

#define CUSTOM_CHAR_HEADER
#include "HomeSpan.h"

#include "RVConstants.h"
#ifdef HOME_KIT_2


typedef enum {
    NOT_FILLING = 0,
    FILLING = 1,
    FULL = 2
} TankFillingState;

typedef enum {
    FULL_TANK = 0,
    FILLING_TANK = 1
} TankFullState;




// #define TANKS_GROUP


// CUSTOM_SERV(TankLevels,C17C17C1-7C17-3097-3095-312231223095);   // use an Occupancy Sensor instead
// CUSTOM_CHAR(TankPercent, C17C17C1-77C2-48FF-8F27-9C2605A29F52, PR+EV, UINT8, 0, 0, 100, false);
// CUSTOM_CHAR(TankSize, C17C17C1-77C5-48FF-8F27-9C2605A29F52, PR+EV, UINT16, 398, 0, 400, false);  // gallons



#endif // TANKS_DEFINITION_H
#endif // ifdef HOME_KIT_2