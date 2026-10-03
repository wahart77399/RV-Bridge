#include "PowerSensorView.h"
#include "RVConstants.h"
#ifdef HOME_KIT_2

namespace {
    // readings are shown as temperatures; limits are given in the displayed (°F) units
    SpanCharacteristic* addReadingAccessory(const String& label, float lowerF, float upperF) {
        new SpanAccessory();
        new Service::AccessoryInformation();
        new Characteristic::Identify();
        new Characteristic::Name(label.c_str());
        new Service::TemperatureSensor();
        SpanCharacteristic* reading = new Characteristic::CurrentTemperature(tempCfromTempF(lowerF));
        reading->setRange(tempCfromTempF(lowerF), tempCfromTempF(upperF));
        new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit);
        return reading;
    }
}

PowerSensorView::PowerSensorView(PowerSensor* model, const char* name, const uint8_t legs, const boolean b,
                                 bool showCurrent, bool showFault)
    : model_(model)
    , voltageChar_{nullptr,nullptr}
    , currentChar_{nullptr, nullptr}
    , faultChar_{nullptr,nullptr}
    , numLegs_(legs)
    , needIO_(b)
{

    for (uint8_t i = 0; i < legs; i++) {
        const String base   = String(name);
        const String leg    = String(i);
        const String input  = needIO() ? " Input" : "";
        const String output = needIO() ? " Output" : "";

        voltageChar(addReadingAccessory(base + " Voltage " + leg + input, VAC_LOWER_LIMIT, VAC_UPPER_LIMIT), i, INPUT_LINE);
        if (showFault) {
            // StatusFault must live in a service; it is optional on TemperatureSensor
            faultChar(new Characteristic::StatusFault(false), i);
        }
        if (needIO()) {
            voltageChar(addReadingAccessory(base + " Voltage " + leg + output, VAC_LOWER_LIMIT, VAC_UPPER_LIMIT), i, OUTPUT_LINE);
        }

        if (showCurrent) {
            currentChar(addReadingAccessory(base + " Current " + leg, AAC_LOWER_LIMIT, AAC_UPPER_LIMIT), i, INPUT_LINE);
            if (needIO()) {
                currentChar(addReadingAccessory(base + " Current " + leg + output, AAC_LOWER_LIMIT, AAC_UPPER_LIMIT), i, OUTPUT_LINE);
            }
        }
    }
}

bool PowerSensorView::updateView()
{
    bool result = false;
    if (model_ != nullptr) {
        float v = 0.0f; // model_->rmsVoltage(); // / 10.0f;
        for (uint8_t i = 0; i < numLegs_; i++) {
            v = model()->rmsVoltage(i, INPUT_LINE);
            if (voltageChar(i, INPUT_LINE) != nullptr) {
                voltageChar(i, INPUT_LINE)->setVal(tempCfromTempF(v));
                if ((needIO()) && voltageChar(i, OUTPUT_LINE) != nullptr) {
                    v = model()->rmsVoltage(i, OUTPUT_LINE);
                    voltageChar(i, OUTPUT_LINE)->setVal(tempCfromTempF(v));
                }
            }
            if (currentChar(i, INPUT_LINE) != nullptr) {
                float a = model()->rmsCurrent(i, INPUT_LINE);
                currentChar(i, INPUT_LINE)->setVal(tempCfromTempF(a));
                if (needIO() && (currentChar(i, OUTPUT_LINE) != nullptr)) {
                    a = model()->rmsCurrent(i, OUTPUT_LINE);
                    currentChar(i, OUTPUT_LINE)->setVal(tempCfromTempF(a));
                }
            }
            if (faultChar(i) != nullptr) {
                bool faulted = model_->isOpenGroundFault() ||
                           model_->isOpenNeutralFault() ||
                           model_->isReversePolarityFault() ||
                           model_->isGroundCurrentFault();
                faultChar(i)->setVal(faulted ? 1 : 0);
            }
        }
        result = true;
    }
    return result;
}
#endif