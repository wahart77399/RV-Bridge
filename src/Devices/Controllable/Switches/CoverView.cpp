#include "RVConstants.h"
#include "elapsedMillis.h"
#include "CoverView.h"
#include "CoverDevice.h"
#include "ChassisMobility.h"
#include "debug.h"

bool CoverView::bridgeCreated_ = false;

CoverKind CoverView::kind() const {
    CoverDevice* mdl = static_cast<CoverDevice*>(const_cast<CoverView*>(this)->getModel());
    return mdl ? mdl->kind() : CoverKind::Awning;
}

void CoverView::createBridge() {
    if (!bridgeCreated_) {
        bridgeCreated_ = true;
    }
}

// ---------------------------------------------------------------------------
// CoverController
// ---------------------------------------------------------------------------
CoverView::CoverController::CoverController(CoverView* vw, GenericDevice* mdl,
                                            const char* name, float extTime, float retTime)
    : Service::WindowCovering()
    , model_(static_cast<CoverDevice*>(mdl))
    , view_(vw)
    , timeExtend_(extTime)
    , timeRetract_(retTime)
    , targetPos_(0.0f)
    , currentPos_(0.0f)
    , startPos_(0.0f)
    , positionState_(new Characteristic::PositionState(POSITION_STOPPED))
{
    if (vw && vw->kind() == CoverKind::Awning) {
        currentState_ = new Characteristic::CurrentPosition(AWNING_FULLY_RETRACTED_PCT);        
        currentState_->setRange(AWNING_FULLY_RETRACTED_PCT, AWNING_FULLY_EXTENDED_PCT, AWNING_STEP_PCT);
        targetState_  = new Characteristic::TargetPosition(AWNING_FULLY_RETRACTED_PCT);
        targetState_->setRange(AWNING_FULLY_RETRACTED_PCT, AWNING_FULLY_EXTENDED_PCT, AWNING_STEP_PCT);
        currentPos_ = AWNING_FULLY_RETRACTED_PCT;
        startPos_   = currentPos_;
        targetPos_  = currentPos_;
    } else {
        currentState_ = new Characteristic::CurrentPosition(SHADES_FULLY_OPEN_PCT);
        currentState_->setRange(SHADES_FULLY_OPEN_PCT, SHADES_FULLY_CLOSED_PCT, SHADES_STEP_PCT);
        targetState_  = new Characteristic::TargetPosition(SHADES_FULLY_OPEN_PCT);
        targetState_->setRange(SHADES_FULLY_OPEN_PCT, SHADES_FULLY_CLOSED_PCT, SHADES_STEP_PCT);
    }
    currentState_->setDescription("Cover Current Position");
    targetState_->setDescription("Cover Target Position");
    obstructionDetected_ = new Characteristic::ObstructionDetected();
}

void CoverView::CoverController::readyToExtend() {
    setExtendCmd(true); setRetractCmd(false); setStopCmd(false);
}
void CoverView::CoverController::readyToRetract() {
    setExtendCmd(false); setRetractCmd(true); setStopCmd(false);
}
void CoverView::CoverController::readyToStop() {
    setExtendCmd(false); setRetractCmd(false); setStopCmd(true);
}

/*
bool CoverView::CoverController::isReadyToStop() {
    if ((isMoving() && startTime_ >= travelTime_) || stopCmd_) {
        clearCommands();
        setStopCmd(true);
        stopMoving();
    }
    return stopCmd_;
}
    */

bool CoverView::CoverController::isReadyToStop() {
    const bool travelDone = isMoving() && (startTime_ >= travelTime_);
    if (travelDone || stopCmd_) {
        if (travelDone) {
            // shades have no position command, so a partial move must be stopped explicitly
            if (model_ && view_ && (view_->kind() == CoverKind::Shade) &&
                (targetPos_ > SHADES_FULLY_OPEN_PCT) && (targetPos_ < SHADES_FULLY_CLOSED_PCT)) {
                model_->stop();
            }
            currentPos_ = targetPos_;
            currentState_->setVal(static_cast<uint8_t>(targetPos_ + 0.5f));
        }
        clearCommands();
        setStopCmd(true);
        stopMoving();
    }
    return stopCmd_;
}

void CoverView::CoverController::startMoving() {
    startTime_ = 0;
    sinceKeepAlive_ = 0;
    startPos_ = currentPos_;
    moving_ = true;
    positionState_->setVal((targetPos_ > currentPos_) ? POSITION_INCREASING : POSITION_DECREASING);
}
void CoverView::CoverController::stopMoving() {
    startTime_ = 0;
    travelTime_ = 0;
    moving_ = false;
    clearCommands();
    positionState_->setVal(POSITION_STOPPED);
}

void CoverView::CoverController::extendCover(float amount) {
    if (!model_ || !view_ || !isReadyToExtend()) return;
    uint8_t amt = (view_->kind() == CoverKind::Awning)
        ? static_cast<uint8_t>(round(amount))
        : static_cast<uint8_t>(round(SHADES_MAX_PERCENT - amount));
    model_->extend(amt);
}

void CoverView::CoverController::retractCover(float amount) {
    if (!model_ || !view_ || !isReadyToRetract()) return;
    uint8_t amt = (view_->kind() == CoverKind::Awning)
        ? static_cast<uint8_t>(round(amount))
        : static_cast<uint8_t>(round(SHADES_MAX_PERCENT - amount));
    model_->retract(amt);
}

void CoverView::CoverController::stopCover() {
    if (model_ && isReadyToStop()) model_->stop();
}

/*
boolean CoverView::CoverController::update() {
    boolean updated = false;
    view_->dontUpdateTheView();
    printf("CoverController::update() called. currentPos_ = %.2f\n", currentPos_);
    currentPos_ = currentState_->getVal<float>();

    if (targetState_->updated() && model_ && ChassisMobility::isParked()) {
        float target = targetState_->getNewVal<float>();
        updated = true;

        if (target < currentPos_) {          // need to retract / open
            clearCommands();
            readyToRetract();
            targetPos_ = target;
            travelTime_ = static_cast<uint16_t>(round(
                (currentPos_ - target) * timeExtend_ / COVER_MAX_PERCENT));
        } else if (target > currentPos_) {   // need to extend / close
            clearCommands();
            readyToExtend();
            targetPos_ = target;
            travelTime_ = static_cast<uint16_t>(round(
                (target - currentPos_) * timeRetract_ / COVER_MAX_PERCENT));
        } else {
            clearCommands();
            readyToStop();
            travelTime_ = 0;
        }
    }
    view_->updateTheView();
    return updated;
}

void CoverView::CoverController::loop() {
    if (isReadyToStop() || !view_ || !ChassisMobility::isParked()) return;

    view_->dontUpdateTheView();

    if (isReadyToExtend()) {
        if (!isMoving()) {
            if (!isReadyToStop()) {
                extendCover(targetPos_);
                startMoving();
                delay(DELAY_TIME);
            }
        } else {
            float elapsed = static_cast<float>(startTime_);
            float pct = (travelTime_ > 0) ? (elapsed / travelTime_) : 1.0f;
            if (pct > 1.0f) pct = 1.0f;

            float newPos = currentPos_ + pct * (targetPos_ - currentPos_);
            currentState_->setVal(static_cast<uint8_t>(newPos));
            currentPos_ = newPos;
            extendCover(targetPos_);
            delay(DELAY_TIME);
        }
    } else if (isReadyToRetract()) {
        if (!isMoving()) {
            if (!isReadyToStop()) {
                retractCover(targetPos_);
                startMoving();
                delay(DELAY_TIME);
            }
        } else {
            float elapsed = static_cast<float>(startTime_);
            float pct = (travelTime_ > 0) ? (elapsed / travelTime_) : 1.0f;
            if (pct > 1.0f) pct = 1.0f;

            float newPos = currentPos_ + pct * (targetPos_ - currentPos_);
            currentState_->setVal(static_cast<uint8_t>(newPos));
            currentPos_ = newPos;
            retractCover(targetPos_);
            delay(DELAY_TIME);
        }
    } else if (isReadyToStop()) {
        stopMoving();
        clearCommands();
    }

    view_->updateTheView();
}
*/

void CoverView::CoverController::moveTo(float target) {
    const bool wasMoving = isMoving();
    const bool wasExtend = isReadyToExtend();
    // "out" is a lower HomeKit value for awnings, a higher one for shades
    const bool towardOut = (target > currentPos_); // (view_->kind() == CoverKind::Awning) ? (target < currentPos_) : (target > currentPos_);

    clearCommands();
    targetPos_ = target;
    if (target == currentPos_) {
        readyToStop();
        travelTime_ = 0;
    } else if (towardOut) {
        readyToExtend();
        travelTime_ = static_cast<uint16_t>(round(fabsf(target - currentPos_) * timeExtend_ / COVER_MAX_PERCENT));
    } else {
        readyToRetract();
        travelTime_ = static_cast<uint16_t>(round(fabsf(target - currentPos_) * timeRetract_ / COVER_MAX_PERCENT));
    }

    if (wasMoving) {
        if (stopCmd_) {
            model_->stop();
        } else if ((view_->kind() == CoverKind::Shade) && (isReadyToExtend() == wasExtend)) {
            // same direction: re-sending the toggle would stop the shade, so just re-plan
            startPos_  = currentPos_;
            startTime_ = 0;
        } else {
            if (view_->kind() == CoverKind::Shade) model_->stop();
            moving_ = false;
        }
    }
}

/*
boolean CoverView::CoverController::update() {
    boolean updated = false;
    view_->dontUpdateTheView();
    const bool wasMoving = isMoving();
    const bool wasExtend = isReadyToExtend();

    currentPos_ = currentState_->getVal<float>();

    if (targetState_->updated() && model_ && ChassisMobility::isParked()) {
        float target = targetState_->getNewVal<float>();
        updated = true;

	    printf("CoverView::CoverController::update target %f\n", target);
        if (target < currentPos_) {          // moving toward lower %  (shade open / awning extend)
            clearCommands();
            readyToRetract();                // “retract” in HomeKit terms
            targetPos_ = target;
            travelTime_ = static_cast<uint16_t>(round(
                (currentPos_ - target) * timeExtend_ / COVER_MAX_PERCENT));
        }
        else if (target > currentPos_) {     // moving toward higher % (shade close / awning retract)
            clearCommands();
            readyToExtend();                 // “extend” in HomeKit terms
            targetPos_ = target;
            travelTime_ = static_cast<uint16_t>(round(
                (target - currentPos_) * timeRetract_ / COVER_MAX_PERCENT));
        }
        else {
            clearCommands();
            readyToStop();
            travelTime_ = 0;
        }
        if (wasMoving) {
            if (stopCmd_) {
                model_->stop();
            } else if ((view_->kind() == CoverKind::Shade) && (isReadyToExtend() == wasExtend)) {
                // same direction: re-sending the toggle would stop the shade, so just re-plan
                startPos_  = currentPos_;
                startTime_ = 0;
            } else {
                if (view_->kind() == CoverKind::Shade) model_->stop();
                moving_ = false;
            }
        }
    }
    view_->updateTheView();
    return updated;
}
*/
boolean CoverView::CoverController::update() {
    boolean updated = false;
    view_->dontUpdateTheView();
    currentPos_ = currentState_->getVal<float>();

    if (targetState_->updated() && model_ && ChassisMobility::isParked()) {
        moveTo(targetState_->getNewVal<float>());
        updated = true;
    }
    view_->updateTheView();
    return updated;
}
void CoverView::CoverController::loop() {
    if (isReadyToStop() || !view_ || !ChassisMobility::isParked()) return;

    view_->dontUpdateTheView();

    // ---------------------------------------------------------------
    // EXTEND direction (HomeKit “out” / higher or lower depending on kind)
    // ---------------------------------------------------------------
    if (isReadyToExtend()) {
        if (!isMoving()) {
            if (!isReadyToStop()) {
                extendCover(targetPos_);
                startMoving();
                delay(DELAY_TIME);
            }
        } else {
            float elapsed = static_cast<float>(startTime_);
            float pct = (travelTime_ > 0) ? (elapsed / travelTime_) : 1.0f;
            if (pct > 1.0f) pct = 1.0f;

            // Smooth linear progress from currentPos_ → targetPos_
            float newPos = startPos_ + pct * (targetPos_ - startPos_);
	        // printf("CoverView::CoverController::Loop newpos %f\n", newPos);

            // Clamp to the legal range for the kind
            if (view_->kind() == CoverKind::Awning) {
                if (newPos < AWNING_FULLY_RETRACTED_PCT) newPos = AWNING_FULLY_RETRACTED_PCT;
                if (newPos > AWNING_FULLY_EXTENDED_PCT) newPos = AWNING_FULLY_EXTENDED_PCT;
            } else {
                if (newPos < SHADES_FULLY_OPEN_PCT) newPos = SHADES_FULLY_OPEN_PCT;
                if (newPos > SHADES_FULLY_CLOSED_PCT) newPos = SHADES_FULLY_CLOSED_PCT;
            }

            const uint8_t shown = static_cast<uint8_t>(newPos + 0.5f);
            if (currentState_->getVal<uint8_t>() != shown) {
                currentState_->setVal(shown);
            }
            currentPos_ = newPos;

            // Keep the hardware moving
            if ((view_->kind() == CoverKind::Awning) && (sinceKeepAlive_ >= KEEP_ALIVE_MS)) {
                extendCover(targetPos_);
                sinceKeepAlive_ = 0;
            }
        }
    }
    // ---------------------------------------------------------------
    // RETRACT direction
    // ---------------------------------------------------------------
    else if (isReadyToRetract()) {
        if (!isMoving()) {
            if (!isReadyToStop()) {
                retractCover(targetPos_);
                startMoving();
                delay(DELAY_TIME);
            }
        } else {
            float elapsed = static_cast<float>(startTime_);
            float pct = (travelTime_ > 0) ? (elapsed / travelTime_) : 1.0f;
            if (pct > 1.0f) pct = 1.0f;

            float newPos = startPos_ + pct * (targetPos_ - startPos_);
	        //printf("CoverView::CoverController::Loop newpos %f\n", newPos);

            if (view_->kind() == CoverKind::Awning) {
                if (newPos < AWNING_FULLY_RETRACTED_PCT) newPos = AWNING_FULLY_RETRACTED_PCT;
                if (newPos > AWNING_FULLY_EXTENDED_PCT) newPos = AWNING_FULLY_EXTENDED_PCT;
            } else {
                if (newPos < SHADES_FULLY_OPEN_PCT) newPos = SHADES_FULLY_OPEN_PCT;
                if (newPos > SHADES_FULLY_CLOSED_PCT) newPos = SHADES_FULLY_CLOSED_PCT;
            }

            const uint8_t shown = static_cast<uint8_t>(newPos + 0.5f);
            if (currentState_->getVal<uint8_t>() != shown) {
                currentState_->setVal(shown);
            }
            currentPos_ = newPos;

            if ((view_->kind() == CoverKind::Awning) && (sinceKeepAlive_ >= KEEP_ALIVE_MS)) {
                retractCover(targetPos_);
                sinceKeepAlive_ = 0;
            }
        }
    }
    else if (isReadyToStop()) {
        stopMoving();
        clearCommands();
    }

    view_->updateTheView();
}

bool CoverView::CoverController::isCoverExtended() const {
    return model_ ? model_->isExtended() : (currentPos_ > 0.0f);
}
bool CoverView::CoverController::isCoverRetracted() const {
    return currentPos_ >= (COVER_MAX_PERCENT - COVER_PCT_FUDGE);
}
bool CoverView::CoverController::isCoverFullyExtended() const {
    return currentPos_ <= COVER_PCT_FUDGE;
}
bool CoverView::CoverController::isCoverFullyRetracted() const {
    return isCoverRetracted();
}
bool CoverView::CoverController::isOut() const {
    if (!view_) return false;
    uint8_t st = currentState_->getVal<uint8_t>();
    return (view_->kind() == CoverKind::Awning)
        ? (st == AWNING_FULLY_EXTENDED_PCT)
        : (st == SHADES_FULLY_CLOSED_PCT);
}

// ---------------------------------------------------------------------------
// CoverExtendRetractController
// ---------------------------------------------------------------------------
CoverView::CoverExtendRetractController::CoverExtendRetractController(
    CoverView* vw, GenericDevice* mdl, CoverController* ctrl)
    : Service::Switch()
    , model_(static_cast<CoverDevice*>(mdl))
    , view_(vw)
    , coverCtrl_(ctrl)
{
    out_ = new Characteristic::On();
    out_->setDescription("Send Cover Out(on) or In(off)");
    out_->setVal(false);
}

boolean CoverView::CoverExtendRetractController::update() {
    boolean updated = false;
    if (!view_) return false;

    view_->dontUpdateTheView();

    if (out_->updated() && coverCtrl_ && ChassisMobility::isParked()) {
        bool outVal = out_->getNewVal<bool>();
        if (outVal) {
            coverCtrl_->requestFullExtend();
        } else {
            coverCtrl_->requestFullRetract();
        }
        updated = true;
    }

    view_->updateTheView();
    return updated;
}

void CoverView::CoverExtendRetractController::loop() {
    //if (coverCtrl_) coverCtrl_->loop();
}

// ---------------------------------------------------------------------------
// CoverView
// ---------------------------------------------------------------------------
CoverView::CoverView(GenericDevice* model, const char* spanDevName)
    : SpanView(model)
    , spanDeviceName_(spanDevName)
{}

CoverView::~CoverView() {
    // controllers are owned by HomeSpan accessory lifetime
}

bool CoverView::updateView() {
    bool updated = false;
    if (isNeedToUpdateView() && ChassisMobility::isParked()) {
        CoverDevice* mdl = static_cast<CoverDevice*>(getModel());
        if (mdl && extendRetractCtrl_ && controller_) {
            extendRetractCtrl_->setOut(controller_->isCoverExtended());
            updated = true;
        }
    }
    return updated;
}

void CoverView::createCoverView(GenericDevice* model, const char* spanDevName,
                                float extTime, float retTime) {
    SpanView::prepHomeSpan();
    createBridge();

    CoverView* vw = new CoverView(model, spanDevName);

    SpanView::createAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);

    auto* ctrl = new CoverController(vw, model, spanDevName, extTime, retTime);
    vw->setController(ctrl);

    auto* extCtrl = new CoverExtendRetractController(vw, model, ctrl);
    vw->setExtendRetractController(extCtrl);
}

void CoverView::CoverController::requestFullExtend() {
    /**
    clearCommands();
    readyToExtend();
    targetPos_ = (view_ && view_->kind() == CoverKind::Awning)
                     ? static_cast<float>(AWNING_FULLY_EXTENDED_PCT)
                     : static_cast<float>(SHADES_FULLY_CLOSED_PCT);
    travelTime_ = static_cast<uint16_t>(timeExtend_);
    */
    const float target = (view_->kind() == CoverKind::Awning)
                             ? static_cast<float>(AWNING_FULLY_EXTENDED_PCT)
                             : static_cast<float>(SHADES_FULLY_CLOSED_PCT);
    targetState_->setVal(static_cast<uint8_t>(target));
    moveTo(target);
}

void CoverView::CoverController::requestFullRetract() {
    /* *
    clearCommands();
    readyToRetract();
    targetPos_ = (view_ && view_->kind() == CoverKind::Awning)
                     ? static_cast<float>(AWNING_FULLY_RETRACTED_PCT)
                     : static_cast<float>(SHADES_FULLY_OPEN_PCT);
    travelTime_ = static_cast<uint16_t>(timeRetract_);
    */
    const float target = (view_->kind() == CoverKind::Awning)
                             ? static_cast<float>(AWNING_FULLY_RETRACTED_PCT)
                             : static_cast<float>(SHADES_FULLY_OPEN_PCT);
    targetState_->setVal(static_cast<uint8_t>(target));
    moveTo(target);
}
