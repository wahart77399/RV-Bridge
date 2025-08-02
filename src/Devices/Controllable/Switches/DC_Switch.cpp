/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2023 Randy Ubillos
 *  
 *  https://github.com/rubillos/RV-Bridge
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 
////////////////////////////////////////////////////////////////
//                                                            //
//    RV-Bridge: A HomeKit to RV-C interface for the ESP32    //
//                                                            //
////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "DC_Switch.h"
#include "Packet.h"


CAN_frame_t* DC_Switch::buildCommand(RVC_DGN dgn) {
    printf("DC_Switch::buildCommand called with dgn=%#x\n", dgn);
    CAN_frame_t* frame = GenericDevice::buildCommand(dgn); // initiates the building of the frame => starting with instance and the dgn (ie the first 2 bytes)
    if (frame != nullptr) {
        printf("DC_Switch::buildCommand: frame built with MsgID=%#x\n", frame->MsgID);
        const uint8_t* sendData = getCurrentData(); 
        const char* buff = nullptr;
        switch (dgn) {
            case DC_DIMMER_COMMAND: {                // Handle second DC Dimmer command
                printf("DC_Switch::buildCommand: Handling DC_DIMMER_COMMAND\n");
                uint8_t* d = frame->data.u8;
                // int16_t i;
                // char* cval = (char* )sendData;
                // int16_t val = atoi(cval); // Convert the first character to an integer value
                // buff = Packet::parseBufferForValuePair(sendData, i, val); 
                //printf("DC_Switch::buildCommand: val=%d\n", val);
                if ((d != nullptr) && (sendData != nullptr )) {
                    // Handle switch ON command for second dimmer
                    printf("On/Offf command received with %u value.\n", sendData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX]);
                    
                    d[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = sendData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX]; // Full brightness value -> RVCBrightMax
                    d[2] = sendData[2]; // ff
	                d[3] = sendData[3]; // ff 
	                d[4] = sendData[4]; // ff
	                d[5] = sendData[5]; // ff

                    printf("DC_Switch::buildCommand: d[0]=%d, d[1]=%d, d[2]=%d, d[3]=%d, d[4]=%d, d[5]=%d, d[6]=%d, d[7]=%d, getOnFlag=%d\n", 
                           d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], getOnFlag());

                } else {
                    // Handle unknown command for second dimmer
                    printf("Unknown command received.\n");
                }
                break;
            }
            case DC_DIMMER_STATUS_1:
                // Handle DC Dimmer Status 1
                printf("DC Dimmer Status 1 received.\n");
                // Process status data if needed
                break;

            case DC_DIMMER_STATUS_2:
                // Handle DC Dimmer Status 2
                printf("DC Dimmer Status 2 received.\n");
                // Process status data if needed 
                break;
            
            case DC_DIMMER_STATUS_3: // goes with DC_DIMMER_COMMAND_2
                // Handle DC Dimmer Status 3
                printf("DC Dimmer Status 3 received.\n");
                // Process status data if needed
                break;
            case DC_LIGHTING_CONTROLLER_STATUS_1:     
                // Handle DC Lighting Controller Status 1
                printf("DC Lighting Controller Status 1 received.\n");
                // Process status data if needed
                break;

            case DC_LIGHTING_CONTROLLER_STATUS_2:
                // Handle DC Lighting Controller Status 2
                printf("DC Lighting Controller Status 2 received.\n");
                // Process status data if needed
                break;
            case DC_LIGHTING_CONTROLLER_STATUS_3:
                // Handle DC Lighting Controller Status 3
                printf("DC Lighting Controller Status 3 received.\n");
                // Process status data if needed
                break;
            case DC_LIGHTING_CONTROLLER_STATUS_4:
                // Handle DC Lighting Controller Status 4
                printf("DC Lighting Controller Status 4 received.\n");
                // Process status data if needed
                break;
            case DC_LIGHTING_CONTROLLER_STATUS_5:
                // Handle DC Lighting Controller Status 5
                printf("DC Lighting Controller Status 5 received.\n");
                // Process status data if needed
                break;
            case DC_LIGHTING_CONTROLLER_STATUS_6:
                // Handle DC Lighting Controller Status 6
                printf("DC Lighting Controller Status 6 received.\n");
                // Process status data if needed
                break;
            case DC_LIGHTING_CONTROLLER_COMMAND_1:
                // Handle DC Lighting Controller Command 1
                printf("DC Lighting Controller Command 1 received.\n");
                // Process command data if needed
                break;
            case DC_LIGHTING_CONTROLLER_COMMAND_2:
                // Handle DC Lighting Controller Command 2
                printf("DC Lighting Controller Command 2 received.\n");
                // Process command data if needed
                break;
            case DC_LIGHTING_CONTROLLER_COMMAND_3:
                // Handle DC Lighting Controller Command 3
                printf("DC Lighting Controller Command 3 received.\n");
                // Process command data if needed
                break;
            case DC_LIGHTING_CONTROLLER_COMMAND_4:
                // Handle DC Lighting Controller Command 4
                printf("DC Lighting Controller Command 4 received.\n");
                // Process command data if needed
                break;
            case DC_LIGHTING_CONTROLLER_COMMAND_5:
                // Handle DC Lighting Controller Command 5
                printf("DC Lighting Controller Command 5 received.\n");
                // Process command data if needed
                break;
            case DC_LIGHTING_CONTROLLER_COMMAND_6: 
                // Handle DC Lighting Controller Command 6
                printf("DC Lighting Controller Command 6 received.\n");
                // Process command data if needed
            default:
                // Handle other commands or unknown DGN
                break;
        }
        
    }
    return frame;
}

/**
void DC_Switch::printPacketData(RVC_DGN dgn, uint8_t* data, PacketPrint printPacket) {
    GenericDevice::printPacketData(dgn, data, printPacket);
    if (printPacket == packetPrintIfKnown || printPacket == packetPrintYes) {
        // Print additional information specific to DC Switch
    }
}
*/

#include "PacketQueue.h"

boolean DC_Switch::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t sAddress) {
    // printf("DC_Switch::executeCommand called with dgn=%#x\n", dgn);
    boolean cmdExecuted = GenericDevice::executeCommand(dgn, data);
    if (!cmdExecuted) { // && (data != nullptr)) {
        // printf("DC_Switch::executeCommand: Command not executed, building command for dgn=%#x\n", dgn);
        CAN_frame_t* frame = nullptr;
        uint8_t* rawData = (uint8_t* )data;
        switch (dgn) {
            case (DC_DIMMER_COMMAND): {
                if (sAddress == SOURCE_ADDRESS) {
                    // printf("DC_Switch::executeCommand: Handling DC_DIMMER_COMMAND\n");
                    // need to send the command on the CAN BUS
                    frame = buildCommand(dgn);
                    if (frame != nullptr) {
                        // printf("DC_Switch::executeCommand: Sending command on CAN bus with MsgID=%#x\n", frame->MsgID);
                        PacketQueue::queuePacket(frame); //Packet::queueItUpForSend(frame);
                    }
                } else {
                    // a command was issued by another controller, not a homekit controller = need to reflect the change 
                    // in homekit, so update the views on the model
                    updateViews(); // it came from the CAN bus, so we update the views
                }
                cmdExecuted  = true;
                break;
            }
            case (DC_DIMMER_COMMAND_2):
                printf("DC_Switch::executeCommand: Handling DC_DIMMER_COMMAND_2 not supported in 2022 ESSEX\n");
                break;
            case (DC_DIMMER_STATUS_1):
                printf("DC_Switch::executeCommand: !!!!!!!!!! CURRENTLY DO NOT Handle DC_DIMMER_STATUS_1 or 2 !!!!!!!!!!!!\n");
            case (DC_DIMMER_STATUS_2):
                printf("DC_Switch::executeCommand: !!!!!!!!!! CURRENTLY DO NOT Handle DC_DIMMER_STATUS_1 or 2 !!!!!!!!!!!!\n");
            case (DC_DIMMER_STATUS_3):
                // then we don't send a command on the CAN bus, we update our views (HOME SPAN)
                // the -> the views will requst the data from the buffer 
                // printf("DC_Switch::executeCommand: Handling DC_DIMMER_STATUS_3\n");
                if (rawData != nullptr) {
                    setData(dgn, rawData); // set the current data to the new data
                    updateViews(); // update the views with the new data
                    cmdExecuted = true;
                } else {
                    printf("DC_Switch::executeCommand: No data received for DC_DIMMER_STATUS_3\n");
                }
                break;

            default:
                printf("DC_Switch::executeCommand: Unknown DGN %d received, cannot execute command.\n", dgn);
                break;
        }
    }
    return cmdExecuted; // Command execution failed
}

