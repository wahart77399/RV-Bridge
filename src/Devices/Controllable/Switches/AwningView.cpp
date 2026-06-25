
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
        printf("AwningView::AwningController::extendAwning %d\n", amt);
        model->extend(amt);
    }
}
void AwningView::AwningController::retractAwning(float amount) {
    if ((model != nullptr) && (isReadyToRetract())) {
        // uint8_t amt = AWNING_MAX_PERCENT + amount; 
        // fully retracted = 100 in HomeKit but 0 in RCX, so we want to retract to amount from homekit. So if the amount is 100, we retract to 0, if the amount is 75, we retract to 25
        uint8_t amt = AWNING_MAX_PERCENT - amount; // fully retracted = 100 in HomeKit so, to retract to 0% in RCC, we need to subtract the amount
        printf("AwningView::retractAwining %d", amt);
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

boolean AwningView::AwningController::update(void) {
    boolean updated = false;
    view->dontUpdateTheView(); 
    if ((targetState->updated()) && (model != nullptr)) {
        float targetValue = targetState->getNewVal<float>();
        // printf("AwningView::AwningController::update targetValue = %f, currentExtendedPosition = %f\n", targetValue, currentExtendedPosition);
        // is target value > to current position? -> remember 100% is fully retracted in HOMEKIT
        // so, it target value > current position, then it needs to retract
        if (targetValue > currentExtendedPosition) {
            // printf("AwningView::AwningController::update Retract to %f\n", targetValue);
            clearCommands();
            readyToRetract();
            targetExtendedPosition = targetValue;
            // targetTime = (targetValue - currentExtendedPosition)/100.0F * timeToFullyExtend gives us the time needed to extend to target
            travelTime = (uint16_t)((targetValue - currentExtendedPosition) * timeToFullyRetract / 100.0F);
        } else if (targetValue < currentExtendedPosition) {
            // printf("AwningView::AwningController::update to Extend to %f\n", targetValue);
            clearCommands();
            readyToExtend();
            travelTime = (uint16_t)((currentExtendedPosition - targetValue) * timeToFullyExtend / 100.0F);
            targetExtendedPosition = targetValue;
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
    uint8_t updatedPosition;
    if (!isReadyToStop()) {
        if (isReadyToExtend()) { // && (!isAwningInMotion())) {
            // start the extend
            view->dontUpdateTheView(); 
            // printf("AwningView::AwningController::loop starting extend travelTime %d\n", travelTime);
            if (!isAwningInMotion()) { 
                // printf("AwningView::AwningController::loop starting extend - not moving so startMoving\n");
                if (!isReadyToStop()) {
                    printf("AwningView::AwningController::loop starting extend - not stopping so extend\n");
                    extendAwning(targetExtendedPosition);
                    startMoving();
                    // delay(100);
                }
            } else {
                // current position is (travel time - start time)/traveltime *100 + current x * 1+%
                unsigned long stime = startTime;
                float percent = (travelTime - stime);
                percent = 1.0F - (percent/travelTime);
                printf ("progress percent = %f\n", percent * 100.0F);
                float extendedPosition = (percent * targetExtendedPosition);
                printf("progress = startTime %d, travelTime %d, position %f\n", stime, travelTime, extendedPosition);
                updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(extendedPosition));
                currentState->setVal(updatedPosition);
                printf("AwningView::AwningController::loop extend, current position= %d\n", updatedPosition);
                extendAwning(targetExtendedPosition);
                // delay(100);
            }
            // if (!isReadyToStop()) {
                // model->extend(targetExtendedPosition);
            //     extendAwning(targetExtendedPosition);
            // }
            view->updateTheView();
        } else if ((isReadyToRetract())) { // && (!isAwningInMotion())) {
            // start the retract
            view->dontUpdateTheView(); 
            // printf("AwningView::AwningController::loop starting retract travelTime %d\n", travelTime);
            if (!isAwningInMotion()) {
                // printf("AwningView::AwningController::loop starting retract - not moving so startMoving\n");
                if (!isReadyToStop()) {
                    printf("AwningView::AwningController::loop starting retract - not stopping so retrat\n");
                    retractAwning(targetExtendedPosition);
                    startMoving();
                    // delay(100);
                }
            } else {
                 // current position is (travel time - start time)/traveltime *100 + current x * 1+%
                unsigned long stime = startTime;
                float percent = (travelTime - stime);
                percent = 1.0F - (percent/travelTime);
                printf ("progress percent = %f\n", percent * 100.0F);
                float extendedPosition = (percent * targetExtendedPosition);
                printf("progress = startTime %d, travelTime %d, position %f\n", stime, travelTime, extendedPosition);
                updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(extendedPosition));
                currentState->setVal(updatedPosition);
                printf("AwningView::AwningController::loop retract, current position= %d\n", updatedPosition);
                retractAwning(targetExtendedPosition);
                // delay(100);
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
            currentExtendedPosition = targetExtendedPosition;

            updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(currentExtendedPosition));
            printf("AwningView::AwningController::loop stopping currentExtendedPosition = %d\n", updatedPosition);
            currentState->setVal(updatedPosition);
            view->updateTheView();
        } else {
            if (currentExtendedPosition != targetExtendedPosition)
                printf("AwningView::AwningController::loop currentExtendedPosition = %d\n", currentExtendedPosition);
            currentExtendedPosition = targetExtendedPosition;

            updatedPosition = static_cast<uint16_t>(static_cast<int16_t>(currentExtendedPosition));
            currentState->setVal(updatedPosition);
        }
        // currentState->setVal(targetExtendedPosition);
    }
} 

boolean AwningView::AwningController::isAwningExtended(void) { 
    // return targetState->getNewVal() == AwningView::FULLY_OPEN_SHADE; 
    boolean isExtended = (currentExtendedPosition > 0.0F);
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

    printf("AwningView::updateView completed \n"); 
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

    if (vw != nullptr)
        printf("AwningView::createAwningView: tmp created successfully\n");
    else
        printf("AwningView::createAwningView: tmp creation failed\n");   
    printf("AwningView::createAwningView completed\n");
}
#endif
