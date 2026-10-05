// InverterView.h / .cpp (thin)
#pragma once
#include "RVConstants.h"
#include "SpanView.h"
#include "Inverter.h"
#include "PowerSensorView.h"

class InverterView : public PowerSensorView {
public:
    InverterView() = delete;
    InverterView(const InverterView&) = delete;
    InverterView& operator=(const InverterView&) = delete;
    InverterView(InverterView&&) = delete;
    InverterView& operator=(InverterView&&) = delete;
    ~InverterView() = default;

    InverterView(Inverter* model, const char* name, const uint8_t numLegs = DEFAULT_NUM_LEGS, bool io = false,
                    bool showCurrent = true, bool showFault = false );
    bool updateView() override;
private:
    Inverter* model_;
    SpanCharacteristic* temperatureChar_[INVERTER_TEMPERATURE_COUNT];
    SpanCharacteristic* dcVoltageChar_;
    SpanCharacteristic* dcCurrentChar_;
};