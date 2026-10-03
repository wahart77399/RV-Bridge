#include "AutomaticTransferSwitchView.h"
#ifdef HOME_KIT_2

AutomaticTransferSwitchView::AutomaticTransferSwitchView(AutomaticTransferSwitch* model, const char* name)
	: PowerSensorView(model, name, DEFAULT_NUM_LEGS, true /* io */)
//     : model_(model)
{
}


#endif


