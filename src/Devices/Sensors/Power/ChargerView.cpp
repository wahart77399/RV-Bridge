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
    SpanView::createAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name((String(name) + " Charge V").c_str());
        new Service::TemperatureSensor();
                measuredVoltChar_ = new Characteristic::CurrentTemperature(tempCfromTempF(0.0F));
                measuredVoltChar_->setRange(tempCfromTempF(-20.0F), tempCfromTempF(100.0F));

    // Measured DC charge current
    SpanView::createAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name((String(name) + " Charge A").c_str());
        new Service::TemperatureSensor();
                measuredCurrChar_ = new Characteristic::CurrentTemperature(tempCfromTempF(0.0F));
                measuredCurrChar_->setRange(tempCfromTempF(-50.0F), tempCfromTempF(200.0F));

    // Operating state 0..7 (see ChargerOperatingState)
    SpanView::createAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name((String(name) + " State").c_str());
        new Service::TemperatureSensor();
                stateChar_ = new Characteristic::CurrentTemperature(tempCfromTempF(0.0F));
                stateChar_->setRange(tempCfromTempF(0.0F), tempCfromTempF(10.0F));
}

bool ChargerView::updateView() {
    bool result = false;

    // AC input tiles (base)
    bool acUpdated = PowerSensorView::updateView();

    if (model_ != nullptr && isNeedToUpdateView()) {
        float v = model_->measuredChargeVoltage();
        float a = model_->measuredChargeCurrent();
        float st = static_cast<float>(static_cast<uint8_t>(model_->operatingState()));

        if ((measuredVoltChar_ != nullptr) && (v >= -20.0F) && (v <= 100.0F)) {
            measuredVoltChar_->setVal(tempCfromTempF(v));
        }
        if ((measuredCurrChar_ != nullptr) && (a >= -50.0F) && (a <= 200.0F)) {
            measuredCurrChar_->setVal(tempCfromTempF(a));
        }
        if ((stateChar_ != nullptr) && (st >= 0.0F) && (st <= 10.0F)) {
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
