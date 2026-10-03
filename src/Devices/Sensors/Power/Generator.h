#pragma once
#include "RVConstants.h"
#ifdef HOME_KIT_2

#include "PowerSensor.h"


enum class GeneratorInstance : uint8_t { // OUTPUT INSTANCES see section 6.18.2
    GENERATOR_INSTANCE_0_INVALID  = 0x00,
    GENERATOR_INSTANCE_1          = 0x01,  // 0000 0001
    GENERATOR_INSTANCE_2          = 0x02,  // 0000 0010
    GENERATOR_INSTANCE_3          = 0x03,  // 0000 0011
    GENERATOR_INSTANCE_4          = 0x04,  // 0000 0100
    GENERATOR_INSTANCE_5          = 0x05,  // 0000 0101
    GENERATOR_INSTANCE_6          = 0x06,  // 0000 0110
    GENERATOR_INSTANCE_7          = 0x07,  // 0000 0111
    GENERATOR_INSTANCE_8          = 0x08,  // 0000 1000
    GENERATOR_INSTANCE_9          = 0x09,  // 0000 1001
    GENERATOR_INSTANCE_10         = 0x0a,  // 0000 1010
    GENERATOR_INSTANCE_11_INVALID = 0x0b,  // 0000 1011
    GENERATOR_INSTANCE_12_INVALID = 0x0c,  // 0000 1100
    GENERATOR_INSTANCE_13_INVALID = 0x0d,  // 0000 1101
    GENERATOR_INSTANCE_14_INVALID = 0x0e,  // 0000 1011
    GENERATOR_INSTANCE_15_INVALID = 0x0f,  // 0000 1111

    GENERATOR_LINE_1              = 0x10,  // 0001 0000
    GENERATOR_LINE_2              = 0x20   // 0010 0000
};

enum class GeneratorLine : uint8_t {
    Unknown, Line1, Line2
};


class GeneratorView;

class Generator : public PowerSensor {
public:
    static const uint8_t GENERATOR_BYTE_0             = 0;
    static const uint8_t GENERATOR_OUTPUT_INDEX_MASK  = 0x0f;  // 0000 1111
    static const uint8_t GENERATOR_LINE_MASK          = 0xf0;  // 1111 0000
    static const uint8_t GENERATOR_LINE_INVALID       = 0xff;
    static const uint8_t GENERATOR_LINE_1_OUTPUT      = 1;
    static const uint8_t GENERATOR_LINE_2_OUTPUT      = 2;
    static const uint8_t GENERATOR_NO_IO_INFO         = 0x00;  // 0000 0000
    friend class GeneratorView;

private:

protected:
    uint8_t lineOf(RVC_DGN dgn, const uint8_t* raw) const override;

public:

    Generator() = delete;
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&&) = delete;
    Generator& operator=(Generator&&) = delete;
    ~Generator()  = default;

    Generator(uint8_t address, uint8_t instance);

    void attachView(const char* name, bool showCurrent = true, bool showFault = true) override;

    boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer,
                           uint8_t val = SOURCE_ADDRESS) override;
};
#endif
