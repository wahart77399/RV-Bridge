#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Arduino.h"
#include "Awning.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "Packet.h"
#include "DGN.h"



CAN_frame_t* Awning::buildCommand(RVC_DGN dgn) {
    // printf("Awning::buildCommand called with dgn=%#x\n", dgn);
    CAN_frame_t* frame = GenericDevice::buildCommand(dgn); // initiates the building of the frame => starting with instance and the dgn (ie the first 2 bytes)
    if (frame != nullptr) {
        // printf("Awning::buildCommand: frame built with MsgID=%#x\n", frame->MsgID);
        const uint8_t* sendData = (uint8_t* )getCurrentData(); 
        const uint8_t* cData = (uint8_t* ) getCommandData();
        const char* buff = nullptr;
        switch (dgn) {
            case AWNING_COMMAND_2:
                printf("Awning::buildCommand: ******** WARNING *********** AWNING_COMMAND_2 called - need to support this command\n");
                break;
            case AWNING_COMMAND: {                // Handle second DC Dimmer command
                printf("Awning::buildCommand: Handling AWINING_COMMAND\n");
                uint8_t* d = frame->data.u8;
                if ((d != nullptr) && (cData != nullptr)) {
                    // Handle switch ON command for second dimmer
                    for (int i = 1; i < 8; i++)
                        d[i] = cData[i];
                    printf("Awning::buildCommand AWNING_COMMAND received with d[0] = %8x, d[1] = %8x, d[2] = %8x, d[3] = %8x, d[4] = %8x, d[5] = %8x, d[6] = %8x, d[7] = %8x.\n", 
                            d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);       
                } else {
                    // Handle unknown command for second dimmer
                    printf("Unknown command received.\n");
                }
                break;
            }
            case AWNING_STATUS_2:
                printf("Awning::buildCommand Awning Status 2 received, ********* WARNING*********** need to support this status \n");
                break;
            case AWNING_STATUS:   
                //printf("Awning::buildCommand: Awning Status received.\n");
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

boolean Awning::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("Awning::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // printf("Awning::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case AWNING_COMMAND:
                printf("Awning::executeCommand AWNING_COMMANBD\n");
                if (sAddress == SOURCE_ADDRESS) { // then we originated the command}
                    // printf("Awning::executeCommand: Handling LOCK_COMMAND\n");
                    // need to send the command on the CAN BUS
                    frame = buildCommand(dgn);
                    if (frame != nullptr) {
                        // printf("Awning::executeCommand: Sending command on CAN bus with MsgID=%#x\n", frame->MsgID);
                        PacketQueue::queuePacket(frame); //Packet::queueItUpForSend(frame);
                        
                    }
                } else {
                    updateViews(); // it came from the CAN bus, so we update the views
                }
                cmdExecuted = true;
                break;
            
            case (AWNING_STATUS):
                printf("Awning::executeCommand case AWNING_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                // if (!isAwningMoving()) {
                    setData(dgn, rawData);
                    updateViews();
                // }
                cmdExecuted = true;
                break;
            case (AWNING_COMMAND_2):
                printf("Awning::executeCommand - AWNING_COMMAND_2 called not supported yet\n");
                break;
            case (AWNING_STATUS_2):
                printf("Awning::executeCommand - AWNING_STATUS_2 called not supported yet\n");
                break;
            default:
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
#endif // ifdef HOME_KIT_2