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

#include "WaterPumpView.h"
#include "WaterPump.h"
#include "Packet.h"
#include "DGN.h"

// bool bridgeCreated = false;

boolean WaterPumpView::WaterPumpController::update(void) {                              // update() method
        boolean result = false;
        view->dontUpdateTheView(); 
        if (model != nullptr) {
            uint8_t* rawData = model->getCurrentData();
            model->turnPumpOn(isOn());
            printf("WaterPumpView::WaterPumpController::update - on: %d\n", model->isPumpOn());
            model->executeCommand(WATER_PUMP_COMMAND, rawData);
        }     
        view->updateTheView();
        return(result);                               // return true
} // update

#include "ChassisMobility.h"
bool WaterPumpView::updateView(void) {
    bool updated = false;
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("WaterPumpView::updateView called - isNeedToUpdateView = %d\n", isNeedToUpdateView());
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        WaterPump* mdl = (WaterPump* )getModel();
        if (mdl != nullptr) {
            // printf("WaterPumpView::updateView - mdl not null\n");
            index = WATER_PUMP_INDEX;
            // toggle the door lock state
            boolean on = mdl->isPumpOn();
            // printf("WaterPumpView::updateView - on=%d\n", on);
            controller.turnOn(on);
            if (index == 0)
                // printf("WaterPumpView::updateView - power = %d, controller.isOn() = %d\n", on, controller.isOn());
                // mdl->setLockedFlag(!locked);
                ;

            updated = true;
        }        
        
    }
    // printf("WaterPumpView::updateView completed \n"); 
    return updated;
}

WaterPumpView::WaterPumpView(GenericDevice* model, const char* spanDevName) : SpanView(model), controller(this, model, spanDevName) {

}

void WaterPumpView::createWaterPumpView(GenericDevice* model, const char* spanDevName) {
    printf("WaterPumpView::createWaterPumpView called\n");
    SpanView::prepHomeSpan();

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    WaterPumpView* tmp = new WaterPumpView(model, spanDevName);
    if (tmp != nullptr)
        printf("WaterPumpView::createWaterPumpView: tmp created successfully\n");
    else
        printf("WaterPumpView::createWaterPumpView: tmp creation failed\n");   
    printf("WaterPumpView::createWaterPumpView completed\n");
}
