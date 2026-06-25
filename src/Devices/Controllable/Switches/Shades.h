#ifdef HOME_KIT
#ifndef SHADES_H // once I'm ready to define this, move this below ifndef
#define SHADES_H

/*********************************************************************************
 *  MIT License
 *                                                                                 
 *  *  
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
#include "GenericDevice.h"
#include "DGN.h"
#include "Packet.h"
#include "RVConstants.h"
#include "ShadesDefinitions.h"

class Shades : public GenericDevice {
    private:
        friend class ShadesView;

        uint8_t* commandData;

        uint8_t* getCommandData(void) const { return commandData; }

        SHADE_GROUP_BITMAP shadeGroup(void) const {
            SHADE_GROUP_BITMAP result = SHADE_GROUP_NONE;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (SHADE_GROUP_BITMAP)(data[SHADE_GROUP_INDEX]);
            }
            return result;
        }

        uint8_t motorDuty(void) const {
            // WINDOW_SHADE_CONTROL_STATUS only
            uint8_t result = 0;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t tmp = data[SHADE_OP_STATUS_INDEX];
                result = (tmp > MAX_RVC_PERCENT) ? MAX_RVC_PERCENT : tmp;
                result = result * RVC_PERCENT_PRECISION;
            }
            return result;
        }

        boolean isShadeLocked(void) const {
            // WINDOW_SHADE_CONTROL_STATUS only
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t tmp = data[SHADE_STATUS_INDEX];
                result = (tmp & SHADE_LOCK_MASK) == SHADE_LOCK_UNLOCKED;
            }
            return result;
        }
        
        boolean isMotorOutputOn(void) const { // ie is the motor running in either direction?
            // WINDOW_SHADE_CONTROL_STATUS only
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t tmp = data[SHADE_STATUS_INDEX];
                result = (tmp & SHADE_MOTOR_MASK) == SHADE_MOTOR_EITHER;
            }
            return result;
        }

        boolean isMotorInForward(void) const {
            // WINDOW_SHADE_CONTROL_STATUS only
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t tmp = data[SHADE_STATUS_INDEX];
                result = (tmp & SHADE_FORWARD_MASK) == SHADE_FORWARD_ON;
            }
            return result;
        }

        boolean isMOtorInReverse(void) const {
            // WINDOW_SHADE_CONTROL_STATUS only
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t tmp = data[SHADE_STATUS_INDEX];
                result = (tmp & SHADE_REVERSE_MASK) == SHADE_REVERSE_ON;
            }
            return result;
        }   

        uint8_t numberSecondsRemaining(void) const { // duration in seconds
            // WINDOW_SHADE_CONTROL_STATUS only
            /*
            Number of seconds remaining in Duration command. Max = 240 seconds
            0 = delay/duration expired
            1 – 239 = seconds remaining
            240 = 240 or more seconds remaining
            255 = no delay/duration active
            */
            uint8_t result = 0;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = data[SHADE_DURATION_INDEX];
                if ((result >= 240) && (result != 255)) // clamp the 240+ to 240 for a more useful value
                    result = 240;
                else if (result == 255)
                    result = 0; // no duration active
            }
            return result;
        }   

        const SHADES_COMMANDS lastCommand(void) const {
            SHADES_COMMANDS result = SHADE_COMMAND_STOP;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (SHADES_COMMANDS) (data[SHADE_COMMAND_INDEX]);               
            }
            return result;
        }
        const boolean isInOverCurrent(void) const {
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (data[SHADE_OVER_INDEX]) == IN_OVER_CURRENT;
            }
            return result;
        }
        const boolean isInOverride(void) const {
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (data[SHADE_OVER_INDEX]) == OVER_RIDE_ACTIVE;
            }
            return result;
        }

        const boolean isDisable1Active(void) const {
            /*
            When disable 1 is active, it has been set through an external
            signal input.
            */
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (data[SHADE_OVER_INDEX]) == DISABLE_1_ENABLED;
            }
            return result;
        }
        
        const boolean isDisable2Active(void) const {
            /*
            When disable 2 is active, it has been set through an external
            signal input.
            */
            boolean result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                result = (data[SHADE_OVER_INDEX]) == DISABLE_2_ENABLED;
            }
            return result;
        }   

        // WINDOW_SHADE_CONTROL_COMMAND methods
        const uint8_t cmdMotorDuty(void) const {
            uint8_t result = 0;
            uint8_t* data = getCommandData();
            if (data != nullptr) {
                uint8_t tmp = data[SHADE_MOTOR_DUTY_INDEX];
                result = (tmp > MAX_RVC_PERCENT) ? MAX_RVC_PERCENT : tmp;
                result = result * RVC_PERCENT_PRECISION;
            }
            return result;
        }

        void setMotorDuty(uint8_t val) {
            uint8_t* data = getCommandData();
            if (data != nullptr) {
                if (val > MAX_RVC_PERCENT)
                    val = MAX_RVC_PERCENT;
                data[SHADE_MOTOR_DUTY_INDEX] = (uint8_t) (val / RVC_PERCENT_PRECISION);
            }
        }
    

        const SHADES_COMMANDS shadeCommand(void) const {
            SHADES_COMMANDS result = SHADE_COMMAND_STOP;
            uint8_t* data = getCommandData();
            if (data != nullptr) {
                result = (SHADES_COMMANDS) (data[SHADE_COMMAND_INDEX]);               
            }
            return result;
        }

        void setShadeCommand(SHADES_COMMANDS cmd) {
            uint8_t* data = getCommandData();
            if (data != nullptr) {
                data[SHADE_COMMAND_INDEX] = (uint8_t) cmd;
            }
        }

        const uint8_t duration(void) const {
            uint8_t result = 0;
            uint8_t* data = getCommandData();
            if (data != nullptr) {
                result = data[SHADE_DURATION_INDEX];
                if ((result >= 240) && (result != 255)) // clamp the 240+ to 240 for a more useful value
                    result = 240;
                else if (result == 255)
                    result = 0; // no duration active
            }
            return result;
        }   

        const boolean isInInterlock(void) const {
            boolean result = false;
            uint8_t* data = getCommandData();
            if (data != nullptr) {
                uint8_t tmp = (data[SHADE_INTERLOCK_INDEX]);
                if ((tmp == 0x01) || (tmp == 0x02))
                    result = true;
            }
            return result;
        }
        
    protected:

        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                uint8_t* cData = getCommandData();
                if ((rawData != nullptr) && (cData != nullptr))// set the current data to the new data
                switch (dgn) {
                    case WINDOW_SHADE_CONTROL_COMMAND:
                        for (uint8_t i = 1; i < 8; i++) {
                            cData[i] = data[i]; // copy the command data
                        }
                        break;
                    case WINDOW_SHADE_CONTROL_STATUS:
                        for (uint8_t i = 1; i < 8; i++) {
                            rawData[i] = data[i]; // copy the status data
                        }
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }

    public:
        Shades() : GenericDevice() { 
            // Constructor implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
        }

        Shades(const Shades& orig) : GenericDevice(orig) { 
            // Copy constructor implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            for (uint8_t i = 0; i < 8; i++) {
                commandData[i] = orig.commandData[i]; // initialize to invalid data
            }
        }

        Shades(uint8_t address, uint8_t instance) : GenericDevice(address, instance) {     
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
        }

        Shades(uint8_t* data) : GenericDevice(data) {
            // Constructor with parameters implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
        }

        virtual ~Shades() {
            // Destructor implementation
        } 

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS);
};




#endif // SHADES_H
#endif // HOME_KIT