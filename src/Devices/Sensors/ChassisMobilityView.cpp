#include "ChassisMobility.h"
#include "ChassisMobilityView.h"
#include "debug.h"

ChassisMobilityView::ChassisMobilityView(GenericDevice* model, const char* spanDevName)
    : SpanView(model)
    , chassisMobilitySensor_(nullptr) {
    (void)spanDevName;
}

ChassisMobilityView::~ChassisMobilityView() {
    chassisMobilitySensor_ = nullptr;
}

void ChassisMobilityView::sensor(ChassisMobilityMotion* value) {
    chassisMobilitySensor_ = value;
}

ChassisMobilityView::ChassisMobilityMotion* ChassisMobilityView::sensor() const {
    return chassisMobilitySensor_;
}

bool ChassisMobilityView::updateView() {
    bool updated = false;
    do {
        ChassisMobility* model = static_cast<ChassisMobility*>(getModel());
        ChassisMobilityMotion* motionService = sensor();
        if (model == nullptr || motionService == nullptr) {
            break;
        }
        if (!model->hasValidStatus()) {
            break;
        }
        MOTION_STATE motion = model->isMoving() ? IN_MOTION : NOT_IN_MOTION;
        motionService->publish(motion);
        updated = true;
    } while (false);
    return updated;
}

void ChassisMobilityView::createChassisMobilityView(GenericDevice* model, const char* spanDevName) {
    do {
        if (model == nullptr || spanDevName == nullptr) {
            break;
        }
        SpanView::prepHomeSpan();
        SpanView::createAccessory();
        new Service::AccessoryInformation();
        new Characteristic::Identify();
        new Characteristic::Name(spanDevName);

        ChassisMobilityView* view = new ChassisMobilityView(model, spanDevName);
        ChassisMobilityMotion* motionService =
            new ChassisMobilityMotion(view, static_cast<ChassisMobility*>(model), spanDevName);
        view->sensor(motionService);
    } while (false);
}