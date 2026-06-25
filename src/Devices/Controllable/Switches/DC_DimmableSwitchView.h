#include "RVConstants.h"
#ifdef HOME_KIT_1
#ifndef DC_DIMMABLESWITCHVIEW_H
#define DC_DIMMABLESWITCHVIEW_H

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
#include "DCDimmerCmd.h"

class DC_DimmableSwitch;
constexpr uint8_t RVCBrightMax = 200;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class DC_DimmableSwitchView : public SpanView {
    protected:
        
    private:
        const uint8_t Lamp = 0; // not a dimmable switch
        static const char* name; //  = "DimmableBulb"; // from Span.h
        static const char* type; //  = "43"; // from Span.h
        static const char  DIMMABLE_FXN_COMMAND; //  = 'o';
        static const char* DIMMABLE_FXN_COMMAND_DESCRIPTION; // = "<index>=<state:0-1>,... - send onOff to <index>";

        struct DC_DimmableSwitchController: Service::LightBulb {

                DC_DimmableSwitch*          model;
                DC_DimmableSwitchView*      view;
                SpanCharacteristic*         power;
                SpanCharacteristic*         brightness;



                DC_DimmableSwitchController(DC_DimmableSwitchView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::LightBulb() {
                    power = new Characteristic::On();
                    brightness = new Characteristic::Brightness(25); // start with 1/4 brightness
                    this->model = (DC_DimmableSwitch* )mdl;
                    view = vw;
                }
                boolean update(void); 
                boolean isOn(void) { return power->getNewVal(); }
                void turnOn(boolean val) { power->setVal(val); PacketQueue::clearLastPacketReceiveTime();}
                void setBrightness(uint8_t bright) { 
                    if (bright <= MAX_PERCENT) 
                        brightness->setVal(bright); 
                    else
                        brightness->setVal(MAX_PERCENT);
                } 

                uint8_t getBrightness(void) { return brightness->getNewVal(); } // multiply by 2 for 
        }; 

        DC_DimmableSwitchController controller;
        DC_DimmableSwitchView(DC_DimmableSwitchView& vw) = delete;

        DC_DimmableSwitchView& operator=(const DC_DimmableSwitchView&) = delete; // Prevent assignment


        // creating cohesion between the DC_Switch and this class so that the model can friend the static callback cmdSendOnOff
        friend class DC_DimmableSwitch;


    protected:
        
        // @brief need to review this... doesn't seem right SpanService(type, name)
        DC_DimmableSwitchView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~DC_DimmableSwitchView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createDC_DimmableSwitchView(GenericDevice* model, const char* spanDevName); 
};
#endif // DC_SWITCHVIEW_H
#endif // HOME_KIT_1
