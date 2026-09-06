#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Shades.h"
#include "ShadesView.h"


#include "ChassisMobility.h"
#include "PacketQueue.h"


const uint8_t ShadesView::FULLY_OPEN_SHADE  = 100;
const uint8_t ShadesView::CLOSED_SHADE      = 0;   
const uint8_t ShadesView::STEP_SHADE        = 10; // step value for HomeKit position changes


bool ShadesView::bridgeCreated = false;

void ShadesView::createBridge(void) {
    if (!ShadesView::bridgeCreated) {
        printf("ShadesView::createBridge called\n");
        // homeSpan.begin(Category::Locks,"HomeSpan Locks");
        // create the bridge for the light switch
        // homeSpan.begin(Category::Bridges, "RV-Bridge-On-Off-Switch", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
        // new SpanAccessory(); 
        // new Service::AccessoryInformation();
        // new Characteristic::Identify();
        ShadesView::bridgeCreated = true;
        printf("ShadesView::createBridge completed\n");
    }
}

void ShadesView::ShadesController::openShade(float amount) {
    if ((model != nullptr) && (isReadyToOpen())) {
        // uint8_t amt = amount; 
        // fully Extended = 0% in Homekit but 100 in RVC, so if the number is 0, we want to extend to 100, if the number is 25, then we extend to 75
        uint8_t amt = SHADES_MAX_PERCENT - amount; // fully extended = 0 in HomeKit so, to extend to 100% in RVX, we need to subtract the amount from the Max
        // printf("AwningView::AwningController::extendAwning %d\n", amt);
        // out->setVal(true); // set the out characteristic to true to indicate we are extending
        model->open(amt);
    }
    /* 
        uint8_t* rawData = model->getCommandData();
        uint8_t duty = (uint8_t)(FULLY_OPEN_SHADE * ((targetState->getNewVal()) / FULLY_OPEN_SHADE));
        printf("ShadesView::ShadesController::openShade Raising Shade - motor-duty: %d\n", duty);
        model->setMotorDuty(duty);
        model->setShadeCommand(SHADE_COMMAND_REVERSE); // always use REVERSE command - motor-duty determines direction
        model->executeCommand(WINDOW_SHADE_CONTROL_COMMAND, rawData);
        // model->setLockedFlag(isLocked());
        // printf("DoorLockView::DoorLockController::update - on: %d\n", model->getLockedFlag());
        // model->executeCommand(LOCK_COMMAND, rawData);
    }
    */
}
void ShadesView::ShadesController::closeShade(float amount) {
        if ((model != nullptr) && (isReadyToClose())) {
        // uint8_t amt = AWNING_MAX_PERCENT + amount; 
        // fully retracted = 100 in HomeKit but 0 in RCX, so we want to retract to amount from homekit. So if the amount is 100, we retract to 0, if the amount is 75, we retract to 25
        uint8_t amt = SHADES_MAX_PERCENT - amount; // fully retracted = 100 in HomeKit so, to retract to 0% in RCC, we need to subtract the amount
        // printf("AwningView::retractAwining %d\n", amt);
        //out->setVal(false); // set the out characteristic to false to indicate we are retracting
        // if (amt >= AWNING_MIN_PERCENT)
        model->close(amt);
    }
    /* 
    if ((model != nullptr) && (!model->isInInterlock()) && 
        (ChassisMobility::isParked()) && (!model->isShadeLocked()) && (!model->isInOverCurrent())) {
        uint8_t* rawData = model->getCommandData();
        uint8_t duty = (uint8_t)(FULLY_OPEN_SHADE * ((FULLY_OPEN_SHADE - targetState->getNewVal()) / FULLY_OPEN_SHADE));
        printf("ShadesView::ShadesController::closeShade Lowering Shade - motor-duty: %d\n", duty);
        model->setMotorDuty(duty);
        model->setShadeCommand(SHADE_COMMAND_FORWARD); // always use FORWARD command - motor-duty determines direction
        model->executeCommand(WINDOW_SHADE_CONTROL_COMMAND, rawData);
        // model->setLockedFlag(isLocked());
        // printf("DoorLockView::DoorLockController::update - on: %d\n", model->getLockedFlag());
        // model->executeCommand(LOCK_COMMAND, rawData);
    }
    */
}

void ShadesView::ShadesController::stopShade(void) {
    if ((model != nullptr) && (isReadyToStop())) {
        printf("AwningView::AwningController::stopAwning\n");
        model->stop();
    }
    /*
    if ((model != nullptr) && (!model->isInInterlock()) && 
        (ChassisMobility::isParked()) && (!model->isShadeLocked()) && (!model->isInOverCurrent())) {
        uint8_t* rawData = model->getCommandData();
        model->setMotorDuty(0); // zero motor-duty to stop the shade
        model->setShadeCommand(SHADE_COMMAND_STOP);
        model->executeCommand(WINDOW_SHADE_CONTROL_COMMAND, rawData);
        // model->setLockedFlag(isLocked());
        // printf("DoorLockView::DoorLockController::update - on: %d\n", model->getLockedFlag());
        // model->executeCommand(LOCK_COMMAND, rawData);
    } 
    */  
}

  
boolean ShadesView::ShadesController::update() {                              // update() method
    boolean updated = false;
    view->dontUpdateTheView(); 
    /*
    if ((model != nullptr) && (!model->isInInterlock()) && 
        (ChassisMobility::isParked()) && (!model->isShadeLocked()) && (!model->isInOverCurrent())) {
        uint8_t* rawData = model->getCommandData();
        if (targetState->getNewVal() < FULLY_OPEN_SHADE) {
            uint8_t duty = (uint8_t)(FULLY_OPEN_SHADE * ((FULLY_OPEN_SHADE - targetState->getNewVal()) / FULLY_OPEN_SHADE));
            printf("ShadesView::ShadesController::update Lowering Shade - motor-duty: %d\n", duty);
            model->setMotorDuty(duty);
        } 
        model->setShadeCommand(SHADE_COMMAND_FORWARD); // always use FORWARD command - motor-duty determines direction
        model->executeCommand(WINDOW_SHADE_CONTROL_COMMAND, rawData);
        // model->setLockedFlag(isLocked());
        // printf("DoorLockView::DoorLockController::update - on: %d\n", model->getLockedFlag());
        // model->executeCommand(LOCK_COMMAND, rawData);
        result = true;
    }
    */
    currentClosedPosition = currentState->getVal<float>();
    if ((targetState->updated()) && (model != nullptr) && (ChassisMobility::isParked())) {
        float targetValue = targetState->getNewVal<float>();
        updated = true;
        // printf("AwningView::AwningController::update targetValue = %f, currentExtendedPosition = %f\n", targetValue, currentExtendedPosition);
        // is target value > to current position? -> remember 100% is fully retracted in HOMEKIT
        // so, it target value > current position, then it needs to retract]

        if (targetValue < currentClosedPosition) {
            // printf("AwningView::AwningController::update Retract to %f\n", targetValue);
            clearCommands();
            readyToOpen();
            targetClosedPosition = targetValue;
            // travelTime = (uint16_t)((currentClosedPosition - targetValue) * timeToFullyClose / 100.0F);
            // targetTime = (targetValue - )/100.0F * timeToFullyExtend gives us the time needed to extend to target
            // travelTime = (uint16_t)((targetValue - currentExtendedPosition ) * timeToFullyRetract / 100.0F);
            // printf("AwningView::AwningController::update Retract to %f, travelTime = %d\n", targetValue, travelTime);
        } else if (targetValue > currentClosedPosition) {
            // printf("AwningView::AwningController::update to Extend to %f\n", targetValue);
            clearCommands();
            readyToClose();
            // travelTime = (uint16_t)((targetValue - currentClosedPosition ) * timeToFullyOpen / 100.0F);
            //travelTime = (uint16_t)((currentExtendedPosition - targetValue) * timeToFullyExtend / 100.0F);
            targetClosedPosition = targetValue;
            // printf("AwningView::AwningController::update Extend to %f, travelTime = %d\n", targetValue, travelTime);
        } else { 
            // target is same as current
            clearCommands();
            readyToStop();
            travelTime = 0;

        }
    }

    view->updateTheView();
    return updated;                               // return true
  
} // update

void ShadesView::ShadesController::loop() {                                     // loop() method

    /*
    if ((model != nullptr) && (!model->isInInterlock()) && 
        (ChassisMobility::isParked()) && (!model->isShadeLocked()) && (!model->isInOverCurrent())) {
        uint8_t* rawData = model->getCurrentData();
        if (rawData != nullptr) {
            // update the current position based on the motor-duty and the shade command
            uint8_t motorDuty = model->motorDuty(); // 0-100%
            SHADES_COMMANDS shadeCmd = model->shadeCommand();
            if (shadeCmd == SHADE_COMMAND_FORWARD) { // closing the shade
                if (motorDuty > 0) {
                    currentShadePosition -= (motorDuty * STEP_SHADE / FULLY_OPEN_SHADE); // move the shade position down based on motor-duty
                    if (currentShadePosition < CLOSED_SHADE)
                        currentShadePosition = CLOSED_SHADE;
                }
            } else if (shadeCmd == SHADE_COMMAND_REVERSE) { // opening the shade
                if (motorDuty > 0) {
                    currentShadePosition += (motorDuty * STEP_SHADE / FULLY_OPEN_SHADE); // move the shade position up based on motor-duty
                    if (currentShadePosition > FULLY_OPEN_SHADE)
                        currentShadePosition = FULLY_OPEN_SHADE;
                }
            } else { // SHADE_COMMAND_STOP or unknown command
                // do nothing - shade is not moving
            }
            // update the currentState characteristic with the new position
            currentState->setVal((uint8_t)currentShadePosition);
            // check if we have reached the target position
            if (currentShadePosition == targetState->getNewVal()) {
                // we have reached the target position, stop the shade
                stopShade();
            }
        }
    } 
    */
    uint8_t updatedPosition = currentState->getVal<uint8_t>();
    if (!isReadyToStop() && (ChassisMobility::isParked())) {
        if (isReadyToClose()) { // && (!isAwningInMotion())) {
            // start the close
            view->dontUpdateTheView(); 
            // printf("AwningView::AwningController::loop starting extend travelTime %d\n", travelTime);
            if (!isShadeInMotion()) { 
                // printf("AwningView::AwningController::loop starting extend - not moving so startMoving\n");
                if (!isReadyToStop()) {
                    // printf("AwningView::AwningController::loop starting extend - not stopping so extend\n");
                    closeShade(targetClosedPosition);
                    startMoving();
                    delay(DELAY_TIME);
                }
            } else {
                // current position is (travel time - start time)/traveltime *100 + current x * 1+%
                unsigned long stime = startTime;
                float percent = (travelTime - stime);
                percent = 1.0F - (percent/travelTime);
                // printf ("AwningView::AwningController::loop extending progress percent = %f\n", percent * 100.0F);
                float closedPosition = (percent * (static_cast<float>(SHADES_FULLY_CLOSED_PCT) - targetClosedPosition));
                // printf("AwningView::AwningController::loop extending, progress = startTime %d, travelTime %d, position %f\n", stime, travelTime, extendedPosition);
                updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(closedPosition));  
                // printf("AwningView::AwningController::loop extending, current position= %d, targetExtendedPosition= %f\n", updatedPosition, targetExtendedPosition);            
                currentState->setVal(updatedPosition);
                currentClosedPosition = (static_cast<float>(updatedPosition));
                // printf("AwningView::AwningController::loop extend, current position= %f, targetExtendedPosition= %f\n", currentExtendedPosition, targetExtendedPosition);
                closeShade(targetClosedPosition);
                delay(DELAY_TIME);
            }
            view->updateTheView();
        } else if ((isReadyToOpen())) { // && (!isAwningInMotion())) {
            // start the retract
            // extending(false); // set the out characteristic to false to indicate we are retracting
            view->dontUpdateTheView(); 
            // printf("AwningView::AwningController::loop starting retract travelTime %d\n", travelTime);
            if (!isShadeInMotion()) {
                // printf("AwningView::AwningController::loop starting retract - not moving so startMoving\n");
                if (!isReadyToStop()) {
                    // printf("AwningView::AwningController::loop starting retract - not stopping so retrat\n");
                    openShade(targetClosedPosition);
                    startMoving();
                    delay(DELAY_TIME);
                }
            } else {
                 // current position is (travel time - start time)/traveltime *100 + current x * 1+%
                unsigned long stime = startTime;
                float percent = (travelTime - stime);
                percent = 1.0F - (percent/travelTime);
                // printf ("AwningView::AwningController::loop retracting progress percent = %f\n", percent * 100.0F);
                float closedPosition = (1.0F - percent) * targetClosedPosition; // (currentExtendedPosition- targetExtendedPosition);
                // printf("AwningView::AwningController::loop retracting progress = startTime %d, travelTime %d, position %f\n", stime, travelTime, extendedPosition);
                updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(closedPosition));

                currentState->setVal(updatedPosition);
                currentClosedPosition = closedPosition;
                // printf("AwningView::AwningController::loop retract, current position= %f, targetExtendedPosition= %f\n", currentExtendedPosition, targetExtendedPosition);
                openShade(targetClosedPosition);
                delay(DELAY_TIME);
            }
            view->updateTheView();
        } else if ((isReadyToStop())) { // && (isAwningInMotion())) {
            // stop the awning
            view->dontUpdateTheView(); 
            // printf("AwningView::AwningController::loop stopping awning\n");
            stopMoving();
            clearCommands();
            // printf("AwningView::AwningController::loop stopping currentExtendedPosition = %f\n", currentExtendedPosition);
            view->updateTheView();
        } else {
            //if (currentExtendedPosition != targetExtendedPosition)
            // printf("AwningView::AwningController::loop else: currentExtendedPosition = %f\n", currentExtendedPosition);
        }
    }
} // loop


boolean ShadesView::ShadesDownController::update() {                              // update() method
    boolean result = false;
    view->dontUpdateTheView();     
    if (down->updated() && (controller != nullptr) && (ChassisMobility::isParked())) {
        boolean downValue = down->getNewVal<boolean>();
        // printf("AwningView::AwningExtendRetractController::update outValue = %d\n", outValue);
        if (!downValue) {
            // fully open the shades
            controller->clearCommands();
            controller->readyToOpen();
            controller->targetClosedPosition = SHADES_MIN_PERCENT;
            // controller->travelTime = controller->timeToFullyClose;
            //printf("AwningView::AwningExtendRetractController::update fully extend, travelTime = %d\n", awningController->travelTime);
        } else {
            // fully retract the awning
            controller->clearCommands();
            controller->readyToClose();
            controller->targetClosedPosition = SHADES_MAX_PERCENT;
            // controller->travelTime = controller->timeToFullyOpen;
            // printf("AwningView::AwningExtendRetractController::update fully retract, travelTime = %d\n", awningController->travelTime);
        }
        result = true;
    }
    view->dontUpdateTheView();
    return result;                               // return true
  
} // update

void ShadesView::ShadesDownController::loop(void) {
    if (controller != nullptr) {
        controller->loop();
    }
}

/**
boolean ShadesView::ShadesStopController::update() {                              // update() method
    boolean result = false;
    view->dontUpdateTheView();     
    if (stop->updated() && (controller != nullptr) && (ChassisMobility::isParked())) {
        boolean stopValue = stop->getNewVal<boolean>();
        // printf("ShadesView::ShadesStopController::update stopValue = %d\n", stopValue);
        if (stopValue) {
            // fully open the shades
            controller->clearCommands();
            controller->readyToStop();
            //printf("ShadesView::ShadesStopController::update stop\n");
        }
        result = true;
    }
    view->dontUpdateTheView();
    return result;                               // return true
  
} // update

void ShadesView::ShadesStopController::loop(void) {
    if (controller != nullptr) {
        controller->loop();
    }
}
*/


#include "ChassisMobility.h"
bool ShadesView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("ShadesView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        Shades* mdl = (Shades* )getModel();
        if (mdl != nullptr) { 
            // printf("ShadesView::updateView - mdl not null\n");
            index = mdl->index();
            if (downController != nullptr) {
                // printf("ShadesView::updateView - mdl not null and extendRetractController not null\n");
                if ((controller != nullptr) && (controller->isDown())) {
                    // printf("ShadesView::updateView - mdl not null and controller not null and shades is extended\n");
                    controller->closeShade(static_cast<float>(SHADES_MAX_PERCENT));
                } else {
                    // printf("ShadesView::updateView - mdl not null and controller not null and shades is NOT extended\n");
                    controller->openShade(static_cast<float>(SHADES_MIN_PERCENT));
                }
            }
            updated = true;
        }        
    }

    // printf("AwningView::updateView completed \n"); 
    return updated;
}

ShadesView::ShadesView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), controller(nullptr) {
}

void ShadesView::createShadesView(GenericDevice* model, const char* spanDevName) {
    printf("ShadesView::createShadesView called\n");
    SpanView::prepHomeSpan();
    ShadesView::createBridge();


    ShadesView* vw = new ShadesView(model, spanDevName);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    const char* append = " Down Switch (on == down, off == up)";
    size_t buffer_size = strlen(spanDevName) + strlen(append) + 1; 
    char* swName = new char[buffer_size];
    strcpy(swName, spanDevName);
    strcat(swName, append);
    ShadesView::ShadesController* ctrl = new ShadesView::ShadesController(vw, model, spanDevName);
    vw->setController(ctrl);
    ShadesView::ShadesDownController* extCtrl = new ShadesView::ShadesDownController(vw, model, ctrl,swName);
    vw->setDownController(extCtrl);  
    const char* append2 = " Down Switch (on == down, off == up)";
    buffer_size = strlen(spanDevName) + strlen(append2) + 1; 
    char* sw2Name = new char[buffer_size];
    strcpy(sw2Name, spanDevName);
    strcat(sw2Name, append2);
    // ShadesView::ShadesStopController* extCtrl2 = new ShadesView::ShadesStopController(vw, model, ctrl, sw2Name);
    //vw->setStopController(extCtrl2); 
    delete [] swName;
    delete [] sw2Name;

    if (vw != nullptr)
        printf("ShadesView::createShadesView: tmp created successfully\n");
    else
        printf("ShadesView::createShadesView: tmp creation failed\n");   
    printf("ShadesView::createShadesView completed\n");
}

#endif // HOME_KIT