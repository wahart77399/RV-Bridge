// Inverter.h
#pragma once
#include "RVConstants.h"
#ifdef HOME_KIT_2

#include "PowerSensor.h"

enum class InverterStatus : uint8_t {
    Disabled       = 0x00,
    Invert         = 0x01,
    PassThru       = 0x02,
    ApsOnly        = 0x03,
    LoadSense      = 0x04,
    WaitingInvert  = 0x05,
    GenSupport     = 0x06
};

const uint8_t INVERTER_INVALID = 0xff;
const uint8_t INVERTER_STATUS_INDEX = 1;
const uint8_t INVERTER_LINE_INDEX = 0;       // line 1 or 2
const uint8_t INVERTER_IO_INDEX = 0;         // input or output
const uint8_t INVERTER_INSTANCE_MASK = 0x0f; // bits 0-3 0000 1111
const uint8_t INVERTER_LINE_MASK =  0x30;    // 0011 0000
const uint8_t INVERTER_IO_MASK =    0xc0;    // 1100 0000
const uint8_t INVERTER_LINE_1_VALUE = 0x00;  // 0000 0000
const uint8_t INVERTER_LINE_2_VALUE = 0x10;  // 0001 0000
const uint8_t INVERTER_INPUT_VALUE = 0x00;   // 0000 0000
const uint8_t INVERTER_OUTPUT_VALUE = 0x40;  // 0100 0000

class InverterView;

class Inverter : public PowerSensor {
    friend class InverterView;

private:
    InverterStatus status() const;

protected:
    uint8_t lineOf(RVC_DGN dgn, const uint8_t* raw) const override;

public:
    Inverter() = delete;
    Inverter(const Inverter&) = delete;
    Inverter& operator=(const Inverter&) = delete;
    Inverter(Inverter&&) = delete;
    Inverter& operator=(Inverter&&) = delete;
    ~Inverter() = default;

    Inverter(uint8_t address, uint8_t instance);

    void attachView(const char* name, bool showCurrent = true, bool showFault = true) override;

    boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer,
                           uint8_t val = SOURCE_ADDRESS);
};
#endif
