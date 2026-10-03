#include "GeneratorView.h"
#ifdef HOME_KIT_2
GeneratorView::GeneratorView(Generator* model, const char* name)
    : PowerSensorView(model, name)
    // , model_(model)
{
    /**
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(name);
        // Add voltage/current TemperatureSensor characteristics
        // identical to PowerSensorView if you want them here
    */
}

/**
bool GeneratorView::updateView()
{
    return PowerSensor::updateView
}
    */

#endif
