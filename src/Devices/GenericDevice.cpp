
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
#include "GenericDevice.h"
#include "Packet.h"
#include "SpanView.h"


CAN_frame_t* GenericDevice::buildCommand(RVC_DGN dgn) {
    
    Packet::initPacket(&frame, getInstance(),  (uint32_t) dgn);
    // switch on the buffer for Generic DGNs
    return &frame;
}

void GenericDevice::printDevice(void) {
    printf("Generic Device: %u \n", instance);
}

// MVC portion
void GenericDevice::updateViews() {
    // get iterator and go thru each SpanView and update.
    //printf("GenericDevice::updateViews called\n");
    if (!isViewsEmpty()) {
        std::list<SpanView* >::iterator iter ;
        do { // using a do-while loop to ensure we always enter the loop at least once
            iter = getViews();
            if ((*iter) != nullptr) 
                (*iter)->updateView();
            ++iter;
        } while (!isLastView(iter)); // Ensure we have a valid iterator
    }
    // for (iter=getViews(); !isLastView(iter); ++iter) {
    //     (*iter)->updateView();
    // } 
    //printf("GenericDevice::updateViews done\n");
}

void GenericDevice::addView(SpanView* v) { // this transfers ownership of the view to the model
    std::list<SpanView*>::iterator it = getViews();
    if (v != nullptr) {
        views.insert(it, v);
    }
}

void GenericDevice::deleteViews() { // must delete owned memory
    std::list<SpanView*>::iterator iter;
    for (iter = views.end(); !isLastView(iter); ++iter) {
        if ((*iter) != nullptr) {
            delete (*iter); // Delete the SpanView
            (*iter) = nullptr; // Set the pointer to nullptr after deletion
        }
    }
}

/**
void GenericDevice::printPacketData(RVC_DGN dgn, uint8_t* data, PacketPrint printPacket) {
    // Print packet data based on DGN and data
    if (printPacket == packetPrintIfKnown || printPacket == packetPrintYes) 
        printf("DGN: %u, Instance: %u, Group: %u, Data: ", dgn, instance, group);

}
*/

boolean GenericDevice::executeCommand(RVC_DGN dgn, const uint8_t* data, uint8_t val) {
    // Execute command based on DGN and data
    // printf("GenericDevice::executeCommand called with dgn=%X, data=%s\n", dgn, data);
    boolean cmdSuccess = false;
    if (data != nullptr) {
        // setCurrentData(data); don't do this!!!! different dgn data sets don't match it needs to be interpreted by DGN in the executeCommand
        // Process the command based on DGN
        switch (dgn) {
            case GENERIC_INDICATOR_COMMAND:
                // Handle indicator command
                cmdSuccess = true;
                break;
            case GENERIC_CONFIGURATION_STATUS:
                // Handle configuration status
                cmdSuccess = true;
                break;
            default:
                // Handle other commands
                break;
        }
         // Assume command executed successfully
    }
    // printf("GenericDevice::executeCommand: Command executed with dgn=%X, data=%s, cmdSuccess=%d\n", dgn, data, cmdSuccess);
    return cmdSuccess; // Command execution failed
}