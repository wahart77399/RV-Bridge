#pragma once
#include "RVConstants.h"
#include "SpanView.h"
#include "Generator.h"
#include "PowerSensorView.h"

class GeneratorView : public PowerSensorView {
public:
    GeneratorView() = delete;
    GeneratorView(const GeneratorView&) = delete;
    GeneratorView& operator=(const GeneratorView&) = delete;
    GeneratorView(GeneratorView&&) = delete;
    GeneratorView& operator=(GeneratorView&&) = delete;
    ~GeneratorView() = default;

    GeneratorView(Generator* model, const char* name);
    bool updateView() override;

private:
    Generator* model_;
    SpanCharacteristic* engineFaultChar_;
    SpanCharacteristic* generatorRunningChar_;
    SpanCharacteristic* externalActivityChar_;
};
