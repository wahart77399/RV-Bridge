#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef POWER_SENSOR_VIEW_H
#define POWER_SENSOR_VIEW_H
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
// #define CUSTOM_CHAR_HEADER

#include "HomeSpan.h"
#include "RVConstants.h"
#include "PacketQueue.h"
#include "DGN.h"

class PowerSensor;

// DC SwitchView provides the view of the HomeKit - it is both a View from MVC is built as a facade to the SpanService
// see HomeSpan.h for more info
class PowerSensorView : public SpanView  {
    private:
        
        //const char* spanDeviceName;
        //static bool bridgeCreated;
        //static void createBridge(void); 

        friend class PowerSensor;

        struct PowerSensorVoltage: Service::TemperatureSensor {

                PowerSensor*           model;
                PowerSensorView*       view;
                SpanCharacteristic*    voltage;
                SpanCharacteristic*    fault;



                PowerSensorVoltage(PowerSensorView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::TemperatureSensor() {
                    voltage = new Characteristic::CurrentTemperature();
                    fault = new Characteristic::StatusFault();
                    this->model = (PowerSensor* )mdl;
                    view = vw;
                    new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                    voltage->setRange(-20, 250); // need to get rid of magic numbers - but from 110v to 135v
                }
                void rmsVoltage(double val) { printf("PowerSensorView::PowerSensorVoltage::rmsVoltage val = %f\n", val); return voltage->setVal(tempCfromTempF(val)); }
                void voltageFault(boolean faulted) { fault->setVal(faulted); }
        }; 
        PowerSensorVoltage* voltageView;
        void setVoltageView(PowerSensorVoltage* vw) { voltageView = vw; }

        struct PowerSensorCurrent: Service::TemperatureSensor {
                PowerSensor*           model;
                PowerSensorView*       view;
                SpanCharacteristic*    current;
                SpanCharacteristic*    fault;



                PowerSensorCurrent(PowerSensorView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::TemperatureSensor() {
                    current = new Characteristic::CurrentTemperature();
                    fault = new Characteristic::StatusFault();
                    this->model = (PowerSensor* )mdl;
                    view = vw;
                    new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                    current->setRange(AAC_LOWER_LIMIT,AAC_UPPER_LIMIT); // shore current is limited to 50 amps, but generator is not 
                }
                void rmsCurrent(double val) { printf("PowerSensorView::PowerSensorCurrent::rmsCurrent val = %f\n", val); current->setVal(tempCfromTempF(val));  }
                void currentFault(boolean faulted) { fault->setVal(faulted); }
        }; 
        PowerSensorCurrent* currentView;
        void setCurrentView(PowerSensorCurrent* vw) { currentView = vw; }

    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        PowerSensorView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~PowerSensorView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createPowerSensorView(GenericDevice* model, const char* spanDevName); 
        
        
};
#endif  // POWER_SENSOR_VIEW_H
#endif // ifdef HOME_KIT_2
