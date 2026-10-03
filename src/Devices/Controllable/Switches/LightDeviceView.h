#include "RVConstants.h"
#pragma once
#ifdef HOME_KIT_1
// #ifndef LIGHTDEVICEVIEW_H
// #define DC_LIGHTSWITCHVIEW_H

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

#include "SpanView.h"
#include "HomeSpan.h"
#include "DGN.h"
#include "PacketQueue.h"
#include "LightDeviceCmd.h"
#include "debug.h"

class LightDevice;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class LightDeviceView : public SpanView {
    private:
        const uint8_t Lamp = 0; // not a dimmable switch
        // static const char* name; //  = "LightBulb"; // from Span.h
        // static const char* type; //  = "43"; // from Span.h
        static const char  ON_OFF_COMMAND; //  = 'o';
        static const char* ON_OFF_COMMAND_DESCRIPTION; // = "<index>=<state:0-1>,... - send onOff to <index>";
        static const char  ON_OFF_STATUS; // = 'O';
        static const char* ON_OFF_STATUS_DESCRIPTION; // = "<index> to retrieve current state in HomeSpan";
        
        struct LightController:Service::LightBulb {

                LightDevice*    	model = nullptr;
                LightDeviceView* 	view =  nullptr;
                SpanCharacteristic*	power = nullptr;
		        SpanCharacteristic*	brightness = nullptr;


                LightController(LightDeviceView* vw, GenericDevice* mdl, const char* spanDeviceName);
                boolean update(void); 
                boolean isOn(void) { return power->getNewVal(); }
                void turnOn(boolean val) { power->setVal(val);  PacketQueue::clearLastPacketReceiveTime();}
                void setBrightness(uint8_t bright) { 
                    if (bright <= MAX_PERCENT) 
                        brightness->setVal(bright); 
                    else
                        brightness->setVal(MAX_PERCENT);
                } 
                uint8_t getBrightness(void) { return (brightness != nullptr) ? brightness->getNewVal() : 0U; } 

        }; 

        LightController controller;
        boolean isItDimmable(void);
        void setItOn(bool on);
        void setItsBrightness(uint8_t val);
        


        LightDeviceView(LightDeviceView& vw) = delete;

        LightDeviceView& operator=(const LightDeviceView&) = delete; // Prevent assignment

        
        static bool bridgeCreated;
        static void createBridge(void); 


        // creating cohesion between the LightDevice and this class so that the model can friend the static callback cmdSendOnOff
        friend class LightDevice;


    protected:
        inline void turnOnLight(void) {
            controller.turnOn(true); // turn on the light
        }

        inline void turnOffLight(void) {
            controller.turnOn(false);  // turn off the light
        }

        // inline const char* getSpanDeviceName(void) const { return spanDeviceName; } // return the name of the device for this view
        

        const uint8_t RVCBrightMax = 200;
        // @brief need to review this... doesn't seem right SpanService(type, name)
        LightDeviceView(GenericDevice* model, const char* spanDevName);
    public:

        
        /// @brief destructor
        virtual ~LightDeviceView(void) { 
        }

        // update HomeSpan view per changes in model
        bool updateView(void) override;

        static void createLightDeviceView(GenericDevice* model, const char* spanDevName); 
                /// @brief destructor

};
// #endif // DC_SWITCHVIEW_H
#endif // HOME_KIT_1
