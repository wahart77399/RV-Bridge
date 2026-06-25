#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Arduino.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "Packet.h"
#include "DGN.h"
#include "Tanks.h"
#include "PacketQueue.h"


const uint8_t Tanks::TANKS_INDEX                  = 0; 
const uint8_t Tanks::TANKS_RELATIVE_LEVEL         = 1;  
const uint8_t Tanks::TANKS_RESOLUTION             = 2;  
const uint8_t Tanks::TANKS_ABSOLUTE_LEVEL_MSB     = 3;
const uint8_t Tanks::TANKS_ABSOLUTE_LEVEL_LSB     = 4;
const uint8_t Tanks::TANKS_SIZE_MSB               = 5;
const uint8_t Tanks::TANKS_SIZE_LSB               = 6; 
const uint8_t Tanks::FRESH_WATER_INSTANCE         = 0;
const uint8_t Tanks::BLACK_WATER_INSTANCE         = 1;
const uint8_t Tanks::GRAY_WATER_INSTANCE          = 2;
const uint8_t Tanks::LPG_INSTANCE                 = 3;
const uint8_t Tanks::FRESH_WATER_SECOND_INSTANCE  = 16;
const uint8_t Tanks::BLACK_WATER_SECOND_INSTANCE  = 17;
const uint8_t Tanks::GRAY_WATER_SECOND_INSTANCE   = 18;
const uint8_t Tanks::LPG_SECOND_INSTANCE          = 19;



CAN_frame_t* Tanks::buildCommand(RVC_DGN dgn) { // do nothing - no commands will be sent to the ATS - we listen only
    return nullptr; // do nothing
}


boolean Tanks::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("Tanks::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // printf("Tanks::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case (TANK_STATUS):
                // printf("Tanks::executeCommand case LOCK_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                updateViews();
                cmdExecuted = true;
                break;
            default:
                printf("Tanks::executeCommand: Status and Commands not managed \n");
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
#endif // ifdef HOME_KIT_2