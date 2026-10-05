#include "GeneratorView.h"
GeneratorView::GeneratorView(Generator* model, const char* name)
    : PowerSensorView(model, name)
    , model_(model)
    , engineFaultChar_(nullptr)
    , generatorRunningChar_(nullptr)
    , externalActivityChar_(nullptr)
{
    new Service::TemperatureSensor();
    new Characteristic::Name("Generator Engine Status");
    engineFaultChar_ = new Characteristic::StatusFault(false);

    new Service::ContactSensor();
    new Characteristic::ConfiguredName("Generator Engine Running");
    generatorRunningChar_ = new Characteristic::ContactSensorState();

    new Service::ContactSensor();
    new Characteristic::ConfiguredName("Generator Auto Start Inhibit");
    externalActivityChar_ = new Characteristic::ContactSensorState();
}

bool GeneratorView::updateView()
{
    bool updated = PowerSensorView::updateView();
    if (model_ != nullptr && (model_->hasStatus1() || model_->status2Received_) && engineFaultChar_ != nullptr) {
        engineFaultChar_->setVal((model_->status1Fault() || model_->hasEngineFault()) ? 1 : 0);
        updated = true;
    }
    if (model_ != nullptr && model_->hasStatus1() && generatorRunningChar_ != nullptr) {
        generatorRunningChar_->setVal(model_->isRunning()
            ? Characteristic::ContactSensorState::DETECTED
            : Characteristic::ContactSensorState::NOT_DETECTED);
        updated = true;
    }
    if (model_ != nullptr && model_->hasExternalActivityStatus() && externalActivityChar_ != nullptr) {
        externalActivityChar_->setVal(model_->externalActivityDetected()
            ? Characteristic::ContactSensorState::DETECTED
            : Characteristic::ContactSensorState::NOT_DETECTED);
        updated = true;
    }
    return updated;
}
