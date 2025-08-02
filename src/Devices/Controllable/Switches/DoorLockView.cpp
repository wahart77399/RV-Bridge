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

#include "DoorLockView.h"
#include "DoorLock.h"
#include "Packet.h"
#include "DGN.h"

std::map<uint8_t, DoorLock*> DoorLockView::locks;
SpanUserCommand* DoorLockView::lockUnLock = nullptr;
SpanUserCommand* DoorLockView::lockStatus = nullptr;

const char  DoorLockView::LOCK_COMMAND_ID = 'd'; // d is for doors
const char* DoorLockView::LOCK_COMMAND_DESCRIPTION = "<index>=<state:0-1>,... - send onOff to <index>";
const char  DoorLockView::LOCK_STATUS_ID = 'D';
const char* DoorLockView::LOCK_STATUS_DESCRIPTION = "<index> to retrieve current state in HomeSpan";
        

// this is a static function used as a callback
void DoorLockView::cmdLockUnLock(const char* buff) { 
    // void cmdSendOnOff(const char* buff) { 
    printf("DoorLockView::cmdSendOnOff called with buff=%s\n", buff);
    RVC_DGN dgn = LOCK_COMMAND;
    DoorLockView::cmdCallback(dgn, buff);
    printf("DoorLockView::cmdSendOnOff completed\n");
}

void DoorLockView::cmdLockStatus(const char* buff) {
    // void cmdOnOffStatus(const char* buff) {
    printf("DoorLockView::cmdOnOffStatus called with buff=%s\n", buff);
    RVC_DGN dgn = LOCK_STATUS;
    DoorLockView::cmdCallback(dgn, buff);
    printf("DoorLockView::cmdOnOffStatus completed\n");
}

void DoorLockView::cmdCallback(RVC_DGN dgn, const char* buff) {
    printf("DoorLockView::cmdCallback called with dgn=%u, buff=%s\n", dgn, buff);
    int16_t index;
	int16_t val;

	buff += 1;
	
	while (buff) {

		buff = Packet::parseBufferForValuePair(buff, index, val);
		if (index!=-1 && val!=-1) {
			printf("%u: DoorLockView::cmdCallBack: index=%d, val=%d\n", (uint32_t)millis(), index, val);
            // insure mutex locked before creating / adding models to the static member for all Light switches
            // std::lock_guard<std::mutex> lock(DC_LightSwithMutex);
            GenericDevice* mdl = locks[index];
            uint8_t* theData = (uint8_t* ) buff;
			if (mdl != nullptr) {
                printf("DoorLockView::cmdCallback: mdl != nullptr - calling mdl->executeCommand\n");
                mdl->executeCommand(dgn, theData);
            }
		}
		else {
			printf("%u: cmdSendOnOff: parameter error!\n", (uint32_t)millis());
		}
	}
    printf("DoorLockView::cmdCallback completed\n");
}

void DoorLockView::prepUserCommands(void) {
    printf("DoorLockView::prepUserCommands called\n");
    if (DoorLockView::lockUnLock == nullptr) {
        // create the user commands for the light switch
       DoorLockView::lockUnLock = new SpanUserCommand(LOCK_COMMAND_ID, LOCK_COMMAND_DESCRIPTION, DoorLockView::cmdLockUnLock);
       printf("DoorLockView::prepUserCommands: onOff command created\n");
        // DoorLockView::onOff = new SpanUserCommand(ON_OFF_COMMAND, ON_OFF_COMMAND_DESCRIPTION, cmdSendOnOff);
        
    }
    if (DoorLockView::lockStatus == nullptr) {
        DoorLockView::lockStatus = new SpanUserCommand(LOCK_STATUS_ID, LOCK_STATUS_DESCRIPTION, DoorLockView::cmdLockStatus);
        // DoorLockView::status = new SpanUserCommand(ON_OFF_STATUS, ON_OFF_STATUS_DESCRIPTION, cmdOnOffStatus);
        printf("DoorLockView::prepUserCommands: status command created\n");
    }
}

bool DoorLockView::bridgeCreated = false;

void DoorLockView::createBridge(void) {
    if (!DoorLockView::bridgeCreated) {
        printf("DoorLockView::createBridge called\n");
        homeSpan.begin(Category::Locks,"HomeSpan Locks");
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

bool DoorLockView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("DoorLockView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        if ((spanCharTargetLockState != nullptr) && (spanCharCurrentLockState != nullptr)) {
            DoorLock* mdl = (DoorLock* )getModel();
            if (mdl != nullptr) {
                // printf("DoorLockView::updateView - mdl not null\n");
                index = mdl->index();
                // toggle the door lock state
                boolean locked = mdl->isLocked();
                spanCharCurrentLockState->setVal(locked);
                PacketQueue::clearLastPacketReceiveTime();
                //if (index == 1)
                    //printf("DoorLockView::updateView - locked = %d, spancharCurrentLockState->getVal() = %d\n", locked, spanCharCurrentLockState->getVal());
                // mdl->setLockedFlag(!locked);
                updated = true;
            }        
        }
    }
    // printf("DoorLockView::updateView completed \n"); 
    return updated;
}

boolean DoorLockView::update(void) {
    // printf("DoorLockView::update called\n");
    dontUpdateTheView(); // reset the flag - don't run thru updateView since the controller and View are sending the update
    bool updated = false;
    if ((spanCharTargetLockState != nullptr) && (spanCharCurrentLockState != nullptr)) {
        // printf("DoorLockView::update: spanCharTargetLockState is not null, index = %d\n", indexOfModel());
        // printf("DoorLockView::update: spanCharTargetLockState->getVal() = %d\n", spanCharTargetLockState->getVal());
        // printf("DoorLockView::update: SpanDeviceName = %s\n", spanDeviceName);
        // spanCharOn->setVal(getModel()->getByte(0));
        // if (spanCharCurrentLockState->getVal() != spanCharTargetLockState->getNewVal()) {

        // printf("DoorLockView::update - spanCharTargetLockState->getNewVal = %d\n", spanCharTargetLockState->getNewVal());
        // printf("DoorLockView::update - spanCharCurrentLockState->getVal = %d\n", spanCharCurrentLockState->getVal());
        DoorLock* model = (DoorLock* )getModel();
        if (model != nullptr) {
            uint8_t* rawData = (uint8_t* )model->getCurrentData();
            if (rawData != nullptr) {
                    // rawData[1] = (spanCharTargetLockState->getNewVal() ? model->LOCK_IT : model->UNLOCK_IT);
                model->setLockedFlag(spanCharTargetLockState->getNewVal() == DoorLockView::VALID_LOCK_STATE);
                    // printf("DoorLockView::update: rawData[1] set to %d\n", rawData[1]);
                    // printf("DoorLockView::update: model->isLocked() = %d\n", model->isLocked());
                    // printf("DoorLockView::update: model->setLockedFlag called with %d\n", spanCharTargetLockState->getNewVal());
                // printf("DoorLockView::update: rawData[1] set to %d\n", model->isLocked());
                    // model->setLockedFlag(rawData[1] == model->LOCK_IT);
                    // model->executeCommand(LOCK_COMMAND, spanCharTargetLockState->getNewVal() ? "1" : "0");
                model->executeCommand(LOCK_COMMAND, rawData);
                    // printf("DoorLockView::update: model->executeCommand called with %s\n", spanCharTargetLockState->getNewVal() ? "1" : "0");
            }
            updated = true;
        }
    }
    updateTheView(); // set the flag to indicate view needs to be updated
    return updated; // return true to indicate success
}

const char* DoorLockView::name = "LockMechanism"; // from Span.h
const char* DoorLockView::type = "45"; // from Span.h

DoorLockView::DoorLockView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), 
          SpanService(DoorLockView::type, DoorLockView::name), 
          spanCharCurrentLockState(nullptr), 
          spanCharTargetLockState(nullptr),
          spanDeviceName(spanDevName) {
    printf("DoorLockView constructor called for %s\n", spanDeviceName);
    
    uint8_t index = indexOfModel();
    if (index < 255) {
        REQ(LockCurrentState);
        REQ(LockTargetState);
        OPT(ConfiguredName);
        OPT_DEP(Name);
        spanCharTargetLockState = new Characteristic::LockTargetState(); 
        spanCharCurrentLockState = new Characteristic::LockCurrentState(); 
        
        printf("DoorLockView constructor model index = %d\n", indexOfModel());
    } else {
        printf("DoorLockView constructor: indexOfModel returned -1, spanCharOn not created\n");
    }
    
    DoorLockView::prepUserCommands();

    printf("DoorLockView constructor completed\n");

}

void DoorLockView::createDoorLockView(GenericDevice* model, const char* spanDevName) {
    printf("DoorLockView::createDoorLockView called\n");
    SpanView::prepHomeSpan();
    DoorLockView::createBridge();
    // SPAN_ACCESSORY(spanDevName);
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
