#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Arduino.h"
#include "FloorHeat.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "Packet.h"
#include "DGN.h"

CAN_frame_t* FloorHeat::buildCommand(RVC_DGN dgn) {
    // printf("FloorHeat::buildCommand called with dgn=%#x\n", dgn);
    CAN_frame_t* frame = GenericDevice::buildCommand(dgn); // initiates the building of the frame => starting with instance and the dgn (ie the first 2 bytes)
    if (frame != nullptr) {
        // printf("FloorHeat::buildCommand: frame built with MsgID=%#x\n", frame->MsgID);
        const uint8_t* sendData = (uint8_t* )getCurrentData(); 
        const uint8_t* cData = (uint8_t* ) getCommandData();
        const char* buff = nullptr;
        switch (dgn) {
            case FLOOR_HEAT_COMMAND:
                //printf("FloorHeat::buildCommand: Handling FLOOR_HEAT_COMMAND\n");
                {
                    uint8_t* d = frame->data.u8;
                    if ((d != nullptr) && (cData != nullptr)) {
                        // Handle switch ON command for second dimmer
                        for (int i = 1; i < 8; i++)
                            d[i] = cData[i];
                        // printf("FloorHeat::buildCommand FLOOR_HEAT_COMMAND received with d[0] = %8x, d[1] = %8x, d[2] = %8x, d[3] = %8x, d[4] = %8x, d[5] = %8x, d[6] = %8x, d[7] = %8x.\n", 
                        //         d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);       
                    } else {
                        // Handle unknown command for second dimmer
                        // printf("Unknown command received.\n");
                    }
                }
                break;
            case FLOOR_HEAT_STATUS:   
                //printf("FloorHeat::buildCommand: Floor Heat Status received.\n");
                setData(dgn, (uint8_t* )sendData);
                break;
            default:
                // Handle other commands or unknown DGN
                break;
        }
        
    }
    return frame;
}   

#include "PacketQueue.h"
boolean FloorHeat::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("FloorHeat::executeCommand called with dgn=%#x\n", dgn);
        // printf("Awning::executeCommand called with dgn=%#x\n", dgn);
    boolean commandExecuted = GenericDevice::executeCommand(dgn, data);
    if (!commandExecuted) { // && (data != nullptr)) {
        // printf("FloorHeat::buildCommand: frame built with MsgID=%#x\n", frame->MsgID);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        const uint8_t* sendData = (uint8_t* )getCurrentData(); 
        const uint8_t* cData = (uint8_t* ) getCommandData();
        switch (dgn) {
            case FLOOR_HEAT_COMMAND:
                // printf("FloorHeat::executeCommand: FLOOR_HEAT_COMMAND received\n");
                if (sAddress == SOURCE_ADDRESS) { // then we originated the command}
                    // printf("FloorHeat::executeCommand: Handling FLOOR_HEAT_COMMAND\n");
                    // need to send the command on the CAN BUS
                    frame = buildCommand(dgn);
                    if (frame != nullptr) {
                        // printf("FloorHeat::executeCommand: Sending command on CAN bus with MsgID=%#x\n", frame->MsgID);
                        PacketQueue::queuePacket(frame); //Packet::queueItUpForSend(frame);
                        
                    }
                } else {
                    updateViews(); // it came from the CAN bus, so we update the views
                }
                commandExecuted = true;
                break;
            case FLOOR_HEAT_STATUS:
                // printf("FloorHeat::executeCommand: FLOOR_HEAT_STATUS received\n");
                                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                setData(dgn, rawData);
                updateViews();
                commandExecuted = true;
                break;
            default:
                // printf("FloorHeat::executeCommand: Unknown DGN received: %#x\n", dgn);
                break;
        }
    }
    return commandExecuted;
}
#endif


