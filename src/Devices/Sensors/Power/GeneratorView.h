#pragma once
#include "RVConstants.h"
#ifdef HOME_KIT_2
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
    // bool updateView() override;

private:
/**
    Generator* model_;

    Generator* model(void) const        { return model_; }
    void model(Generator* mdl)    { model_ = mdl; }
*/
};
#endif
