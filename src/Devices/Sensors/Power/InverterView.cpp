#include "InverterView.h"
#include "PowerSensorView.h"

InverterView::InverterView(Inverter* model, const char* name, const uint8_t numLegs, bool io,
                    bool showCurrent, bool showFault)
    : PowerSensorView(model, name, numLegs, io, showCurrent, showFault)
    , model_(model)
    , temperatureChar_{}
    , dcVoltageChar_(nullptr)
    , dcCurrentChar_(nullptr)
{
    const char* temperatureNames[INVERTER_TEMPERATURE_COUNT] = {
        "FET 1 Temperature",
        "Transformer Temperature",
        "FET 2 Temperature",
        "Control/Power Board Temperature",
        "Capacitor Temperature",
        "Ambient Temperature"
    };

    for (uint8_t sensorIndex = 0; sensorIndex < INVERTER_TEMPERATURE_COUNT; ++sensorIndex) {
        new Service::TemperatureSensor();
        new Characteristic::Name(temperatureNames[sensorIndex]);
        temperatureChar_[sensorIndex] = new Characteristic::CurrentTemperature(0);
        temperatureChar_[sensorIndex]->setRange(-40, 100, 0.1);
    }

    new Service::TemperatureSensor();
    new Characteristic::Name("DC Voltage (V)");
    dcVoltageChar_ = new Characteristic::CurrentTemperature(tempCfromTempF(0.0f));
    dcVoltageChar_->setRange(tempCfromTempF(0.0f), tempCfromTempF(100.0f));

    new Service::TemperatureSensor();
    new Characteristic::Name("DC Current (A)");
    dcCurrentChar_ = new Characteristic::CurrentTemperature(tempCfromTempF(0.0f));
    dcCurrentChar_->setRange(tempCfromTempF(-200.0f), tempCfromTempF(200.0f));
}

bool InverterView::updateView()
{
    bool result = PowerSensorView::updateView();
    if (model_ != nullptr) {
        for (uint8_t sensorIndex = 0; sensorIndex < INVERTER_TEMPERATURE_COUNT; ++sensorIndex) {
            if (model_->componentTemperatureAvailable_[sensorIndex] && temperatureChar_[sensorIndex] != nullptr) {
                temperatureChar_[sensorIndex]->setVal(model_->componentTemperatureC_[sensorIndex]);
                result = true;
            }
        }
        if (model_->dcVoltageAvailable_ && dcVoltageChar_ != nullptr &&
            model_->dcVoltageV_ >= 0.0f && model_->dcVoltageV_ <= 100.0f) {
            dcVoltageChar_->setVal(tempCfromTempF(model_->dcVoltageV_));
            result = true;
        }
        if (model_->dcCurrentAvailable_ && dcCurrentChar_ != nullptr &&
            model_->dcCurrentA_ >= -200.0f && model_->dcCurrentA_ <= 200.0f) {
            dcCurrentChar_->setVal(tempCfromTempF(model_->dcCurrentA_));
            result = true;
        }
    }
    return result;
}
