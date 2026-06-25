#include "RVConstants.h"
#ifdef HOME_KIT_1
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

boolean DC_LightSwitchView::DC_LightSwitchController::update(void) {                              // update() method
        boolean result = false;
        view->dontUpdateTheView(); 
        if (model != nullptr) {
            uint8_t* rawData = model->getCurrentData();
            model->setOnFlag(isOn());
            printf("DC_LightSwitchView::DC_LightSwitchController::update - on: %d\n", model->isOn());
            model->executeCommand(DC_DIMMER_COMMAND, rawData);
            result = true;
        }     
        view->updateTheView();
        return(result);                               // return true
} // update

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
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        DC_Switch* mdl = (DC_Switch* )getModel();
        if (mdl != nullptr) {
            // printf("DC_SwitchView::updateView - mdl not null\n");
            index = mdl->index();;
            // toggle the switch state
            boolean on = mdl->isOn();
            printf("DC_SwitchView::updateView - on=%d\n", on);
            controller.turnOn(on);
            if (index == 0)
                printf("DC_SwitchView::updateView - power = %d, controller.isOn() = %d\n", on, controller.isOn());
                // mdl->setLockedFlag(!locked);
                // ;
            updated = true;
        }        
        // printf("DC_LightSwitchView::updateView completed \n"); 
    }
    return updated;
}


// const char* DC_LightSwitchView::name = "LightBulb"; // from Span.h
// const char* DC_LightSwitchView::type = "43"; // from Span.h

DC_LightSwitchView::DC_LightSwitchView(GenericDevice* model, const char* spanDevName) : SpanView(model), controller(this, model, spanDevName) {

}

void DC_LightSwitchView::createDC_LightSwitchView(GenericDevice* model, const char* spanDevName) {
    printf("DC_LightSwitch::createDC_LightSwitch called\n");
    SpanView::prepHomeSpan();

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    DC_LightSwitchView* tmp = new DC_LightSwitchView(model, spanDevName);
    if (tmp != nullptr)
        printf("DC_LightSwitchView::createDC_LightSwitchView: tmp created successfully\n");
    else
        printf("DC_LightSwitchView::createDC_LightSwitchView: tmp creation failed\n");   
    printf("DC_LightSwitchView::createDC_LightSwitchView completed\n");
}
#endif