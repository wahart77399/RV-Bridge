#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef INVERTER_H
#define INVERTER_H // once I'm ready to define this, move this below ifndef

/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2023 Randy Ubillos
 *  
 *  https://github.com/rubillos/RV-Bridge
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 
////////////////////////////////////////////////////////////////
//                                                            //
//    RV-Bridge: A HomeKit to RV-C interface for the ESP32    //
//                                                            //
////////////////////////////////////////////////////////////////
#include "Arduino.h"
#include "PowerSensor.h"
#include <map>

enum class INVERTER_STATUS_DEFINITION : uint8_t {
    INVERTER_DISABLED       = 0x00, 
    INVERTER_INVERT         = 0x01,
    INVERTER_PASS_THRU      = 0x02,
    INVERTER_ASP_ONLY       = 0x03,
    INVERTER_LOAD_SENSE     = 0x04,
    INVERTER_WAITING_INVERT = 0x05,
    INVERTER_GEN_SUPPORT    = 0x06
};

/**
enum class INVERTER_IO_TYPE : uint8_t {
    INVERTER_INPUT  = 0x00, // AC in
    INVERTER_OUTPUT = 0x01  // AC out
};

enum class INVERTER_LINE_TYPE : uint8_t {
    INVERTER_LINE_1_TYPE = 0x00,
    INVERTER_LINE_2_TYPE = 0x01
}; */

// current data information
const uint8_t INVERTER_INVALID = 0xff;
const uint8_t INVERTER_LINE_INDEX = 0;       // line 1 or 2
const uint8_t INVERTER_IO_INDEX = 0;         // input or output
const uint8_t INVERTER_INSTANCE_MASK = 0x0f; // bits 0-3 0000 1111
const uint8_t INVERTER_LINE_MASK =  0x30;    // 0011 0000
const uint8_t INVERTER_IO_MASK =    0xc0;    // 1100 0000
const uint8_t INVERTER_LINE_1_VALUE = 0x00;  // 0000 0000
const uint8_t INVERTER_LINE_2_VALUE = 0x10;  // 0001 0000
const uint8_t INVERTER_INPUT_VALUE = 0x00;   // 0000 0000
const uint8_t INVERTER_OUTPUT_VALUE = 0x40;  // 0100 0000

        // inverter data information (ie inverterData)
const uint8_t INVERTER_STATUS_INDEX = 1;
const uint8_t INVERTER_ENABLE_STATE_INDEX = 2;
const uint8_t INVERTER_GEN_SUPPORT_INDEX = 3;
const uint8_t INVERTER_BATTERY_TEMP_MASK = 0x03;     // 0000 0011
const uint8_t INVERTER_LOAD_SENSE_MASK = 0x0c;       // 0000 1100
const uint8_t INVERTER_INVERTER_ENABLED_MASK = 0x30; // 0011 0000
const uint8_t INVERTER_PASS_THRU_MASK = 0xc0;        // 1100 0000
const uint8_t INVERTER_BATTERY_TEMP_ENABLED = 0x01;  // 0000 0001
const uint8_t INVERTER_LOAD_SENSE_ENABLED = 0x04;    // 0000 0100
const uint8_t INVERTER_INVERTER_ENABLED = 0x10;      // 0001 0000
const uint8_t INVERTER_PASS_THRU_ENABLED = 0x40;     // 0100 0000
const uint8_t INVERTER_GEN_SUPPORT_ENABLED = 0x01;   // 0000 0001

class Inverter : public PowerSensor {
    private:
        friend class InverterView;
    
        std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> inverterData; // data specific to the inverter
        // std::map<INVERTER_LINE_TYPE, uint8_t*> inverterData; // data specific to the inverter
        // INVERTER_LINE_TYPE  line; // 1 or 2
        // INVERTER_IO_TYPE    io;   // 0 = AC in, 1 = AC out

        const INVERTER_LINE_TYPE getLine(const uint8_t* data) const { 
            INVERTER_LINE_TYPE result = INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE;
            // uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = static_cast<INVERTER_LINE_TYPE>((data[INVERTER_LINE_INDEX] & INVERTER_LINE_MASK) == INVERTER_LINE_1_VALUE
                                                ? INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE : INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE);
            }
            return result;
        }
        // void setLine(INVERTER_LINE_TYPE ln) { line = ln; }

        const INVERTER_IO_TYPE getIO(const uint8_t* data) const {
            INVERTER_IO_TYPE result = INVERTER_IO_TYPE::INVERTER_INPUT;
            // uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = static_cast<INVERTER_IO_TYPE>((data[INVERTER_IO_INDEX] & INVERTER_IO_MASK) == INVERTER_INPUT_VALUE 
                                                ? INVERTER_IO_TYPE::INVERTER_INPUT : INVERTER_IO_TYPE::INVERTER_OUTPUT);
            }
            return result;
        }
        // void setIO(INVERTER_IO_TYPE inputOutput) { io = inputOutput; }
        const std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>>& getInverterData(void) const { return inverterData; }

        const INVERTER_STATUS_DEFINITION getInverterStatus(INVERTER_LINE_TYPE line, INVERTER_IO_TYPE io) const {
            INVERTER_STATUS_DEFINITION result = INVERTER_STATUS_DEFINITION::INVERTER_DISABLED;
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> data = getInverterData();
            if (!data.empty()) {
                result = static_cast<INVERTER_STATUS_DEFINITION>(data[line][io][INVERTER_STATUS_INDEX]);
            }
            return result;
        }   

        const boolean isBatteryTempEnabled(void) const {
            boolean result = false;
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> data = getInverterData();
            if (!data.empty()) {
                result = (data[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][INVERTER_ENABLE_STATE_INDEX] & INVERTER_BATTERY_TEMP_MASK) == INVERTER_BATTERY_TEMP_ENABLED;
            }
            return result;
        }
        
        const boolean isLoadSenseEnabled(void) const {
            boolean result = false;
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> data = getInverterData();
            if (!data.empty()) {
                result = (data[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][INVERTER_ENABLE_STATE_INDEX] & INVERTER_LOAD_SENSE_MASK) == INVERTER_LOAD_SENSE_ENABLED;
            }
            return result;
        }   

        const boolean isInverterEnabled(void) const {
            boolean result = false;
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> data = getInverterData();
            if (!data.empty()) {
                result = (data[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][INVERTER_ENABLE_STATE_INDEX] & INVERTER_INVERTER_ENABLED_MASK) == INVERTER_INVERTER_ENABLED;
            }
            return result;
        }

        const boolean isPassThruEnabled(void) const {
            boolean result = false;
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> data = getInverterData();
            if (!data.empty()) {
                result = (data[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][INVERTER_ENABLE_STATE_INDEX] & INVERTER_PASS_THRU_MASK) == INVERTER_PASS_THRU_ENABLED;
            }
            return result;
        }

        const boolean isGeneratorSupportEnabled(void) const {
            boolean result = false;
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> data = getInverterData();
            if (!data.empty()) {
                result = (data[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][INVERTER_GEN_SUPPORT_INDEX]) == INVERTER_GEN_SUPPORT_ENABLED;
            }
            return result;
        }

    protected:

            // power values
        virtual uint16_t rmsVoltage(uint8_t line, uint8_t io) { 
            const INVERTER_LINE_TYPE ln = static_cast<INVERTER_LINE_TYPE>(line);
            const INVERTER_IO_TYPE ioType = static_cast<INVERTER_IO_TYPE>(io);
            uint16_t result = 0;
            auto lineIt = inverterData.find(ln);
            if (lineIt != inverterData.end()) {
                auto ioIt = lineIt->second.find(ioType);
                if (ioIt != lineIt->second.end()) {
                    uint16_t value = getACPointValue(ioIt->second.data(), AC_POINT_RMS_VOLTAGE_MSB_INDEX, AC_POINT_RMS_VOLTAGE_LSB_INDEX);
                    if (value <= VAC_MAX) {
                        result = validateVolts(line, (value - VAC_OFFSET) * VAC_PRECISION);
                    }
                }
            }
            return result;
            /* 
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> iData = getInverterData();
            if (!iData.empty()) {
                uint16_t value = getACPointValue(iData[static_cast<INVERTER_LINE_TYPE>(line)][static_cast<INVERTER_IO_TYPE>(io)].data(), AC_POINT_RMS_VOLTAGE_MSB_INDEX, AC_POINT_RMS_VOLTAGE_LSB_INDEX);

                if (value <= VAC_MAX) {

                    result = validateVolts(line, (value - VAC_OFFSET) * VAC_PRECISION);
                } 
            }
            return result;
            */
        }

        virtual uint16_t rmsCurrent(uint8_t line, uint8_t io) { 
            uint16_t result = 0;
            const auto ln = static_cast<INVERTER_LINE_TYPE>(line);
            const auto ioType = static_cast<INVERTER_IO_TYPE>(io);  
            auto lineIt = inverterData.find(ln);
            if (lineIt != inverterData.end()) {
                auto ioIt = lineIt->second.find(ioType);
                if (ioIt != lineIt->second.end()) {
                    uint16_t value = getACPointValue(ioIt->second.data(), AC_POINT_RMS_CURRENT_MSB_INDEX, AC_POINT_RMS_CURRENT_LSB_INDEX);
                    float tmp = static_cast<float>((value - AAC_ZERO) * AAC_PRECISION);
                    result = validateAmps(line, tmp);
                }
            }
            return result;
            /*
            std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> iData = getInverterData();
            if (!iData.empty()) {
                uint16_t value = getACPointValue(iData[static_cast<INVERTER_LINE_TYPE>(line)][static_cast<INVERTER_IO_TYPE>(io)].data(), AC_POINT_RMS_CURRENT_MSB_INDEX, AC_POINT_RMS_CURRENT_LSB_INDEX);
                // printf("PowerSensor::rmsCurrent value %d\n", value);
                float tmp = static_cast<float>((value - AAC_ZERO) * AAC_PRECISION);
                result = validateAmps(line, tmp);
            }
            return result;
            */
        }
        
        // virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the ATS - we listen only
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                INVERTER_LINE_TYPE line = getLine(data);
                INVERTER_IO_TYPE io = getIO(data);
                std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, std::array<uint8_t, DATA_SIZE>>> iData = getInverterData();
                if ((rawData != nullptr) && (!iData.empty()))// set the current data to the new data
                switch (dgn) {
                    case INVERTER_STATUS:
                        for (uint8_t i = 0; i < 8; i++) {
                            rawData[i] = data[i]; // copy the inverter status data
                        }
                        break;
                    case INVERTER_AC_STATUS_1:
                        rawData[0] = data[0]; // copy the instance index
                        memcpy(iData[line][io].data(), data, DATA_SIZE); // copy the  AC status data
                        for (uint8_t i = 0; i < 8; i++) {
                            iData[line][io][i] = data[i]; // copy the  AC status data
                        }
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }

    public:
        Inverter() : PowerSensor(), inverterData() {
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT].data(), INVALID_DATA, DATA_SIZE);  // [0] = index(); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT].data(), INVALID_DATA, DATA_SIZE); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT].data(), INVALID_DATA, DATA_SIZE); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT].data(), INVALID_DATA, DATA_SIZE);
            /* 
            for (uint8_t i = 1; i < 8; i++) {
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i] = INVALID_DATA; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i] = INVALID_DATA; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i] = INVALID_DATA; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i] = INVALID_DATA; // initialize to invalid data
            } */
            // Constructor implementation
        }

        Inverter(const Inverter& orig) : PowerSensor(orig), inverterData(orig.inverterData) {
            // Copy constructor implementation
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            // inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT] = new uint8_t[DATA_SIZE]; // allocate 8 bytes for the data
            // memcpy(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT], orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT], DATA_SIZE);
            // memcpy(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT], orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT], DATA_SIZE);
            // memcpy(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT], orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT], DATA_SIZE);
            // memcpy(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT], orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT], DATA_SIZE);
            /*
            for (uint8_t i = 0; i < 8; i++) {
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i] = orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i]; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i] = orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i]; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i] = orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i]; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i] = orig.inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i]; // initialize to invalid data
            }
                */  
        }

        Inverter(uint8_t address, uint8_t indx) : PowerSensor(address, indx), inverterData() {
            // Constructor with parameters implementation
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT].data(), INVALID_DATA, DATA_SIZE);  // [0] = index(); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT].data(), INVALID_DATA, DATA_SIZE); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT].data(), INVALID_DATA, DATA_SIZE); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT].data(), INVALID_DATA, DATA_SIZE);
            /*
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT] = new uint8_t[DATA_SIZE];
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT] = new uint8_t[DATA_SIZE];
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT] = new uint8_t[DATA_SIZE];
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT] = new uint8_t[DATA_SIZE];
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][0] = index(); // set the first byte to the instance index
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][0] = index(); // set the first byte to the instance index
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][0] = index(); // set the first byte to the instance index
            inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i] = INVALID_DATA; // initialize to invalid data   
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i] = INVALID_DATA; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT][i] = INVALID_DATA; // initialize to invalid data
                inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT][i] = INVALID_DATA; // initialize to invalid data  
            }
                */
        }

        Inverter(uint8_t* data) : PowerSensor(data), inverterData() {
            // Constructor with parameters implementation
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT].data(), INVALID_DATA, DATA_SIZE);  // [0] = index(); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT].data(), INVALID_DATA, DATA_SIZE); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT].data(), INVALID_DATA, DATA_SIZE); // set the first byte to the instance index
            memset(inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT].data(), INVALID_DATA, DATA_SIZE);
            /**
            inverterData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            inverterData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                inverterData[i] = INVALID_DATA; // initialize to invalid data
            }
                */
        }

        virtual ~Inverter() {
            // Destructor implementation
            /*
            delete[] inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT];
            delete[] inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_1_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT];
            delete[] inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_INPUT];
            delete[] inverterData[INVERTER_LINE_TYPE::INVERTER_LINE_2_TYPE][INVERTER_IO_TYPE::INVERTER_OUTPUT];
            */
            // delete[] inverterData;
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS); // this is only to deal with Inverter dgns
};
#endif  // INVERTER_H
#endif // ifdef HOME_KIT_2