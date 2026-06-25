#ifdef HOME_KIT
#include "Shades.h"
#include "ShadesView.h"


#include "ChassisMobility.h"
#include "PacketQueue.h"


const uint8_t ShadesView::FULLY_OPEN_SHADE  = 100;
const uint8_t ShadesView::CLOSED_SHADE      = 0;   
const uint8_t ShadesView::STEP_SHADE        = 10; // step value for HomeKit position changes

void ShadesView::ShadesController::openShade(void) {
    if ((model != nullptr) && (!model->isInInterlock()) && 
        (ChassisMobility::isParked()) && (!model->isShadeLocked()) && (!model->isInOverCurrent())) {
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
}
void ShadesView::ShadesController::closeShade(void) {
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
}
void ShadesView::ShadesController::stopShade(void) {
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
}

  
boolean ShadesView::ShadesController::update() {                              // update() method
    boolean result = false;
    view->dontUpdateTheView(); 
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
    return result;                               // return true
  
} // update

void ShadesView::ShadesController::loop() {                                     // loop() method

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
   
} // loop

#endif // HOME_KIT