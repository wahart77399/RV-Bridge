#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef INVERTER_VIEW_H
#define INVERTER_VIEW_H
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
#include "map"
#include "SpanView.h"
// #define CUSTOM_CHAR_HEADER

#include "HomeSpan.h"
#include "RVConstants.h"
#include "PacketQueue.h"
#include "DGN.h"

class Inverter;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class InverterView : public SpanView  {
    private:
        
        //const char* spanDeviceName;
        //static bool bridgeCreated;
        //static void createBridge(void); 

        friend class Inverter;

        struct InverterVoltage: Service::TemperatureSensor {

                Inverter*               model;
                InverterView*           view;
                SpanCharacteristic*     voltage;
                SpanCharacteristic*     fault;



                InverterVoltage(InverterView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::TemperatureSensor() {
                    voltage = new Characteristic::CurrentTemperature(tempCfromTempF(ZERO_VOLT_DEGREE_F));
                    fault = new Characteristic::StatusFault();
                    this->model = (Inverter* )mdl;
                    view = vw;
                    new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                    voltage->setRange(-20, 250); // need to get rid of magic numbers - but from 110v to 135v
                }
                void rmsVoltage(double val) { /* printf("InverterView::InverterVoltage::rmsVoltage val = %f\n", tempCfromTempF(val)); */ return voltage->setVal(tempCfromTempF(val)); }
                void voltageFault(boolean faulted) { fault->setVal(faulted); }
        }; 
        // InverterVoltage* voltageView;
        std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, InverterVoltage*>> voltageViewMap;
        void setVoltageView(InverterVoltage* vw, INVERTER_LINE_TYPE ln, INVERTER_IO_TYPE io) { voltageViewMap[ln][io] = vw; }

        struct InverterCurrent: Service::TemperatureSensor {
                Inverter*               model;
                InverterView*           view;
                SpanCharacteristic*     current;
                SpanCharacteristic*     fault;



                InverterCurrent(InverterView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::TemperatureSensor() {
                    current = new Characteristic::CurrentTemperature(tempCfromTempF(ZERO_AMP_DEGREE_F));
                    fault = new Characteristic::StatusFault();
                    this->model = (Inverter* )mdl;
                    view = vw;
                    new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                    current->setRange(AAC_LOWER_LIMIT,AAC_UPPER_LIMIT, 1); // shore current is limited to 50 amps, but generator is not 
                }
                void rmsCurrent(double val) { /* printf("InverterView::InverterCurrent::rmsCurrent val = %f\n", tempCfromTempF(val));*/ current->setVal(tempCfromTempF(val)); }
                void currentFault(boolean faulted) { fault->setVal(faulted); }
        }; 
        // InverterCurrent* currentView;
        std::map<INVERTER_LINE_TYPE, std::map<INVERTER_IO_TYPE, InverterCurrent*>> currentViewMap;
        void setCurrentView(InverterCurrent* vw, INVERTER_LINE_TYPE ln, INVERTER_IO_TYPE io) { currentViewMap[ln][io] = vw; }

    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        InverterView(GenericDevice* model, const char* spanDevName): SpanView(model), voltageViewMap(), currentViewMap() {
            // Constructor implementation
            // spanDeviceName = spanDevName;
            // if (!bridgeCreated)
            //     createBridge();
            // model->addView(this); // add this view to the model's list of views
        }
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~InverterView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createInverterView(GenericDevice* model, const char* spanDevName); 
        
        
};
#endif  // INVERTER_VIEW_H
#endif // ifdef HOME_KIT_2
