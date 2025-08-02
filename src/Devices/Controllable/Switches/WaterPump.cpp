#include "Arduino.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
// #include "Packet.h"
#include "PacketQueue.h"
#include "DGN.h"


#include "WaterPump.h"
// #include "WaterPumpView.h"



CAN_frame_t* WaterPump::buildCommand(RVC_DGN dgn) {
    // printf("WaterPump::buildCommand called with dgn=%#x\n", dgn);
    CAN_frame_t* frame = GenericDevice::buildCommand(dgn); // initiates the building of the frame => starting with instance and the dgn (ie the first 2 bytes)
    if (frame != nullptr) {
        // printf("WaterPump::buildCommand: frame built with MsgID=%#x\n", frame->MsgID);
        const uint8_t* sendData = (uint8_t* )getCurrentData(); 
        const char* buff = nullptr;
        switch (dgn) {
            case WATER_PUMP_COMMAND: {                // Handle second DC Dimmer command
                // printf("WaterPump::buildCommand: Handling LOCK_COMMAND\n");
                uint8_t* d = frame->data.u8;
  
                if ((d != nullptr) && (sendData != nullptr)) {
                    // Handle switch ON command for second dimmer
                    d[WATER_PUMP_INDEX] = sendData[WATER_PUMP_INDEX];
                    printf("WaterPump::buildCommand enable/disable command received with %u value.\n", d[WATER_PUMP_INDEX]);       

                    // printf("WaterPump::buildCommand: d[0]=%d, d[1]=%d, d[2]=%d, d[3]=%d, d[4]=%d, d[5]=%d, d[6]=%d, d[7]=%d, val=%d, setWater PumpedFlag=%d\n", 
                    //       d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], val, isWater Pumped());

                } else {
                    // Handle unknown command for second dimmer
                    printf("Unknown command received.\n");
                }
                break;
            }
            case WATER_PUMP_STATUS:   
                printf("WaterPump::buildCommand: Water Pump Status received.\n");
                break;
            default:
                // Handle other commands or unknown DGN
                break;
        }
        
    }
    return frame;
}

boolean WaterPump::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("WaterPump::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { 
        // printf("WaterPump::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case WATER_PUMP_COMMAND:
                if (sAddress == SOURCE_ADDRESS) { // then we originated the command}
                    // printf("WaterPump::executeCommand: Handling WATER_PUMP_COMMAND\n");
                    // need to send the command on the CAN BUS
                    frame = buildCommand(dgn);
                    if (frame != nullptr) {
                        // printf("WaterPump::executeCommand: Sending command on CAN bus with MsgID=%#x\n", frame->MsgID);
                        PacketQueue::queuePacket(frame); //Packet::queueItUpForSend(frame);
                        
                    }
                } else {
                    updateViews(); // it came from the CAN bus, so we update the views
                }
                cmdExecuted = true;
                break;
            
            case (WATER_PUMP_STATUS):
                // printf("WaterPump::executeCommand case WATER_PUMP_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                // printf("WaterPump::executeCommand calling updateViews \n");
                updateViews();
                cmdExecuted = true;
                break;
            default:
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
