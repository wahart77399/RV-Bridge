#ifndef WATER_PUMP_VIEW_H
#define WATER_PUMP_VIEW_H // once I'm ready to define this, move this below ifndef
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
#include "PacketQueue.h"
#include <mutex>
#include "DGN.h"

class WaterPump;

class WaterPumpView : public SpanView {
    private:
        // constants
        const uint8_t OFF = 0;
        const uint8_t ON = 1;
        
        boolean needToUpdateView = true;
        // static bool bridgeCreated;
        // static void createBridge(void); 

        friend class WaterPump;

        inline void updateTheView(void) { needToUpdateView = true; } // set the flag to indicate view needs to be updated   
        inline void dontUpdateTheView(void) { needToUpdateView = false; } // reset the flag to indicate view does not need to be updated
        inline const boolean isNeedToUpdateView(void) const { return needToUpdateView; } // return the flag to indicate view needs to be updated

        struct WaterPumpController: Service::Switch {

                WaterPump*          model;
                WaterPumpView*      view;
                SpanCharacteristic* power;



                WaterPumpController(WaterPumpView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::Switch() {
                    power = new Characteristic::On();
                    this->model = (WaterPump* )mdl;
                    view = vw;
                }
                boolean update(void); 
                boolean isOn(void) { return power->getNewVal(); }
                void turnOn(boolean val) { power->setVal(val);PacketQueue::clearLastPacketReceiveTime();}
        }; 
        
        WaterPumpController controller;
        WaterPumpView(WaterPumpView& vw) = delete;

        WaterPumpView& operator=(const WaterPumpView&) = delete; // Prevent assignment

    protected:
                // @brief need to review this... doesn't seem right SpanService(type, name)
        WaterPumpView(GenericDevice* model, const char* spanDevName);

    public:
               
        /// @brief destructor
        virtual ~WaterPumpView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);


        static void createWaterPumpView(GenericDevice* model, const char* spanDevName); 

};

#endif