#include "RVConstants.h"

#include "Generator.h"
#include "GeneratorView.h"
#include "Packet.h"
#include "DGN.h"

Generator::Generator(uint8_t address, uint8_t instance)
    : PowerSensor(address, instance)
{
}

uint8_t Generator::lineOf(RVC_DGN /*dgn*/, const uint8_t* raw) const
{
    uint8_t result = NO_LINE;
    if (raw != nullptr) {
        uint8_t masked = raw[GENERATOR_BYTE_0] & GENERATOR_LINE_MASK;
        if (masked == static_cast<uint8_t>(GeneratorInstance::GENERATOR_LINE_1)) {
            result = 0;
        } else if (masked == static_cast<uint8_t>(GeneratorInstance::GENERATOR_LINE_2)) {
            result = 1;
        }
    }
    return result;
}

void Generator::attachView(const char* name, bool showCurrent, bool showFault)
{
    GeneratorView* view = new GeneratorView(this, name);
    addView(view);
}

boolean Generator::executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t /*val*/)
{
    boolean handled = false;

    if (buffer != nullptr) {
        switch (dgn) {
            case GENERATOR_AC_STATUS_1:
                setData(dgn, const_cast<uint8_t*>(buffer));
                updateViews();
                handled = true;
                break;
            case GENERATOR_COMMAND:
            case GENERATOR_AC_STATUS_2:
            case GENERATOR_AC_STATUS_3:
            case GENERATOR_AC_STATUS_4:
            default:
                // intentionally not managed – same as original
                break;
        }
    }
    return handled;
}
