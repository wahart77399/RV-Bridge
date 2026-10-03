#pragma once
#include "PowerSensorView.h"
#include "AutomaticTransferSwitch.h"

class AutomaticTransferSwitchView : public PowerSensorView {
public:
    AutomaticTransferSwitchView() = delete;
    AutomaticTransferSwitchView(const AutomaticTransferSwitchView&) = delete;
    AutomaticTransferSwitchView& operator=(const AutomaticTransferSwitchView&) = delete;
    AutomaticTransferSwitchView(AutomaticTransferSwitchView&&) = delete;
    AutomaticTransferSwitchView& operator=(AutomaticTransferSwitchView&&) = delete;
    ~AutomaticTransferSwitchView() = default;

    AutomaticTransferSwitchView(AutomaticTransferSwitch* model, const char* name);
    // bool updateView() override;

private:
    // AutomaticTransferSwitch* model_;
};
