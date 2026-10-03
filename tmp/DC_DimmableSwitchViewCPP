#include "RVConstants.h"
#ifdef HOME_KIT_1
#include "Arduino.h"
#include "DC_DimmableSwitchView.h"
#include "DC_DimmableSwitch.h"

const char* name = "DimmableLamp"; 
const char* type = "43"; // from Span.h
const char  DC_DimmableSwitchView::DIMMABLE_FXN_COMMAND = 'b';
const char* DC_DimmableSwitchView::DIMMABLE_FXN_COMMAND_DESCRIPTION = "<index>=<state:0-1>,... - send Brightness to <index>";


#include <algorithm>

#include "ChassisMobility.h"
#include "PacketQueue.h"

boolean DC_DimmableSwitchView::DC_DimmableSwitchController::update(void) {
    boolean result = false;
    view->dontUpdateTheView(); 
    if ((model != nullptr) && (brightness != nullptr) && (power != nullptr)) {
        if (brightness->updated()) 
            model->setBrightness(getBrightness());        
        model->setOnFlag(isOn());
        model->executeCommand(DC_DIMMER_COMMAND, model->getCurrentData());
        result = true;
    }
    view->updateTheView();
    return result;
}

bool DC_DimmableSwitchView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("DC_DimmableSwitchView::updateView called\n");
    bool updated = false;
    DC_DimmableSwitch* mdl = (DC_DimmableSwitch* )getModel();
    if (isNeedToUpdateView() && ChassisMobility::isParked()) {
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        
        if (mdl != nullptr) {
            index = mdl->index();
            uint8_t modelBrightness = mdl->getBrightness(); // convert to HomeKit percent value
            controller.setBrightness(modelBrightness);
            // mdl->setOnFlag(modelBrightness > 0);
            controller.turnOn(modelBrightness > 0);
            PacketQueue::clearLastPacketReceiveTime();
            updated = true;
        }        
    } else if (isNeedToUpdateView() && !ChassisMobility::isParked()) {
        if (mdl != nullptr) {
            controller.turnOn(false);
            mdl->setOnFlag(false);
        }
    }
    // printf("DC_DimmableSwitchView::updateView completed \n"); 
    return updated;
}


DC_DimmableSwitchView::DC_DimmableSwitchView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), controller(this, model, spanDevName)  {
}

void DC_DimmableSwitchView::createDC_DimmableSwitchView(GenericDevice* model, const char* spanDevName) {
    printf("DC_DimmableSwitchView::createDC_LightSwitchView called\n");
    SpanView::prepHomeSpan();
    
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
     
    DC_DimmableSwitchView* tmp = new DC_DimmableSwitchView(model, spanDevName);
    if (tmp != nullptr)
        printf("DC_DimmableSwitchView::createDC_DimmableSwitchView: tmp created successfully\n");
    else
        printf("DC_DimmableSwitchView::createDC_DimmableSwitchView: tmp creation failed\n");   
    printf("DC_DimmableSwitchView::createDC_DimmableSwitchView completed\n");
}
#endif
