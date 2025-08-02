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

#include "ThermostatView.h"
#include "Thermostat.h"
#include "Packet.h"
#include "DGN.h"
 
bool ThermostatView::categoryCreated = false;
void ThermostatView::createCategory(void) {
    if (!categoryCreated) {
         homeSpan.begin(Category::Thermostats,"Thermostats");
         categoryCreated = true;
    }
}
boolean ThermostatView::FanController::update(void) {
	bool updated = false;
    if ((_active != nullptr) && (speed != nullptr) && (targetState != nullptr) && (model != nullptr) && (view != nullptr)) {
        view->dontUpdateTheView();
	    if (_active->updated() || speed->updated() || targetState->updated()) {
		    printf("ThermostatView::FanController::update  - active: %d, speed - %d, targetState - %d\n", 
            _active->getNewVal(), speed->getNewVal(), targetState->getNewVal());
            // updated = view->updateView();
            if (speed->updated()) {
                float spd = speed->getNewVal();
                float lowSpeed = view->HomeKitPercentMax/2;
                RVCForcedFan newSpeed = AUTO_FAN;
                if (spd > 0)
                    newSpeed = (spd <= lowSpeed) ? LOW_FAN : HIGH_FAN;
                model->setFanSpeed(newSpeed);
            }
            if (_active->updated() && (_active->getNewVal() > 0)) 
                model->setFanMode(ON_FAN_MODE);
            else  
                model->setFanMode(AUTO_FAN_MODE);
            model->executeCommand(THERMOSTAT_COMMAND_1, model->getCurrentData());
        }
        view->updateTheView();
    }
	return (updated);  
}

RVCFanMode ThermostatView::FanController::getMode(void) const {
    RVCFanMode mode = AUTO_FAN_MODE;
    if ((_active != nullptr) && (_active->getVal() > 0))
        mode = ON_FAN_MODE;
    return mode;
}

uint16_t ThermostatView::FanController::getSpeed(void) const {
    uint16_t speedResult = (uint16_t) AUTO_FAN;
    if (speed != nullptr) {
        float spd = speed->getVal();
        float lowSpeed = view->HomeKitPercentMax/2;
        if (spd > 0)
            speedResult = (spd <= lowSpeed) ? (uint16_t) LOW_FAN : (uint16_t) HIGH_FAN;
    }
    return speedResult;
}

#include "PacketQueue.h"

 void ThermostatView::FanController::setModeSpeed(uint8_t fanMode, uint8_t spd) {
	bool newActive = fanMode == ON_FAN_MODE;

    if (view != nullptr) {
    	spd = spd * view->HomeKitPercentMax / RVCPercentMax;

	    if ((_active != nullptr) && (newActive != _active->getVal())) {
		    printf("Thermostat::FanController::setModeSpeed #%d - setActive: %d\n", (uint32_t)millis(), model->index(), newActive);
		    _active->setVal(newActive);
	    }
	    if ((speed != nullptr) && (spd != speed->getVal())) {
		    printf("Thermostat::FanController::setModeSpeed #%d - setSpeed: %d\n", model->index(), spd);
		    speed->setVal((spd < 100) ? spd : INITIAL_ROTATION_SPEED);
        }
        PacketQueue::clearLastPacketReceiveTime();
    }
} 



void ThermostatView::FanController::setLevel(uint8_t level) {
	bool on = level > 0;
	bool changed = false;

    if ((model != nullptr) && (!model->isFanAuto())) {
        if ((level > 0) && (level <= view->HomeKitPercentMax)) {
            currentState->setVal(currentFanStateBlowing);
        } else {
            currentState->setVal(currentFanStateIdle);
        }
        PacketQueue::clearLastPacketReceiveTime();
    }
} 
    //*/


// const double tempCScale = 0.03125;
// const double tempCScaleInv = 1.0 / tempCScale;
// const double tempCRoundingOffset = -0.25;
// const float  DEFAULT_TEMP = 72.0;





typedef enum {
    THERM_OFF=0,
    THERM_HEAT = 1,
    THERM_COOL = 2,
    THERM_AUTO = 3
} HomeKitOperatingMode;

boolean ThermostatView::ThermostatController::update(void) {
    boolean updated = false;
    if ((targetState != nullptr) && (targetTemp != nullptr) && (model != nullptr) && (view != nullptr)) {
        view->dontUpdateTheView();
        HVAC_Thermostat* myModel = model;
        if (targetState->updated()) {
            switch(targetState->getNewVal()) {
                case (THERM_OFF):
                    myModel->setOperatingMode(RVCMode::OFF);
                    break;
                case (THERM_HEAT):
                    myModel->setOperatingMode(RVCMode::HEAT);
                    break;
                case(THERM_COOL):
                    myModel->setOperatingMode(RVCMode::COOL);
                    break;
                case(THERM_AUTO):
                default:
                    myModel->setOperatingMode(RVCMode::AUTO);
                    break;
            }
            updated = true;
        }
        if (targetTemp->updated()) {
            RVCMode md = myModel->getOperatingMode();
            float temp = targetTemp->getNewVal();
            switch (md) {
                case (RVCMode::COOL):
                    myModel->setCoolTemp(temp);
                    break;
                case(RVCMode::HEAT):
                    myModel->setHeatTemp(temp);
                    break;
                case (RVCMode::AUTO):
                    myModel->setCoolTemp(temp);
                    myModel->setHeatTemp((temp - 4.0));
                    break;
                case(RVCMode::OFF):
                default:
                    // do nothing
                    break;
            }
        }
        myModel->executeCommand(THERMOSTAT_COMMAND_1, myModel->getCurrentData());
        updated = true;
        view->updateTheView();
    }
    return updated;
}

void ThermostatView::ThermostatController::setInfo(RVCMode opMode, RVCFanMode fanMode, RVCForcedFan fanSpeed, double heatTemp, double coolTemp) {
    if ((currentState != nullptr) && (targetState != nullptr) && (targetTemp != nullptr) && (fan != nullptr) && (ambientTemp != nullptr)) {
        HomeKitOperatingMode homeKitOperatingMode = (HomeKitOperatingMode) currentState->getVal();
        switch(opMode) {
            case (RVCMode::COOL):
                
                switch (homeKitOperatingMode) {
                    case(THERM_OFF):
                    case(THERM_AUTO):
                    case(THERM_HEAT):
                        targetState->setVal(THERM_COOL);
                        break;
                    case(THERM_COOL):
                    default:
                        break;
                        // do nothing
                }
                if (fabs(coolTemp - targetTemp->getVal<double>()) > 0.2) {
	    			    printf("ThermostatView::ThermostatController - Cool Thermostat #%d: targetTemp = %f\n", model->index(), coolTemp);
		    		    targetTemp->setVal(coolTemp);
		        }
                
                break;
            case (RVCMode::HEAT):
                switch(homeKitOperatingMode) {
                    case(THERM_OFF):
                    case(THERM_AUTO):
                    case(THERM_COOL):
                        targetState->setVal(THERM_HEAT);
                        break;
                    case(THERM_HEAT):
                    default:
                        break;
                        // do nothing
                }
                if (fabs(heatTemp - targetTemp->getVal<double>()) > 0.2) {
	    			    printf("ThermostatView::ThermostatController - Heat Thermostat #%d: targetTemp = %f\n", model->index(), heatTemp);
		    		    targetTemp->setVal(heatTemp);
		        }
                break;
            case (RVCMode::AUTO):
                switch(homeKitOperatingMode) {
                    case(THERM_OFF):
                    case(THERM_COOL):
                    case(THERM_HEAT):
                        targetState->setVal(THERM_AUTO);
                        break;
                    case (THERM_AUTO):
                    default:
                        // do nothing
                        break;
                }
                if (ambientTemp->getVal() > coolTemp) 
                    targetTemp->setVal(coolTemp);
                else if (ambientTemp->getVal() < heatTemp)
                    targetTemp->setVal(heatTemp);
                break;
            case (RVCMode::OFF):
                switch(homeKitOperatingMode) {
                    case(THERM_COOL):
                    case(THERM_HEAT):
                    case(THERM_AUTO):
                        targetState->setVal(THERM_OFF);
                        break;
                    case(THERM_OFF):
                    default:
                        // do nothing
                        break;
                }
                break;
            default:
                break;
        }
        fan->setModeSpeed(fanMode, fanSpeed);
        PacketQueue::clearLastPacketReceiveTime();
    }
}

#include "ChassisMobility.h"

bool ThermostatView::updateView(void) {
        // 
    // printf("DoorLockView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        HVAC_Thermostat* mdl = (HVAC_Thermostat* )getModel();
        if (mdl != nullptr) {
            RVCFanMode fanMode = mdl->getFanMode();
            RVCMode opMode = mdl->getOperatingMode();
            RVCForcedFan fanSpeed = mdl->getFanSpeed();
            float coolTemp = mdl->getCoolTemp();
            float heatTemp = mdl->getHeatTemp();
            controller.setInfo(opMode, fanMode, fanSpeed, heatTemp, coolTemp);
            updated = true; 
        }
    }
    // printf("DoorLockView::updateView completed \n"); 
    return updated;
 }

 ThermostatView::ThermostatView(GenericDevice* model, const char* spanDevName) : SpanView(model), controller(this, model, spanDevName) {

 }

 void ThermostatView::createThermostatView(GenericDevice* model, const char* spanDevName) {
    printf("ThermostatView::createThermostatView called\n");
    SpanView::prepHomeSpan();
    // SPAN_ACCESSORY(spanDevName);
    ThermostatView::createCategory();
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit);
    // new Characteristic::ConfiguredName(spanDevName);
    // SPAN_ACCESSORY(spanDevName);
    ThermostatView* tmp = new ThermostatView(model, spanDevName);
    if (tmp != nullptr)
        printf("ThermostatView::createThermostatView: tmp created successfully\n");
    else
        printf("ThermostatView::createThermostatView: tmp creation failed\n");   
    printf("ThermostatView::createThermostatView completed\n");
}