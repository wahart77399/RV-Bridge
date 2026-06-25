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

typedef enum {
    INVERTER_DISABLED       = 0x00, 
    INVERTER_INVERT         = 0x01,
    INVERTER_PASS_THRU      = 0x02,
    INVERTER_ASP_ONLY       = 0x03,
    INVERTER_LOAD_SENSE     = 0x04,
    INVERTER_WAITING_INVERT = 0x05,
    INVERTER_GEN_SUPPORT    = 0x06
} INVERTER_STATUS_DEFINITION;

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
    
        uint8_t* inverterData; // data specific to the inverter
        // INVERTER_LINE_TYPE  line; // 1 or 2
        // INVERTER_IO_TYPE    io;   // 0 = AC in, 1 = AC out

        const INVERTER_LINE_TYPE getLine(void) const { 
            INVERTER_LINE_TYPE result = INVERTER_LINE_1_TYPE;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (INVERTER_LINE_TYPE)(data[INVERTER_LINE_INDEX] & INVERTER_LINE_MASK) == INVERTER_LINE_1_VALUE 
                                                ? INVERTER_LINE_1_TYPE : INVERTER_LINE_2_TYPE;
            }
            return result;
        }
        // void setLine(INVERTER_LINE_TYPE ln) { line = ln; }

        const INVERTER_IO_TYPE getIO(void) const {
            INVERTER_IO_TYPE result = INVERTER_INPUT;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (INVERTER_IO_TYPE)(data[INVERTER_IO_INDEX] & INVERTER_IO_MASK) == INVERTER_INPUT_VALUE 
                                                ? INVERTER_INPUT : INVERTER_OUTPUT;
            }
            return result;
        }
        // void setIO(INVERTER_IO_TYPE inputOutput) { io = inputOutput; }
        uint8_t* getInverterData(void) const { return inverterData; }

        const INVERTER_STATUS_DEFINITION getInverterStatus(void) const {
            INVERTER_STATUS_DEFINITION result = INVERTER_DISABLED;
            uint8_t* data = getInverterData();
            if (data != nullptr) {
                result = (INVERTER_STATUS_DEFINITION)(data[INVERTER_STATUS_INDEX]); 
            }
            return result;
        }   

        const boolean isBatteryTempEnabled(void) const {
            boolean result = false;
            uint8_t* data = getInverterData();
            if (data != nullptr) {
                result = (data[INVERTER_ENABLE_STATE_INDEX] & INVERTER_BATTERY_TEMP_MASK) == INVERTER_BATTERY_TEMP_ENABLED;
            }
            return result;
        }
        const boolean isLoadSenseEnabled(void) const {
            boolean result = false;
            uint8_t* data = getInverterData();
            if (data != nullptr) {
                result = (data[INVERTER_ENABLE_STATE_INDEX] & INVERTER_LOAD_SENSE_MASK) == INVERTER_LOAD_SENSE_ENABLED;
            }
            return result;
        }   
        const boolean isInverterEnabled(void) const {
            boolean result = false;
            uint8_t* data = getInverterData();
            if (data != nullptr) {
                result = (data[INVERTER_ENABLE_STATE_INDEX] & INVERTER_INVERTER_ENABLED_MASK) == INVERTER_INVERTER_ENABLED;
            }
            return result;
            }
        const boolean isPassThruEnabled(void) const {
            boolean result = false;
            uint8_t* data = getInverterData();
            if (data != nullptr) {
                result = (data[INVERTER_ENABLE_STATE_INDEX] & INVERTER_PASS_THRU_MASK) == INVERTER_PASS_THRU_ENABLED;
            }
            return result;
        }
        const boolean isGeneratorSupportEnabled(void) const {
            boolean result = false;
            uint8_t* data = getInverterData();
            if (data != nullptr) {
                result = (data[INVERTER_GEN_SUPPORT_INDEX]) == INVERTER_GEN_SUPPORT_ENABLED;
            }
            return result;
        }

    protected:
        
        // virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the ATS - we listen only
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                uint8_t* iData = getInverterData();
                if ((rawData != nullptr) && (iData != nullptr))// set the current data to the new data
                switch (dgn) {
                    case INVERTER_STATUS:
                        for (uint8_t i = 0; i < 8; i++) {
                            iData[i] = data[i]; // copy the inverter status data
                        }
                        break;
                    case INVERTER_AC_STATUS_1:
                        for (uint8_t i = 0; i < 8; i++) {
                            rawData[i] = data[i]; // copy the  AC status data
                        }
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }

    public:
        Inverter() : PowerSensor() {
            inverterData = new uint8_t[sizeof(uint8_t) * 8]; 
            inverterData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                inverterData[i] = INVALID_DATA; // initialize to invalid data
            }
            // Constructor implementation
        }

        Inverter(const Inverter& orig) : PowerSensor(orig) {
            // Copy constructor implementation
            inverterData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            for (uint8_t i = 0; i < 8; i++) {
                inverterData[i] = orig.inverterData[i]; // initialize to invalid data
            }
        }

        Inverter(uint8_t address, uint8_t indx) : PowerSensor(address, indx) {
            // Constructor with parameters implementation
            inverterData = new uint8_t[sizeof(uint8_t) * 8]; 
            inverterData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                inverterData[i] = INVALID_DATA; // initialize to invalid data
            }
        }

        Inverter(uint8_t* data) : PowerSensor(data) {
            // Constructor with parameters implementation
            inverterData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            inverterData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                inverterData[i] = INVALID_DATA; // initialize to invalid data
            }
        }

        virtual ~Inverter() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS); // this is only to deal with Inverter dgns
};
#endif  // INVERTER_H
#endif // ifdef HOME_KIT_2