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
#include "RVConstants.h"
#ifdef HOME_KIT_1

#include "DoorLockView.h"
#include "DoorLock.h"
#include "Packet.h"
#include "DGN.h"



bool DoorLockView::bridgeCreated = false;
const uint8_t DoorLockView::VALID_LOCK_STATE = 0x01;
const uint8_t DoorLockView::VALID_UNLOCKED_STATE = 0x00;

void DoorLockView::createBridge(void) {
    if (!DoorLockView::bridgeCreated) {
        printf("DoorLockView::createBridge called\n");
        // homeSpan.begin(Category::Locks,"HomeSpan Locks");
        // create the bridge for the light switch
        // homeSpan.begin(Category::Bridges, "RV-Bridge-On-Off-Switch", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
        // new SpanAccessory(); 
        // new Service::AccessoryInformation();
        // new Characteristic::Identify();
        DoorLockView::bridgeCreated = true;
        printf("DoorLockView::createBridge completed\n");
    }
}

#include "ChassisMobility.h"
#include "PacketQueue.h"


boolean DoorLockView::DoorLockController::update(void) {                              // update() method
        boolean result = false;
        view->dontUpdateTheView(); 
        if (model != nullptr) {
            uint8_t* rawData = model->getCurrentData();
            model->setLockedFlag(isLocked());
            printf("DoorLockView::DoorLockController::update - on: %d\n", model->getLockedFlag());
            model->executeCommand(LOCK_COMMAND, rawData);
            result = true;
        }     
        view->updateTheView();
        return(result);                               // return true
} // update

bool DoorLockView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("DoorLockView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        DoorLock* mdl = (DoorLock* )getModel();
        if (mdl != nullptr) {
            // printf("DoorLockView::updateView - mdl not null\n");
            index = mdl->index();
            // toggle the door lock state
            boolean locked = mdl->isLocked();
            if (locked)
                controller.lockIt();
            else
                controller.unLockIt();
            // PacketQueue::clearLastPacketReceiveTime();
            // if (index == 1)
                // printf("DoorLockView::updateView - locked = %d, spancharCurrentLockState->getVal() = %d\n", locked, controller.isLocked());
            updated = true;
        }        
    }
    // printf("DoorLockView::updateView completed \n"); 
    return updated;
}


DoorLockView::DoorLockView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), controller(this, model, spanDevName) {
}

void DoorLockView::createDoorLockView(GenericDevice* model, const char* spanDevName) {
    printf("DoorLockView::createDoorLockView called\n");
    SpanView::prepHomeSpan();
    DoorLockView::createBridge();
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    DoorLockView* tmp = new DoorLockView(model, spanDevName);
    if (tmp != nullptr)
        printf("DoorLockView::createDoorLockView: tmp created successfully\n");
    else
        printf("DoorLockView::createDoorLockView: tmp creation failed\n");   
    printf("DoorLockView::createDoorLockView completed\n");
}

#endif // ifdef HOME_KIT_1
