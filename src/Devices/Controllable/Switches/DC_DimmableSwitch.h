
#ifndef DC_DIMMABLE_SWITCH_H
#define DC_DIMMABLE_SWITCH_H // once I'm ready to define this, move this below ifndef
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
#include "DC_Switch.h"
#include "DC_DimmableSwitchView.h"

class DC_DimmableSwitch : public  DC_Switch {
    private:

        void setBrightness(uint8_t brightness) {
            uint8_t* rawData = (uint8_t* )getCurrentData();
            if (rawData != nullptr) {
                rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX] = brightness; // set the brightness value
             updateViews(); // update all views
            }
        }

        uint8_t getBrightness(void) const {
            uint8_t brightness = 0x6;
            uint8_t* rawData = (uint8_t* )getCurrentData();
            if (rawData != nullptr) {
                brightness = rawData[DC_DIMMER_COMMAND_BRIGHTNESS_INDEX]; // get the brightness value
            }
            return brightness;
        }

        // creating cohesion between the view/controller and the model
        friend bool DC_DimmableSwitchView::updateView(void);
        friend boolean DC_DimmableSwitchView::update(void);    
    protected:

        const uint8_t getBrightnessValue(void) const { return getBrightness(); }    

    public:
         DC_DimmableSwitch() : DC_Switch() {
            // Constructor implementation
            ;
        }
         DC_DimmableSwitch(const  DC_DimmableSwitch& orig) : DC_Switch(orig) {
            // Copy constructor implementation
            ;
        }

         DC_DimmableSwitch(uint8_t address, uint8_t instance) : DC_Switch(address, instance) { 
            printf(" DC_DimmableSwitch constructor called with address=%d, instance=%d \n", address, instance); 
            // Constructor with parameters implementation
        }

         DC_DimmableSwitch(uint8_t* data) : DC_Switch(data) {
            // Constructor with parameters implementation
        }
        virtual ~ DC_DimmableSwitch() {
            // Destructor implementation
            
        } 


};
#endif
