
#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "elapsedMillis.h"
#include "AwningView.h"
#include "Awning.h"

/*
uint64_t millis64() {
	volatile static uint32_t low32 = 0, high32 = 0;
	uint32_t new_low32 = millis();

	if (new_low32 < low32)
		high32++;

	low32 = new_low32;

	return (uint64_t) high32 << 32 | low32;
}
    */

bool AwningView::bridgeCreated = false;

void AwningView::createBridge(void) {
    if (!AwningView::bridgeCreated) {
        printf("DoorLockView::createBridge called\n");
        // homeSpan.begin(Category::Locks,"HomeSpan Locks");
        // create the bridge for the light switch
        // homeSpan.begin(Category::Bridges, "RV-Bridge-On-Off-Switch", DEFAULT_HOST_NAME, "RV-Bridge-ESP32");
        // new SpanAccessory(); 
        // new Service::AccessoryInformation();
        // new Characteristic::Identify();
        AwningView::bridgeCreated = true;
        printf("DoorLockView::createBridge completed\n");
    }
}

void AwningView::AwningController::extendAwning(float amount) {
    if ((model != nullptr) && (isReadyToExtend())) {
        // uint8_t amt = amount; 
        // fully Extended = 0% in Homekit but 100 in RVC, so if the number is 0, we want to extend to 100, if the number is 25, then we extend to 75
        uint8_t amt = AWNING_MAX_PERCENT - amount; // fully extended = 0 in HomeKit so, to extend to 100% in RVX, we need to subtract the amount from the Max
        // printf("AwningView::AwningController::extendAwning %d\n", amt);
        // out->setVal(true); // set the out characteristic to true to indicate we are extending
        model->extend(amt);
    }
}
void AwningView::AwningController::retractAwning(float amount) {
    if ((model != nullptr) && (isReadyToRetract())) {
        // uint8_t amt = AWNING_MAX_PERCENT + amount; 
        // fully retracted = 100 in HomeKit but 0 in RCX, so we want to retract to amount from homekit. So if the amount is 100, we retract to 0, if the amount is 75, we retract to 25
        uint8_t amt = AWNING_MAX_PERCENT - amount; // fully retracted = 100 in HomeKit so, to retract to 0% in RCC, we need to subtract the amount
        // printf("AwningView::retractAwining %d\n", amt);
        //out->setVal(false); // set the out characteristic to false to indicate we are retracting
        // if (amt >= AWNING_MIN_PERCENT)
        model->retract(amt);
    }
}

void AwningView::AwningController::stopAwning(void) {
    if ((model != nullptr) && (isReadyToStop())) {
        printf("AwningView::AwningController::stopAwning\n");
        model->stop();
    }
}

#include "ChassisMobility.h"

boolean AwningView::AwningController::update(void) {
    boolean updated = false;

    view->dontUpdateTheView(); 
    currentExtendedPosition = currentState->getVal<float>();
    /*
    if (out->updated() && (model != nullptr) && (ChassisMobility::isParked())) {
        boolean outValue = out->getNewVal<boolean>();
        printf("AwningView::AwningController::update outValue = %d\n", outValue);
        if (outValue) {
            // fully extend the awning
            clearCommands();
            readyToExtend();
            targetExtendedPosition = AWNING_MIN_PERCENT;
            travelTime = timeToFullyExtend;
            printf("AwningView::AwningController::update fully extend, travelTime = %d\n", travelTime);
        } else {
            // fully retract the awning
            clearCommands();
            readyToRetract();
            targetExtendedPosition = AWNING_MAX_PERCENT;
            travelTime = timeToFullyRetract;
            printf("AwningView::AwningController::update fully retract, travelTime = %d\n", travelTime);
        }
    } else */
    if ((targetState->updated()) && (model != nullptr) && (ChassisMobility::isParked())) {
        float targetValue = targetState->getNewVal<float>();
        updated = true;
        // printf("AwningView::AwningController::update targetValue = %f, currentExtendedPosition = %f\n", targetValue, currentExtendedPosition);
        // is target value > to current position? -> remember 100% is fully retracted in HOMEKIT
        // so, it target value > current position, then it needs to retract]

        if (targetValue < currentExtendedPosition) {
            // printf("AwningView::AwningController::update Retract to %f\n", targetValue);
            clearCommands();
            readyToRetract();
            targetExtendedPosition = targetValue;
            travelTime = (uint16_t)((currentExtendedPosition - targetValue) * timeToFullyExtend / 100.0F);
            // targetTime = (targetValue - )/100.0F * timeToFullyExtend gives us the time needed to extend to target
            //travelTime = (uint16_t)((targetValue - currentExtendedPosition ) * timeToFullyRetract / 100.0F);
            // printf("AwningView::AwningController::update Retract to %f, travelTime = %d\n", targetValue, travelTime);
        } else if (targetValue > currentExtendedPosition) {
            // printf("AwningView::AwningController::update to Extend to %f\n", targetValue);
            clearCommands();
            readyToExtend();
            travelTime = (uint16_t)((targetValue - currentExtendedPosition ) * timeToFullyRetract / 100.0F);
            //travelTime = (uint16_t)((currentExtendedPosition - targetValue) * timeToFullyExtend / 100.0F);
            targetExtendedPosition = targetValue;
            // printf("AwningView::AwningController::update Extend to %f, travelTime = %d\n", targetValue, travelTime);
        } else { 
            // target is same as current
            clearCommands();
            readyToStop();
            travelTime = 0;

        }
    }
    view->updateTheView();
	return updated;
}

void AwningView::AwningController::loop(void) {
    // if (model != nullptr) {
    uint8_t updatedPosition = currentState->getVal<uint8_t>();
    /**
    if((currentState->getVal() != targetState->getVal()) && (targetState->timeVal() > travelTime)){          // if 5 seconds have elapsed since the target-position was last modified...
      currentState->setVal(targetState->getVal());                                        // ...set the current position to equal the target position
    } else {

    }
    */

    if (!isReadyToStop() && (ChassisMobility::isParked())) {
        if (isReadyToExtend()) { // && (!isAwningInMotion())) {
            // start the extend
            //out->setVal(true); // set the out characteristic to true to indicate we are extending
            // extending(true);
            view->dontUpdateTheView(); 
            // printf("AwningView::AwningController::loop starting extend travelTime %d\n", travelTime);
            if (!isAwningInMotion()) { 
                // printf("AwningView::AwningController::loop starting extend - not moving so startMoving\n");
                if (!isReadyToStop()) {
                    // printf("AwningView::AwningController::loop starting extend - not stopping so extend\n");
                    extendAwning(targetExtendedPosition);
                    startMoving();
                    delay(30);
                }
            } else {
                // current position is (travel time - start time)/traveltime *100 + current x * 1+%
                unsigned long stime = startTime;
                float percent = (travelTime - stime);
                percent = 1.0F - (percent/travelTime);
                // printf ("AwningView::AwningController::loop extending progress percent = %f\n", percent * 100.0F);
                float extendedPosition = (percent * (static_cast<float>(AWNING_FULLY_RETRACTED_PCT) - targetExtendedPosition));
                // printf("AwningView::AwningController::loop extending, progress = startTime %d, travelTime %d, position %f\n", stime, travelTime, extendedPosition);
                updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(extendedPosition));  
                // printf("AwningView::AwningController::loop extending, current position= %d, targetExtendedPosition= %f\n", updatedPosition, targetExtendedPosition);            
                currentState->setVal(updatedPosition);
                currentExtendedPosition = (static_cast<float>(updatedPosition));
                // printf("AwningView::AwningController::loop extend, current position= %f, targetExtendedPosition= %f\n", currentExtendedPosition, targetExtendedPosition);
                extendAwning(targetExtendedPosition);
                delay(30);
            }
            // if (!isReadyToStop()) {
                // model->extend(targetExtendedPosition);
            //     extendAwning(targetExtendedPosition);
            // }
            view->updateTheView();
        } else if ((isReadyToRetract())) { // && (!isAwningInMotion())) {
            // start the retract
            // extending(false); // set the out characteristic to false to indicate we are retracting
            view->dontUpdateTheView(); 
            // printf("AwningView::AwningController::loop starting retract travelTime %d\n", travelTime);
            if (!isAwningInMotion()) {
                // printf("AwningView::AwningController::loop starting retract - not moving so startMoving\n");
                if (!isReadyToStop()) {
                    // printf("AwningView::AwningController::loop starting retract - not stopping so retrat\n");
                    retractAwning(targetExtendedPosition);
                    startMoving();
                    delay(30);
                }
            } else {
                 // current position is (travel time - start time)/traveltime *100 + current x * 1+%
                unsigned long stime = startTime;
                float percent = (travelTime - stime);
                percent = 1.0F - (percent/travelTime);
                // printf ("AwningView::AwningController::loop retracting progress percent = %f\n", percent * 100.0F);
                float extendedPosition = (1.0F - percent) * targetExtendedPosition; // (currentExtendedPosition- targetExtendedPosition);
                // printf("AwningView::AwningController::loop retracting progress = startTime %d, travelTime %d, position %f\n", stime, travelTime, extendedPosition);
                updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(extendedPosition));

                currentState->setVal(updatedPosition);
                currentExtendedPosition = extendedPosition;
                // printf("AwningView::AwningController::loop retract, current position= %f, targetExtendedPosition= %f\n", currentExtendedPosition, targetExtendedPosition);
                retractAwning(targetExtendedPosition);
                delay(30);
            }
            // if (!isReadyToStop()) {
            //    printf("AwningView::AwningController::loop starting retract - not stopping so retract\n");
                // model->retract(targetExtendedPosition); 
            //    retractAwning(targetExtendedPosition);
            // }
            view->updateTheView();

        } else if ((isReadyToStop())) { // && (isAwningInMotion())) {
            // stop the awning
            view->dontUpdateTheView(); 
            // model->stop();
            // printf("AwningView::AwningController::loop stopping awning\n");
            stopMoving();
            clearCommands();
            // currentExtendedPosition = static_cast<float>(AWNING_FULLY_RETRACTED_PCT) - targetExtendedPosition; // fully retracted = 100 in HomeKit but 0 in RCC, so we want to retract to amount from homekit. So if the amount is 100, we retract to 0, if the amount is 75, we retract to 25
            // updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(currentExtendedPosition));
            //currentState->setVal(updatedPosition);
            // printf("AwningView::AwningController::loop stopping currentExtendedPosition = %f\n", currentExtendedPosition);
            view->updateTheView();
            // out->setVal(isAwningExtended());
        } else {
            //if (currentExtendedPosition != targetExtendedPosition)
            // printf("AwningView::AwningController::loop else: currentExtendedPosition = %f\n", currentExtendedPosition);
            // currentExtendedPosition = targetExtendedPosition;

            // updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(currentExtendedPosition));
            // currentState->setVal(updatedPosition);
        }
        // currentState->setVal(targetExtendedPosition);
    }
} 

boolean AwningView::AwningController::isAwningExtended(void) { 
    // return targetState->getNewVal() == AwningView::FULLY_OPEN_SHADE; 
    boolean isExtended = (currentExtendedPosition > 0.0F);
    if (model != nullptr) {
        isExtended = model->isExtended();
    }
    return isExtended;
}

boolean AwningView::AwningController::isAwningRetracted(void) { 
    // return targetState->getNewVal() == AwningView::CLOSED_SHADE; 
    boolean isRetracted = (currentExtendedPosition == static_cast<float>(AWNING_MAX_PERCENT));
    return isRetracted;
}

boolean AwningView::AwningController::isAwningFullyExtended(void) {
    boolean isFullyExtended = (currentExtendedPosition == static_cast<float>(AWNING_MIN_PERCENT));
    return isFullyExtended;
}
boolean AwningView::AwningController::isAwningFullyRetracted(void) {
    boolean isFullyRetracted = isAwningRetracted();
    return isFullyRetracted;
}

// AwningExtendRetractController is a service that allows the awning to be extended or retracted fully with a single command. It is used in conjunction with the AwningController which controls the position of the awning. The AwningExtendRetractController has an "out" characteristic that can be set to true to extend the awning fully or false to retract it fully. It also has an update method that checks if the "out" characteristic has been updated and sends the appropriate command to the AwningController.

boolean AwningView::AwningExtendRetractController::update(void) {
    boolean updated = false;
    if (view != nullptr) {
        view->dontUpdateTheView(); 
    
        if (out->updated() && (awningController != nullptr) && (ChassisMobility::isParked())) {
            boolean outValue = out->getNewVal<boolean>();
            // printf("AwningView::AwningExtendRetractController::update outValue = %d\n", outValue);
            if (outValue) {
                // fully extend the awning
                awningController->clearCommands();
                awningController->readyToExtend();
                awningController->targetExtendedPosition = AWNING_MIN_PERCENT;
                awningController->travelTime = awningController->timeToFullyExtend;
                //printf("AwningView::AwningExtendRetractController::update fully extend, travelTime = %d\n", awningController->travelTime);
            } else {
                // fully retract the awning
                awningController->clearCommands();
                awningController->readyToRetract();
                awningController->targetExtendedPosition = AWNING_MAX_PERCENT;
                awningController->travelTime = awningController->timeToFullyRetract;
                // printf("AwningView::AwningExtendRetractController::update fully retract, travelTime = %d\n", awningController->travelTime);
            }
            updated = true;
        }
        view->updateTheView();
    }
    return updated;
}

void AwningView::AwningExtendRetractController::loop(void) {
    if (awningController != nullptr) {
        awningController->loop();
    }
}

/**
void AwningView::AwningController::targetUpdate() {
	uint64_t curTime = millis64();

	float targetValue = targetState->getVal<float>();

	printf("AwningView::AwningController::targetUpdate - target=%0.1f, currentTarget=%0.1f\n", targetValue, currentAwningTarget);

	if (targetValue != currentAwningTarget) {
		// float moveAmount = targetValue - currentAwningPosition;

		printf("AwningView::AwningController::targetUpdate: moveAmount=%0.1f\n", moveAmount);

		if (moveAmount != 0) {
			// AwningState newDirection = (moveAmount > 0) ? AwningStateRetracting : AwningStateExtending;

			//if (newDirection == (awningState & AwningStateMoving)) { // currently moving correct direction
			//	awningState = AwningStateHomeKitAction | newDirection;
			//	currentAwningTarget = targetValue;
			//	printf("AwningView::AwningController::targetUpdate - New target position %0.1f%%\n", targetValue);
			//} else 
            if (moveAmount > 0) { // currently moving opposite direction
				// printf("AwningView::AwningController::targetUpdate - Wrong direction, stop movement, queue move\n");
				// if (awningState & AwningStateExtending) {	// stop movement
                        extendAwning(moveAmount);
				// } else {
            } else {
                        retractAwning(moveAmount);
				//}
            }
				// currentState->setVal(currentAwningPosition);
				awningState = 0;
				targetUpdateAtTime = curTime + awningOutputTimeMS + awningUpdateHoldTimeMS;
			//} else { // not currently moving
				// awningState = AwningStateHomeKitAction | newDirection;
			//	currentAwningTarget = targetValue;

			//	printf("AwningView::AwningController::targetUpdate - Change position from %0.1f%% to %0.1f%%\n", currentAwningPosition, targetValue);
			//	printf("AwningView::AwningController::targetUpdate - State changed to 0x%02X\n", awningState);

			//	if (awningState & AwningStateExtending) {
			//		extendAwning();
			//	} else {
			//		retractAwning();
			//	}
			// }
		}
	}
}
    */


#include "ChassisMobility.h"
bool AwningView::updateView(void) {
    // the light switch may have been turned on/off at the wall and thus needs to be reflected in the SpanView
    // 
    // printf("AwningView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        Awning* mdl = (Awning* )getModel();
        if (mdl != nullptr) { 
            // printf("AwningView::updateView - mdl not null\n");
            index = mdl->index();
            if (extendRetractController != nullptr) {
                // printf("AwningView::updateView - mdl not null and extendRetractController not null\n");
                if ((controller != nullptr) && (controller->isAwningExtended())) {
                    // printf("AwningView::updateView - mdl not null and controller not null and awning is extended\n");
                    extendRetractController->extending(true);
                } else {
                    // printf("AwningView::updateView - mdl not null and controller not null and awning is NOT extended\n");
                    extendRetractController->extending(false);
                }
            }
            //PacketQueue::clearLastPacketReceiveTime();
            // controller->currentState
            // if (mdl->isExtending() || mdl->isStopped()) {
            //    uint8_t amtExtended = mdl->extendedAmount();
                // controller->currentState->setVal(amtExtended); 
            //    if (mdl->isStopped())
            //        ; // controller->targetState->setVal(amtExtended);
            // } else if (mdl->isRetracting()) {
                //uint8_t amtRetracted = mdl->retractedAmount();
                // controller->currentState->setVal(amtRetracted);
            // } 

            // PacketQueue::clearLastPacketReceiveTime();
            // if (index == 1)
                // printf("AwningView::updateView - locked = %d, spancharCurrentLockState->getVal() = %d\n", locked, controller.isLocked());
            updated = true;
        }        
    }

    // printf("AwningView::updateView completed \n"); 
    return updated;
}


AwningView::AwningView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), controller(nullptr) {
}

void AwningView::createAwningView(GenericDevice* model, const char* spanDevName, float extTime, float retTime) {
    printf("AwningView::createAwningView called\n");
    SpanView::prepHomeSpan();
    AwningView::createBridge();


    AwningView* vw = new AwningView(model, spanDevName);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    AwningView::AwningController* ctrl = new AwningView::AwningController(vw, model, spanDevName, extTime, retTime);
    vw->setController(ctrl);
    AwningView::AwningExtendRetractController* extCtrl = new AwningView::AwningExtendRetractController(vw, model, ctrl);
    vw->setExtendRetractController(extCtrl);    

    if (vw != nullptr)
        printf("AwningView::createAwningView: tmp created successfully\n");
    else
        printf("AwningView::createAwningView: tmp creation failed\n");   
    printf("AwningView::createAwningView completed\n");
}
#endif
