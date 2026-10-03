#pragma once
// #ifdef CHARGER_H
#ifndef CHARGER_VIEW_H
#define CHARGER_VIEW_H // once I'm ready to define this, move this below ifndef

#include "RVConstants.h"


#include "PowerSensorView.h"
#include "Charger.h"
#include "HomeSpan.h"

class ChargerView : public PowerSensorView {
public:
    ChargerView() = delete;
    ChargerView(const ChargerView&) = delete;
    ChargerView& operator=(const ChargerView&) = delete;
    ChargerView(ChargerView&&) = delete;
    ChargerView& operator=(ChargerView&&) = delete;
    ~ChargerView() = default;

    ChargerView(Charger* model, const char* name);

    bool updateView() override;

private:
    Charger* model_;

    SpanCharacteristic* measuredVoltChar_;
    SpanCharacteristic* measuredCurrChar_;
    SpanCharacteristic* stateChar_;   // operating state as number 0..7

    void buildChargeAccessories(const char* name);
};
#endif
// #endif