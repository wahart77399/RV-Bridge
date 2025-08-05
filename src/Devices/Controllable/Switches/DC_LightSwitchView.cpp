
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

#include "DC_LightSwitchView.h"
#include "DC_Switch.h"
#include "Packet.h"
#include "PacketQueue.h"


bool DC_LightSwitchView::bridgeCreated = false;

void DC_LightSwitchView::createBridge(void) {
    if (!DC_LightSwitchView::bridgeCreated) {
        printf("DC_LightSwitchView::createBridge called\n");
        // create the bridge for the light switch
        // homeSpan.begin(Category::Bridges, "RV-Bridge-On-Off-Switch", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
        // new SpanAccessory(); 
        // new Service::AccessoryInformation();
        // new Characteristic::Identify();
        DC_LightSwitchView::bridgeCreated = true;
        // printf("DC_LightSwitchView::createBridge completed\n");
    }
}



#include "ChassisMobility.h"
bool DC_LightSwitchView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("DC_LightSwitchView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) {
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        if (spanCharOn != nullptr) {
            DC_Switch* mdl = (DC_Switch* )getModel();
            if (mdl != nullptr) {
                index = mdl->index();
                // printf("DC_LightSwitchView::updateView: mdl != nullptr, index = %d, mdl-isOn() = %d, spanCharOn-getVal() = %d\n", 
                //       index, mdl->isOn(), spanCharOn->getVal());
                if (mdl->isOn() != spanCharOn->getVal()) {
                    // printf("DC_LightSwitchView::updateView: mdl->isOn() =n%d spanCharOn->getVal() = %d, updating spanCharOn\n", 
                    //         mdl->isOn(), spanCharOn->getVal());
                    // toggle switch
                    spanCharOn->setVal(mdl->isOn());
                    PacketQueue::clearLastPacketReceiveTime();
                    updated = true;
                }
            }        
        }
    }
    // printf("DC_LightSwitchView::updateView completed \n"); 
    return updated;
}

boolean DC_LightSwitchView::update(void) {
    // printf("DC_LightSwitchView::update called\n");
    dontUpdateTheView(); // reset the flag - don't run thru update since the controller and View are sending the update
    bool updated = false;
    if (spanCharOn != nullptr) {
        // printf("DC_LightSwitchView::update: spanCharOn is not null, index = %d\n", indexOfModel());
        // printf("DC_LightSwitchView::update: spanCharOn->getVal() = %d\n", spanCharOn->getVal());
        // printf("DC_LightSwitchView::update: SpanDeviceName = %s\n", spanDeviceName);
        // spanCharOn->setVal(getModel()->getByte(0));
        if ((spanCharOn != nullptr) && (spanCharOn->updated())) {
            // printf("DC_LightSwitchView::update: spanCharOn->updated() is true\n");
            DC_Switch* model = (DC_Switch* )getModel();
            if (model != nullptr) {
                uint8_t* rawData = (uint8_t* )model->getCurrentData();
                if (rawData != nullptr) {
                    // printf("DC_LightSwitchView::update: rawData is not null, setting rawData to %d\n", spanCharOn->getNewVal() ? model->SWITCH_ON : model->SWITCH_OFF);
                    // rawData[2] = (spanCharOn->getNewVal() ? model->SWITCH_ON : model->SWITCH_OFF);
                    // set the on flag in the model
                    model->setOnFlag(spanCharOn->getNewVal());
                    // send the command to the model
                    model->executeCommand(DC_DIMMER_COMMAND, rawData);
                    // printf("DC_LightSwitchView::update: model->executeCommand called with %s\n", model->isOn() ? "1" : "0");
                }
            updated = true;
            }
        }
    }
    updateTheView(); // set the flag to indicate view needs to be updated
    return updated; // return true to indicate success
}

const char* DC_LightSwitchView::name = "LightBulb"; // from Span.h
const char* DC_LightSwitchView::type = "43"; // from Span.h

DC_LightSwitchView::DC_LightSwitchView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), 
          SpanService(DC_LightSwitchView::type, DC_LightSwitchView::name), 
          spanCharOn(nullptr), 
          spanDeviceName(spanDevName) {
    printf("DC_LightSwitchView constructor called for %s\n", spanDeviceName);
    // SPAN_ACCESSORY(spanDeviceName);
    uint8_t index = indexOfModel();
    if (index < 255) {
        REQ(On);
        OPT(Name);
        spanCharOn = new Characteristic::On();
        printf("DC_LightSwitchView constructor model index = %d\n", indexOfModel());
    } else {
        printf("DC_LightSwitchView constructor: indexOfModel returned -1, spanCharOn not created\n");
    }
    

    printf("DC_LightSwitchView constructor completed\n");

}

void DC_LightSwitchView::createDC_LightSwitchView(GenericDevice* model, const char* spanDevName) {
    printf("DC_LightSwitchView::createDC_LightSwitchView called\n");
    SpanView::prepHomeSpan();
    SPAN_ACCESSORY(spanDevName);
    DC_LightSwitchView* tmp = new DC_LightSwitchView(model, spanDevName);
    if (tmp != nullptr)
        printf("DC_LightSwitchView::createDC_LightSwitchView: tmp created successfully\n");
    else
        printf("DC_LightSwitchView::createDC_LightSwitchView: tmp creation failed\n");   
    printf("DC_LightSwitchView::createDC_LightSwitchView completed\n");
}
