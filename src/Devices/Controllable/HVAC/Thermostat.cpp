#include "Arduino.h"
#include "ESP32CAN.h"
#include "CAN_config.h"
#include "PacketQueue.h"
#include "DGN.h"


#include "Thermostat.h"



CAN_frame_t* HVAC_Thermostat::buildCommand(RVC_DGN dgn) {
    // printf("HVAC_Thermostat::buildCommand called with dgn=%#x\n", dgn);
    CAN_frame_t* frame = GenericDevice::buildCommand(dgn); // initiates the building of the frame => starting with instance and the dgn (ie the first 2 bytes)
    if (frame != nullptr) {
        // printf("HVAC_Thermostat::buildCommand: frame built with MsgID=%#x\n", frame->MsgID);
        const uint8_t* sendData = (uint8_t* )getCurrentData(); 
        const char* buff = nullptr;
        uint8_t* d = nullptr;
        int i = 0;
        switch (dgn) {
            case THERMOSTAT_COMMAND_1:                 // Handle second DC Dimmer command
                // printf("HVAC_Thermostat::buildCommand: Handling LOCK_COMMAND\n");
                d = frame->data.u8;
  
                if ((d != nullptr) && (sendData != nullptr)) {
                    // Handle switch ON command for second dimmer
                    for (i = 0; i < 8; i++) 
                        d[i] = sendData[i];
                    // printf("HVAC_Thermostat::buildCommand enable/disable command received with %u value.\n", d[THERMOSTAT_INDEX]);       

                    printf("HVAC_Thermostat::buildCommand: dgn=%#x, d[0]=%d, d[1]=%d, d[2]=%d, d[3]=%d, d[4]=%d, d[5]=%d, d[6]=%d, d[7]=%d\n", 
                          dgn, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);

                } else {
                    // Handle unknown command for second dimmer
                    printf("Unknown command received.\n");
                }
                break;
            
            case (THERMOSTAT_COMMAND_2):
            case (THERMOSTAT_STATUS_1):
            case (THERMOSTAT_STATUS_2):
            default:
                // Handle other commands or unknown DGN
                printf("HVAC_Thermostat::buildCommand - dgn=%d - not supported here\n");
                break;
        }
        
    }
    return frame;
}

boolean HVAC_Thermostat::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("HVAC_Thermostat::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { 
        // printf("HVAC_Thermostat::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case THERMOSTAT_COMMAND_1:
                if (sAddress == SOURCE_ADDRESS) { // then we originated the command}
                    // printf("HVAC_Thermostat::executeCommand: Handling THERMOSTAT_COMMAND\n");
                    // need to send the command on the CAN BUS
                    frame = buildCommand(dgn);
                    if (frame != nullptr) {
                        // printf("HVAC_Thermostat::executeCommand: Sending command on CAN bus with MsgID=%#x\n", frame->MsgID);
                        PacketQueue::queuePacket(frame); //Packet::queueItUpForSend(frame);
                        
                    }
                } else {
                    updateViews(); // it came from the CAN bus, so we update the views
                }
                cmdExecuted = true;
                break;
            
            case (THERMOSTAT_STATUS_1):
                // printf("HVAC_Thermostat::executeCommand case THERMOSTAT_STATUS\n");
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer
                setData(dgn, rawData);
                // printf("HVAC_Thermostat::executeCommand calling updateViews \n");
                updateViews();
                cmdExecuted = true;
                break;

            case (THERMOSTAT_STATUS_2):
                // do nothing
                // printf("HVAC_Thermostat::executeCommand - THERMOSTAT_STATUS_2 dgn not supported\n");
            default:
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}
