#include "Arduino.h"
#include "DoorLock.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "Packet.h"
#include "DGN.h"



CAN_frame_t* DoorLock::buildCommand(RVC_DGN dgn) {
    // printf("DoorLock::buildCommand called with dgn=%#x\n", dgn);
    CAN_frame_t* frame = GenericDevice::buildCommand(dgn); // initiates the building of the frame => starting with instance and the dgn (ie the first 2 bytes)
    if (frame != nullptr) {
        // printf("DoorLock::buildCommand: frame built with MsgID=%#x\n", frame->MsgID);
        const uint8_t* sendData = (uint8_t* )getCurrentData(); 
        const char* buff = nullptr;
        switch (dgn) {
            case LOCK_COMMAND: {                // Handle second DC Dimmer command
                // printf("DoorLock::buildCommand: Handling LOCK_COMMAND\n");
                uint8_t* d = frame->data.u8;
  
                if ((d != nullptr) && (sendData != nullptr)) {
                    // Handle switch ON command for second dimmer
                    d[1] = sendData[1];
                    printf("DoorLock::buildCommand lock/unlock command received with %u value.\n", d[1]);       

                    // printf("DoorLock::buildCommand: d[0]=%d, d[1]=%d, d[2]=%d, d[3]=%d, d[4]=%d, d[5]=%d, d[6]=%d, d[7]=%d, val=%d, setLockedFlag=%d\n", 
                    //       d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], val, isLocked());

                } else {
                    // Handle unknown command for second dimmer
                    printf("Unknown command received.\n");
                }
                break;
            }
            case LOCK_STATUS:   
                printf("DoorLock::buildCommand: Lock Status received.\n");
               
                break;
            default:
                // Handle other commands or unknown DGN
                break;
        }
        
    }
    return frame;
}



#include "PacketQueue.h"

boolean DoorLock::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("DoorLock::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // printf("DoorLock::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case LOCK_COMMAND:
                if (sAddress == SOURCE_ADDRESS) { // then we originated the command}
                    // printf("DoorLock::executeCommand: Handling LOCK_COMMAND\n");
                    // need to send the command on the CAN BUS
                    frame = buildCommand(dgn);
                    if (frame != nullptr) {
                        // printf("DoorLock::executeCommand: Sending command on CAN bus with MsgID=%#x\n", frame->MsgID);
                        PacketQueue::queuePacket(frame); //Packet::queueItUpForSend(frame);
                        
                    }
                } else {
                    updateViews(); // it came from the CAN bus, so we update the views
                }
                cmdExecuted = true;
                break;
            
            case (LOCK_STATUS):
                // printf("DoorLock::executeCommand case LOCK_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                updateViews();
                cmdExecuted = true;
                break;
            default:
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
