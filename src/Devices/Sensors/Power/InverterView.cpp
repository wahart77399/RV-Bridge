#include "InverterView.h"
#include "PowerSensorView.h"

InverterView::InverterView(Inverter* model, const char* name, const uint8_t numLegs, bool io,
                    bool showCurrent, bool showFault)
    : PowerSensorView(model, name, numLegs, io, showCurrent, showFault)
{
    // Re-use the shared voltage/current presentation
    // (or duplicate the small accessory block if you prefer no extra include)
    /**
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(name);
        // voltage + current characteristics identical to PowerSensorView

    */
}

/**
bool InverterView::updateView()
{
    bool result = false;
    if (model_ != nullptr) {
        // push model_->rmsVoltage() / rmsCurrent() into characteristics
        result = true;
    }
    return result;
}
    */
