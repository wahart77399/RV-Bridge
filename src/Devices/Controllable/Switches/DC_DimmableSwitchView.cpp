#include "Arduino.h"
#include "DC_DimmableSwitchView.h"
#include "DC_DimmableSwitch.h"


SpanUserCommand* DC_DimmableSwitchView::setBrightness = nullptr;
const char* name = "DimmableLamp"; 
const char* type = "43"; // from Span.h
const char  DC_DimmableSwitchView::DIMMABLE_FXN_COMMAND = 'b';
const char* DC_DimmableSwitchView::DIMMABLE_FXN_COMMAND_DESCRIPTION = "<index>=<state:0-1>,... - send Brightness to <index>";
// const char  DC_DimmableSwitchView::DIMMABLE_FXN_STATUS = 'B';
// const char* DC_DimmableSwitchView::DIMMABLE_FXN_STATUS_DESCRIPTION = "<index> to retrieve current state in HomeSpan";

void DC_DimmableSwitchView::cmdSetBrightness(const char* buff) {
    // void cmdOnOffStatus(const char* buff) {
    printf("DC_DimmableSwitchView::cmdOnOffStatus called with buff=%s\n", buff);
    RVC_DGN dgn = DC_DIMMER_STATUS_3;
    DC_LightSwitchView::cmdCallback(dgn, buff);
    printf("DC_DimmableSwitchView::cmdOnOffStatus completed\n");
}

#include <algorithm>
void DC_DimmableSwitchView::prepUserCommands(void) {
    printf("DC_DimmableSwitchView::prepUserCommands called\n");
    if (DC_DimmableSwitchView::setBrightness == nullptr) {
        // create the user commands for the light switch
       DC_DimmableSwitchView::setBrightness = new SpanUserCommand(DIMMABLE_FXN_COMMAND, DIMMABLE_FXN_COMMAND_DESCRIPTION, DC_DimmableSwitchView::cmdSetBrightness);
       printf("DC_DimmableSwitchView::prepUserCommands: onOff command created\n");
        // DC_DimmableSwitchView::onOff = new SpanUserCommand(ON_OFF_COMMAND, ON_OFF_COMMAND_DESCRIPTION, cmdSendOnOff);
        
    }
}

#include "ChassisMobility.h"
#include "PacketQueue.h"
bool DC_DimmableSwitchView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("DC_DimmableSwitchView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) {
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        if (spanCharBrightness != nullptr) {
            DC_DimmableSwitch* mdl = (DC_DimmableSwitch* )getModel();
            if (mdl != nullptr) {
                index = mdl->index();
                uint8_t modelBrightness = (mdl->getBrightness() * HomeKitPercentMax)/RVCBrightMax; // convert to HomeKit percent value
                uint8_t spanCharBrightnessVal = spanCharBrightness->getVal();
                // printf("DC_DimmableSwitchView::updateView: modelBrightness = %d, spanCharBrightnessVal = %d\n", modelBrightness, spanCharBrightnessVal);

                if ((modelBrightness > mdl->SWITCH_OFF) && (modelBrightness <= HomeKitPercentMax)) {
                    spanCharBrightness->setVal(modelBrightness );                 
                    turnOnLight(); // turn on the light if brightness is greater than 0
                } else {
                    turnOffLight(); // turn off the light if brightness is 0
                    
                }
                PacketQueue::clearLastPacketReceiveTime();
                updated = true;
            }        
        }
        DC_LightSwitchView::updateView(); // call the base class updateView to update the view
    }
    // printf("DC_DimmableSwitchView::updateView completed \n"); 
    return updated;
}

boolean DC_DimmableSwitchView::update(void) {
    // printf("DC_DimmableSwitchView::update called\n");
    dontUpdateTheView(); // reset the flag - don't run thru update since the controller and View are sending the update
    bool updated = false;
    if (spanCharBrightness != nullptr) {
        // printf("DC_DimmableSwitchView::update: spanCharOn is not null, index = %d\n", indexOfModel());
        // printf("DC_DimmableSwitchView::update: spanCharOn->getNewVal() = %d\n", spanCharBrightness->getNewVal());
        // printf("DC_DimmableSwitchView::update: SpanDeviceName = %s\n",  getSpanDeviceName());
        // spanCharOn->setVal(getModel()->getByte(0));
        if (spanCharBrightness->updated()) {
            // printf("DC_DimmableSwitchView::update: spanCharOn->updated() is true\n");
            DC_DimmableSwitch* model = (DC_DimmableSwitch* )getModel();
            if (model != nullptr) {
                uint8_t* rawData = (uint8_t* )model->getCurrentData();
                if (rawData != nullptr) {
                    uint16_t newLevel = (spanCharBrightness->getNewVal() * RVCBrightMax) / HomeKitPercentMax;
                    uint16_t max = RVCBrightMax;
                    // printf("DC_DimmableSwitchView::update: rawData is not null, setting rawData to %d\n", newLevel);
                    
                    if (newLevel > model->SWITCH_OFF)
                        model->setBrightness(std::min(newLevel, max)); // set the brightness value in the model
                    else
                        model->setOnFlag(false);
                    //since we are dependent on DIMMER_COMMAND and not #2 the index is the same, so, don't set onFlag model->setOnFlag(spanCharBrightness->getNewVal() > 0); // set the on flag based on the brightness value
                    // send the command to the model
                    model->executeCommand(DC_DIMMER_COMMAND, rawData);
                    // printf("DC_DimmableSwitchView::update: model->executeCommand called with %d\n", model->getBrightness());
                }
            updated = true;
            }
        }
    }
    updateTheView(); // set the flag to indicate view needs to be updated
    return updated; // return true to indicate success
}

DC_DimmableSwitchView::DC_DimmableSwitchView(GenericDevice* model, const char* spanDevName) 
        : DC_LightSwitchView(model, spanDevName),
          spanCharBrightness(nullptr) {
    printf("DC_DimmableSwitchView constructor called for %s\n", spanDevName);
    // SPAN_ACCESSORY(spanDeviceName);
    uint8_t index = indexOfModel();
    if (index < 255) {
        // SPAN_ACCESSORY(spanDeviceName);
        OPT(Brightness);
		OPT(Hue);
		OPT(Saturation);
		OPT(ColorTemperature);;
        spanCharBrightness = new Characteristic::Brightness(HomeKitPercentMax);
		spanCharBrightness->setRange(0, HomeKitPercentMax, 5);
        printf("DC_DimmableSwitchView constructor model index = %d\n", indexOfModel());
    } else {
        printf("DC_DimmableSwitchView constructor: indexOfModel returned -1, spanCharOn not created\n");
    }
    
    DC_DimmableSwitchView::prepUserCommands();

    printf("DC_DimmableSwitchView constructor completed\n");

}

void DC_DimmableSwitchView::createDC_DimmableSwitchView(GenericDevice* model, const char* spanDevName) {
    printf("DC_DimmableSwitchView::createDC_LightSwitchView called\n");
    SpanView::prepHomeSpan();
    SPAN_ACCESSORY(spanDevName);
    DC_DimmableSwitchView* tmp = new DC_DimmableSwitchView(model, spanDevName);
    if (tmp != nullptr)
        printf("DC_DimmableSwitchView::createDC_DimmableSwitchView: tmp created successfully\n");
    else
        printf("DC_DimmableSwitchView::createDC_DimmableSwitchView: tmp creation failed\n");   
    printf("DC_DimmableSwitchView::createDC_DimmableSwitchView completed\n");
}
