#pragma once
/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2023 Randy Ubillos
 *  
 *  https://github.com/rubillos/RV-Bridge
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
#include "Arduino.h"
#include "GenericDevice.h"
#include "Packet.h"
#include "LightDeviceCmd.h"
#include "debug.h"

#ifndef DC_DIMMER_COMMAND_BRIGHTNESS_INDEX
constexpr uint8_t DC_DIMMER_COMMAND_BRIGHTNESS_INDEX = 1; // index in the data array for the dimmer command
#endif
#ifndef DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX
constexpr uint8_t DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX = 2; // index in the data array for the dimmer command duration
#endif

enum class LightKind : uint8_t {OnOff, Dimmable};


class LightDevice : public GenericDevice {
        friend class LightDeviceView;
    protected:
        const uint8_t SWITCH_OFF = DIMMER_STATUS_3_SWITCH_OFF;
    private:
        const uint8_t SWITCH_ON = 0xc8;
        const uint8_t STATUS_SWITCH_OFF = 0x00;
        
        const uint8_t RVCBrightMax = 200; 
        const uint8_t RVC_CONTINUOUS_DURATION = 0xff; // from spec RV-C Specification on DC_DIMMER_COMMAND_2
        
        LightKind lightKind; // kind of light: OnOff or Dimmable
        friend class LightView; // allow {LightView to access private members

        uint8_t getBrightnessRaw() const {
            const uint8_t* d = getCurrentData();
            return d ? d[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] : DIMMER_STATUS_3_SWITCH_OFF;
        }

        void setBrightnessRaw(uint8_t bright) {
            uint8_t* d = getCurrentData();
            if (d != nullptr) {
                if (bright == DIMMER_STATUS_3_SWITCH_OFF) {
                    d[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = DIMMER_STATUS_3_SWITCH_OFF;
                } else {
                    if (bright > MAX_PERCENT) bright = MAX_PERCENT;
                    d[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] =
                        static_cast<uint8_t>(bright / RVC_PERCENT_PRECISION);
                }
            }
        }

        /**
        uint8_t getOnFlag(void) const {
            uint8_t isOn = SWITCH_OFF; // default to off
            uint8_t* rawData = (uint8_t* )getCurrentData();
            if (rawData != nullptr) {
                isOn = rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX]; // get the brightness value
            }
            return isOn;
        } */

        // creating cohesion between the view/controller and the model 
        // cmdSendOnOff is a callback for the HomeSpan SpanView derived class DC_LightSwitch - it will need to update the model per
        // actions made by the user
        /// @param buff 
        // friend void DC_LightSwitchView::cmdSendOnOff(const char *buff);
        // friend void DC_LightSwitchView::cmdOnOffStatus(const char* buff);

        bool isOn(void) const { // { return getOnFlag()>SWITCH_OFF; }
            return getBrightnessRaw() > DIMMER_STATUS_3_SWITCH_OFF;
        }

        bool isDimmable() const { return lightKind == LightKind::Dimmable; }

    protected:

        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                if (rawData != nullptr)// set the current data to the new data
                switch (dgn) {
                    case DC_DIMMER_COMMAND:
                    case DC_DIMMER_COMMAND_2:
                        rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = data[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX];
                        break;
                    case DC_DIMMER_STATUS_1:
                    case DC_DIMMER_STATUS_2:
                    case DC_DIMMER_STATUS_3:
                        if (data[DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX] == 0)
                            data[DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX] = DIMMER_STATUS_3_SWITCH_OFF; // SWITCH_OFF; // translate 
                        rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = data[DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX];
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }

        void setOn(bool on) {
            if (on) {
                setBrightnessRaw(MAX_PERCENT);
            } else {
                setBrightnessRaw(DIMMER_STATUS_3_SWITCH_OFF);
            }
        }

        uint8_t getBrightness() const {
            uint8_t result = 0;
            uint8_t d = getBrightnessRaw();
            if (d > DIMMER_STATUS_3_SWITCH_OFF) {
                if (d >= RVCBrightMax)
                    result = MAX_PERCENT;
                else
                    result = static_cast<uint8_t>(d * RVC_PERCENT_PRECISION);
            }
            return result;
        }
        
        CAN_frame_t* buildCommand(RVC_DGN dgn) override;
        // virtual boolean sendCommand(RVC_DGN dgn);

    public:


        LightDevice(uint8_t address, uint8_t instance, LightKind k=LightKind::Dimmable) : GenericDevice(address, instance),lightKind(k) {  // I chose dimmable cuz most in coach are
            // RV_PRINTF("LightDevice constructor called with address=%d, instance=%d\n", address, instance); 
            // setOnFlag(false); // initialize the switch to off
            // Constructor with parameters implementation
            setBrightnessRaw(DIMMER_STATUS_3_SWITCH_OFF);
            // RV_PRINTF("LightDevice(%u, %u%s)\n", address, instance, k==LightKind::Dimmable ? "Dimmable" : "OnOff");
        }

        LightDevice(uint8_t* data) : GenericDevice(data), lightKind(LightKind::Dimmable) {
            // Constructor with parameters implementation
        }
        virtual ~LightDevice() {
            // Destructor implementation
            
        } 


        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS) override; // execute command based on DGN and data received from the controller
};
// #endif // DC_SWITCH_H
