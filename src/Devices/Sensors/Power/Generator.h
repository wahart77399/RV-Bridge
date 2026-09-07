#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef GENERATOR_H
#define GENERATOR_H // once I'm ready to define this, move this below ifndef
#include "Arduino.h"
// #define CUSTOM_CHAR_HEADER
// #include "HomeSpan.h"
#include "PowerDefinition.h"
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


// GENERATOR is only (for now) a sensor, not a controller where we can start/stop it
// Initially, we are only going to monitor line output of generator, but we may add status of the generator in general at a later point
class Generator : public PowerSensor {
    public: 

        static const uint8_t GENERATOR_BYTE_0             = 0; 
        static const uint8_t GENERATOR_OUTPUT_INDEX_MASK  = 0x0f;  // 0000 1111
        static const uint8_t GENERATOR_LINE_MASK          = 0xf0;  // 1111 0000
        static const uint8_t GENERATOR_LINE_INVALID       = 0xff;
        static const uint8_t GENERATOR_LINE_1_OUTPUT      = 1;
        static const uint8_t GENERATOR_LINE_2_OUTPUT      = 2;

    private:
        uint8_t line1Data[DATA_SIZE]; // data specific to line 1 of the generator
        uint8_t line2Data[DATA_SIZE]; // data specific to line 2 of

        
        friend class GeneratorView;
        GeneratorLine line(void) const {
            GeneratorLine result = GeneratorLine::Unknown;
            uint8_t* rawData = getCurrentData();
            // printf("Generator::line: rawData[0]=0x%02X\n", rawData[GENERATOR_BYTE_0]);
            // printf("Generator::line: rawData[0] & GENERATOR_OUTPUT_INDEX_MASK=0x%02X\n", rawData[GENERATOR_BYTE_0] & GENERATOR_OUTPUT_INDEX_MASK);
            // printf("Generator::line: rawData[0] & GENERATOR_LINE_MASK=0x%02X\n", rawData[GENERATOR_BYTE_0] & GENERATOR_LINE_MASK);
            if (rawData != nullptr) {
                uint8_t theLine = rawData[GENERATOR_BYTE_0] & GENERATOR_LINE_MASK;
                // printf("Generator::line: theLine=0x%02X\n", theLine);
                switch (rawData[GENERATOR_BYTE_0] & GENERATOR_LINE_MASK) {
                    case static_cast<uint8_t>(GeneratorInstance::GENERATOR_LINE_1):
                        result = GeneratorLine::Line1;
                        break;
                    case static_cast<uint8_t>(GeneratorInstance::GENERATOR_LINE_2):
                        result = GeneratorLine::Line2;
                        break;
                    default: // invalid output instance
                        break;
                }
            }
            return result;
        }

    protected:
        void setData(RVC_DGN dgn, uint8_t* data);
        // virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the GENERATOR - we listen only
        uint16_t rmsVoltage(uint8_t line) override {
            uint16_t result = 0;
            uint8_t* rawData = (line == static_cast<uint8_t>(GeneratorLine::Line2)) ? line2Data : line1Data;
            if (rawData != nullptr) {
                uint16_t value = getACPointValue(rawData, AC_POINT_RMS_VOLTAGE_MSB_INDEX, AC_POINT_RMS_VOLTAGE_LSB_INDEX);
                if (value <= VAC_MAX)
                    result = validateVolts(line, (value - VAC_OFFSET) * VAC_PRECISION);
            }
            return result;
        }
        uint16_t rmsCurrent(uint8_t line) override {
            uint16_t result = 0;
            uint8_t* rawData = (line == static_cast<uint8_t>(GeneratorLine::Line2)) ? line2Data : line1Data;
            if (rawData != nullptr) {
                uint16_t value = getACPointValue(rawData, AC_POINT_RMS_CURRENT_MSB_INDEX, AC_POINT_RMS_CURRENT_LSB_INDEX);
                result = validateAmps(line, static_cast<float>((value - AAC_ZERO) * AAC_PRECISION));
            }
            return result;
        }

    public:

        Generator() : PowerSensor() {
            // Constructor implementation
            memset(line1Data, INVALID_DATA, DATA_SIZE);
            memset(line2Data, INVALID_DATA, DATA_SIZE);
        }

        Generator(const Generator& orig) : PowerSensor(orig) {
            // Copy constructor implementation
            memcpy(line1Data, orig.line1Data, DATA_SIZE);
            memcpy(line2Data, orig.line2Data, DATA_SIZE);
        }

        Generator(uint8_t address, uint8_t index) : PowerSensor(address, index) { 

            memset(line1Data, INVALID_DATA, DATA_SIZE);
            memset(line2Data, INVALID_DATA, DATA_SIZE);
        }

        Generator(uint8_t* data) : PowerSensor(data) {
            // Constructor with parameters implementation

            memset(line1Data, INVALID_DATA, DATA_SIZE);
            memset(line2Data, INVALID_DATA, DATA_SIZE);
        }

        virtual ~Generator() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS); // this is only to deal with Generator dgns

};

#endif  // GENERATOR_H
#endif // ifdef HOME_KIT_2