#include "RVConstants.h"
// #ifdef CHARGER_H
#ifdef HOME_KIT_2
#include "ChargerView.h"

ChargerView::ChargerView(Charger* model, const char* name)
    : PowerSensorView(model, name, /*showCurrent=*/true, /*showFault=*/true)
    , model_(model)
    , measuredVoltChar_(nullptr)
    , measuredCurrChar_(nullptr)
    , stateChar_(nullptr)
{
    buildChargeAccessories(name);
}

void ChargerView::buildChargeAccessories(const char* name) {
    // Measured DC charge voltage (from CHARGER_STATUS_2)
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name((String(name) + " Charge V").c_str());
        new Service::TemperatureSensor();
            measuredVoltChar_ = new Characteristic::CurrentTemperature(0.0);
            measuredVoltChar_->setRange(-20, 100);
            new Characteristic::TemperatureDisplayUnits(1);

    // Measured DC charge current
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name((String(name) + " Charge A").c_str());
        new Service::TemperatureSensor();
            measuredCurrChar_ = new Characteristic::CurrentTemperature(0.0);
            measuredCurrChar_->setRange(-50, 200);
            new Characteristic::TemperatureDisplayUnits(1);

    // Operating state 0..7 (see ChargerOperatingState)
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name((String(name) + " State").c_str());
        new Service::TemperatureSensor();
            stateChar_ = new Characteristic::CurrentTemperature(0.0);
            stateChar_->setRange(0, 10);
            new Characteristic::TemperatureDisplayUnits(1);
}

bool ChargerView::updateView() {
    bool result = false;

    // AC input tiles (base)
    bool acUpdated = PowerSensorView::updateView();

    if (model_ != nullptr && isNeedToUpdateView()) {
        float v = model_->measuredChargeVoltage();
        float a = model_->measuredChargeCurrent();
        float st = static_cast<float>(static_cast<uint8_t>(model_->operatingState()));

        if (measuredVoltChar_ != nullptr) {
            measuredVoltChar_->setVal(tempCfromTempF(v));
        }
        if (measuredCurrChar_ != nullptr) {
            measuredCurrChar_->setVal(tempCfromTempF(a));
        }
        if (stateChar_ != nullptr) {
            // state is dimensionless 0..7; still pushed through same path for display
            stateChar_->setVal(tempCfromTempF(st));
        }

        dontUpdateTheView();
        result = true;
    }

    if (acUpdated) {
        result = true;
    }
    return result;
}

#endif
// #endif
