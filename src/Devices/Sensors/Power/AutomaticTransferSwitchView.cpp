#include "AutomaticTransferSwitchView.h"

AutomaticTransferSwitchView::AutomaticTransferSwitchView(AutomaticTransferSwitch* model, const char* name)
	: PowerSensorView(model, name, DEFAULT_NUM_LEGS, true /* io */)
//     : model_(model)
{
}
