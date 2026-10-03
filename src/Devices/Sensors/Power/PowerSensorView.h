#pragma once
// #ifdef HOME_KIT_2
#include "SpanView.h"
#include "HomeSpan.h"
#include "PowerSensor.h"
#include "RVConstants.h"

constexpr uint8_t DEFAULT_NUM_LEGS = 2U;

class PowerSensorView : public SpanView {
public:
    PowerSensorView() = delete;
    PowerSensorView(const PowerSensorView&) = delete;
    PowerSensorView& operator=(const PowerSensorView&) = delete;
    PowerSensorView(PowerSensorView&&) = delete;
    PowerSensorView& operator=(PowerSensorView&&) = delete;
    ~PowerSensorView() = default;

    PowerSensorView(PowerSensor* model, const char* name, const uint8_t numLegs = DEFAULT_NUM_LEGS, bool io = false,
                    bool showCurrent = true, bool showFault = true);

    bool updateView() override;

private:
    PowerSensor*        model_;
    SpanCharacteristic* voltageChar_[DEFAULT_NUM_LEGS][NUMIO];
    SpanCharacteristic* currentChar_[DEFAULT_NUM_LEGS][NUMIO];
    SpanCharacteristic* faultChar_[DEFAULT_NUM_LEGS][NUMIO];
    uint8_t             numLegs_;
    boolean             needIO_;

    PowerSensor*        model(void) { return model_; }
    void                model(PowerSensor* m) { model_ = m; }
    SpanCharacteristic* voltageChar(uint8_t line=0, uint8_t io=0) const                               { return voltageChar_[line][io]; }
    void                voltageChar(SpanCharacteristic* c, uint8_t line=0, uint8_t io=OUTPUT_LINE)    { voltageChar_[line][io] = c; }
    SpanCharacteristic* currentChar(uint8_t line=0, uint8_t io=OUTPUT_LINE) const                      { return currentChar_[line][io]; }
    void                currentChar(SpanCharacteristic* c, uint8_t line=0, uint8_t io=OUTPUT_LINE)    { currentChar_[line][io] = c; }
    SpanCharacteristic* faultChar(uint8_t line=0, uint8_t io=OUTPUT_LINE)  const                      { return faultChar_[line][io]; }
    void                faultChar(SpanCharacteristic* c, uint8_t line=0, uint8_t io=OUTPUT_LINE)      { faultChar_[line][io] = c; }
    
    uint8_t             numLegs(void) const { return numLegs_; }
    void                numLegs(const uint8_t l) { numLegs_ = l; }
    boolean             needIO(void) const { return needIO_; }
    void                needIO(const boolean b) { needIO_ = b;}
};
