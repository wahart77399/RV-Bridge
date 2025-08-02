#ifndef DC_SWITCH_H
#define DC_SWITCH_H
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
#include "GenericDevice.h"
#include "Packet.h"
#include "DC_LightSwitchView.h"
#include "DCDimmerCmd.h"

constexpr uint8_t DC_DIMMER_COMMAND_BRIGHTNESS_INDEX = 1; // index in the data array for the dimmer command
constexpr uint8_t DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX = 2; // index in the data array for the dimmer command duration

class DC_Switch : public GenericDevice {
    protected:
        const uint8_t SWITCH_OFF = 0x6;
    private:
        const uint8_t SWITCH_ON = 0xc8;
        const uint8_t STATUS_SWITCH_OFF = 0x00;
        
        const uint8_t RVCBrightMax = 200; 
        const uint8_t RVC_CONTINUOUS_DURATION = 0xff; // from spec RV-C Specification on DC_DIMMER_COMMAND_2
        


        uint8_t getOnFlag(void) const {
            uint8_t isOn = SWITCH_OFF; // default to off
            uint8_t* rawData = (uint8_t* )getCurrentData();
            if (rawData != nullptr) {
                isOn = rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX]; // get the brightness value
            }
            return isOn;
        }

        // creating cohesion between the view/controller and the model 
        // cmdSendOnOff is a callback for the HomeSpan SpanView derived class DC_LightSwitch - it will need to update the model per
        // actions made by the user
        /// @param buff 
        // friend void DC_LightSwitchView::cmdSendOnOff(const char *buff);
        // friend void DC_LightSwitchView::cmdOnOffStatus(const char* buff);
        friend bool DC_LightSwitchView::updateView(void);
        friend boolean DC_LightSwitchView::update(void);
        const bool isOn(void) const { return getOnFlag()>SWITCH_OFF; }

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
                            data[DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX] = SWITCH_OFF; // translate 
                        rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = data[DC_DIMMER_STATUS_3_BRIGHTNESS_INDEX];
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }

        void setOnFlag(bool on) {
            uint8_t* rawData = (uint8_t* )getCurrentData();
            if (rawData != nullptr) { 
                if (on && (rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] <= SWITCH_OFF)){
                    rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = SWITCH_ON;
                } else if (on && (rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] > SWITCH_OFF)) {
                    if (rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] > RVCBrightMax)
                        rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = RVCBrightMax;  // else do nothing
                } else
                    rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = SWITCH_OFF;
                updateViews();
            }
        }
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);
        // virtual boolean sendCommand(RVC_DGN dgn);

    public:
        DC_Switch() : GenericDevice() {
            // Constructor implementation
            ;
        }
        DC_Switch(const DC_Switch& orig) : GenericDevice(orig) {
            // Copy constructor implementation
            ;
        }

        DC_Switch(uint8_t address, uint8_t instance, uint8_t grp, std::list <RVC_DGN> dgns) : GenericDevice(address, instance, grp, dgns) { 
            printf("DC_Switch constructor called with address=%d, instance=%d, group=%d\n", address, instance, grp);
            setOnFlag(false); // initialize the switch to off
            // Constructor with parameters implementation
        }

        DC_Switch(uint8_t* data) : GenericDevice(data) {
            // Constructor with parameters implementation
        }
        virtual ~DC_Switch() {
            // Destructor implementation
            
        } 

        // virtual void printPacketData(RVC_DGN dgn, uint8_t* data, PacketPrint printPacket);

        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS); // execute command based on DGN and data received from the controller
};
#endif // DC_SWITCH_H