
#ifndef AUTOMATIC_TRANSFER_SWITCH_H
#define AUTOMATIC_TRANSFER_SWITCH_H // once I'm ready to define this, move this below ifndef

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
#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Arduino.h"
#include "PowerSensor.h"
#include "ATS_Definitions.h"

class AutomaticTransferSwitch : public PowerSensor {
    public:
        static const uint8_t ATS_BYTE_0 = 0; 
        static const uint8_t ATS_STATUS_INDEX_MASK  = 0x07;  // 0000 0111
        static const uint8_t ATS_STATUS_IOTYPE_MASK = 0x08;  // 0000 1000
        static const uint8_t ATS_STATUS_SOURCE_MASK = 0x70;  // 0111 0000
        static const uint8_t ATS_STATUS_LEG_MASK    = 0x80;  // 1000 0000
    private:
        friend class AutomaticTransferSwitchView;

        uint8_t getInstance(void) const {
            uint8_t* rawData = getCurrentData();
            uint8_t result = ATS_INSTANCE_0_INVALID; // default to invalid
            if (rawData != nullptr) {
                result = rawData[ATS_BYTE_0] & ATS_STATUS_INDEX_MASK; // get the instance index
            }
            return result;
        }

        ATS_IO_TYPE getIOType(void) const {
            uint8_t* rawData = getCurrentData();
            ATS_IO_TYPE result = ATS_INPUT_TYPE; // default to input
            if (rawData != nullptr) {
                result = (ATS_IO_TYPE) (rawData[ATS_BYTE_0] & ATS_STATUS_IOTYPE_MASK); // get the IO type
            }
            return result;
        }

        ATS_SOURCE_TYPE getSource(void) const {
            uint8_t* rawData = getCurrentData();
            ATS_SOURCE_TYPE result = ATS_SOURCE_PRIMARY_TYPE; // default to primary source 
            if (rawData != nullptr) {
                result = (ATS_SOURCE_TYPE) ((rawData[ATS_BYTE_0] & ATS_STATUS_SOURCE_MASK) >> 4); // get the source type
            }
            return result;
        }   

        ATS_LEG_TYPE getLeg(void) const {
            uint8_t* rawData = getCurrentData();
            ATS_LEG_TYPE result = ATS_LEG_1_TYPE; // default to leg 1
            if (rawData != nullptr) {
                result = (ATS_LEG_TYPE) ((rawData[ATS_BYTE_0] & ATS_STATUS_LEG_MASK) >> 7); // get the leg type
            }
            return result;
        }

    protected:
        
        // virtual CAN_frame_t* buildCommand(RVC_DGN dgn); // do nothing - no commands will be sent to the ATS - we listen only

    public:

        AutomaticTransferSwitch() : PowerSensor() {
            // Constructor implementation
        }

        AutomaticTransferSwitch(const AutomaticTransferSwitch& orig) : PowerSensor(orig) {
            // Copy constructor implementation
        }

        AutomaticTransferSwitch(uint8_t address, uint8_t index) : PowerSensor(address, index) { 
        }

        AutomaticTransferSwitch(uint8_t* data) : PowerSensor(data) {
            // Constructor with parameters implementation
        }

        virtual ~AutomaticTransferSwitch() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS); // this is only to deal with ATS dgns
};

#endif
#endif // AUTOMATIC_TRANSFER_SWITCH_H // HOW_KIT_2