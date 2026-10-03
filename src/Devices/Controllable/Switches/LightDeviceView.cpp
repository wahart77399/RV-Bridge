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

#include "LightDeviceView.h"
#include "LightDevice.h"
#include "Packet.h"
#include "PacketQueue.h"
#include "debug.h"


bool LightDeviceView::bridgeCreated = false;
boolean LightDeviceView::isItDimmable(void)  {
    boolean result = false;
    LightDevice* mdl = static_cast<LightDevice* >(this->getModel());
    if (mdl != nullptr) {
        result = mdl->isDimmable();
    }
    return result;
}

void LightDeviceView::setItOn(bool on) { 
    LightDevice* mdl = static_cast<LightDevice*>(this->getModel());
    if (mdl != nullptr)
        mdl->setOn(on);
}

void LightDeviceView::setItsBrightness(uint8_t val) {
    LightDevice* mdl = static_cast<LightDevice*>(this->getModel());
    if (mdl != nullptr)
        mdl->setBrightnessRaw(val);
}

LightDeviceView::LightController::LightController(LightDeviceView* vw, GenericDevice* mdl, const char* spanDeviceName)
        : Service::LightBulb(),
            power(nullptr),
            brightness(nullptr),
            model(nullptr),
            view(nullptr) {
    power = new Characteristic::On();
    this->model = static_cast<LightDevice* >(mdl);
    view = vw;
    if ((vw != nullptr) && vw->isItDimmable()) 
        brightness = new Characteristic::Brightness(DIMMER_QUARTER_BRIGHTNESS); // start with 1/4
}

bool LightDeviceView::LightController::update(void) {                              // update() method
        boolean result = false;
        view->dontUpdateTheView(); 
        // if (model != nullptr) {
            uint8_t* rawData = model->getCurrentData();
            view->setItOn(isOn());
            if ((brightness != nullptr) && (view->isItDimmable())) {
                uint8_t pct = brightness->getNewVal();
                view->setItsBrightness(pct);
            }
            RV_PRINTF("LightDeviceView::LightDeviceController::update - on: %d\n", model->isOn());
            model->executeCommand(DC_DIMMER_COMMAND, rawData);
            result = true;
        // }     
        view->updateTheView();
        return(result);                               // return true
} // update

void LightDeviceView::createBridge(void) {
    if (!LightDeviceView::bridgeCreated) {
        RV_PRINTF("LightDeviceView::createBridge called\n");
        // create the bridge for the light switch
        // homeSpan.begin(Category::Bridges, "RV-Bridge-On-Off-Switch", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
        // new SpanAccessory(); 
        // new Service::AccessoryInformation();
        // new Characteristic::Identify();
        LightDeviceView::bridgeCreated = true;
        // RV_PRINTF("LightDeviceView::createBridge completed\n");
    }
}



#include "ChassisMobility.h"
bool LightDeviceView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // RV_PRINTF("LightDeviceView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        LightDevice* mdl = static_cast<LightDevice*>(getModel());
        if (mdl != nullptr) {
            // RV_PRINTF("DC_SwitchView::updateView - mdl not null\n");
            index = mdl->index();;
            // toggle the switch state
            boolean on = mdl->isOn();
            // RV_PRINTF("DC_SwitchView::updateView - on=%d\n", on);
            controller.turnOn(on);
            if (mdl->isDimmable())
                controller.setBrightness(mdl->getBrightness());
            if (index == 0)
                // RV_PRINTF("LightDeviceView::updateView - power = %d, controller.isOn() = %d\n", on, controller.isOn());
                // mdl->setLockedFlag(!locked);
                ;
            updated = true;
        }        
        // RV_PRINTF("LightDeviceView::updateView completed \n"); 
    }
    return updated;
}


// const char* LightDeviceView::name = "LightBulb"; // from Span.h
// const char* LightDeviceView::type = "43"; // from Span.h

LightDeviceView::LightDeviceView(GenericDevice* model, const char* spanDevName) : SpanView(model), controller(this, model, spanDevName) {

}

void LightDeviceView::createLightDeviceView(GenericDevice* model, const char* spanDevName) {
    RV_PRINTF("LightDevice::createLightDevice called\n");
    SpanView::prepHomeSpan();

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    LightDeviceView* tmp = new LightDeviceView(model, spanDevName);
    if (tmp != nullptr)
        RV_PRINTF("LightDeviceView::createLightDeviceView: tmp created successfully\n");
    else
        RV_PRINTF("LightDeviceView::createLightDeviceView: tmp creation failed\n");   
    RV_PRINTF("LightDeviceView::createLightDeviceView completed\n");
}
#endif
