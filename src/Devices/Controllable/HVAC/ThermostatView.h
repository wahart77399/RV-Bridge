
#ifndef THERMOSTAT_VIEW_H
#define THERMOSTAT_VIEW_H

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
#include <mutex>
#include "DGN.h"
#include "HVAC_defintions.h"



class HVAC_Thermostat;


class ThermostatView : public SpanView {
    private:
        // static
        
        boolean needToUpdateView = true;
        static bool categoryCreated;
        static void createCategory(void); 

        friend class HVAC_Thermostat;

        inline void updateTheView(void) { needToUpdateView = true; } // set the flag to indicate view needs to be updated   
        inline void dontUpdateTheView(void) { needToUpdateView = false; } // reset the flag to indicate view does not need to be updated
        inline const boolean isNeedToUpdateView(void) const { return needToUpdateView; } // return the flag to indicate view needs to be updated
        static inline float degCfromDegF(float degF) {
	        return degF / 1.8;
        }

        static inline float tempCfromTempF(float tempF) {
            return degCfromDegF(tempF - 32.0);
        }

        struct FanController: Service::Fan {

            HVAC_Thermostat*    model;
            ThermostatView*     view;

            SpanCharacteristic* _active;
	        SpanCharacteristic* currentState;
	        SpanCharacteristic* targetState;
	        SpanCharacteristic* speed;

            FanController(ThermostatView* vw, HVAC_Thermostat* device) : Service::Fan() { 
                
    		    _active = new Characteristic::Active(false);
		        speed = new Characteristic::RotationSpeed(INITIAL_ROTATION_SPEED);
		        speed->setRange(0, vw->HomeKitPercentMax, vw->HomeKitPercentMax/2);
		        currentState = new Characteristic::CurrentFanState(HomeKitHVACStates::currentFanStateIdle);
		        targetState = new Characteristic::TargetFanState(HomeKitHVACStates::targetFanStateAuto);
                view = vw;
                model = device;

            } 
	
	        boolean update();
            void setModeSpeed(uint8_t fanMode, uint8_t speed);
            RVCFanMode getMode(void) const;
            uint16_t getSpeed(void) const;
            void setLevel(uint8_t level); 
		
        }; 
        
        struct ThermostatController: Service::Thermostat {

                HVAC_Thermostat*    model;
                ThermostatView*     view;

                SpanCharacteristic* ambientTemp;
	            SpanCharacteristic* targetTemp;
	            SpanCharacteristic* targetState;
	            SpanCharacteristic* currentState;

 
                ThermostatView::FanController*       fan;

                // double convToTempC(uint16_t value);
                // uint16_t convFromTempC(double tempC);
                // float degCfromDegF(float degF);
                // float dtempCfromTempF(float tempF);

                ThermostatController(ThermostatView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::Thermostat() {
                    ambientTemp = new Characteristic::CurrentTemperature(tempCfromTempF(DEFAULT_TEMP));
                    targetTemp = new Characteristic::TargetTemperature(tempCfromTempF(DEFAULT_TEMP));
                    currentState = new Characteristic::CurrentHeatingCoolingState(heatingCoolingStateOff);
                    targetState = new Characteristic::TargetHeatingCoolingState(heatingCoolingStateOff);
                    fan = new ThermostatView::FanController(vw, (HVAC_Thermostat*) mdl); 
                    targetTemp->setRange(tempCfromTempF(50), tempCfromTempF(95), degCfromDegF(1.0))->setVal(tempCfromTempF(68));
                    this->model = (HVAC_Thermostat* )mdl;
                    view = vw;
                }
                boolean update(void); 
                // void setLevel(uint8_t index, uint8_t level);
                void setAmbientTemp(uint8_t index, double tempC);
                void setInfo(RVCMode opMode, RVCFanMode fanMode, RVCForcedFan fanSpeed, double heatTemp, double coolTemp);
                

        }; 
        
        ThermostatController controller;
        ThermostatView(ThermostatView& vw) = delete;

        ThermostatView& operator=(const ThermostatView&) = delete; // Prevent assignment

    protected:
                // @brief need to review this... doesn't seem right SpanService(type, name)
        ThermostatView(GenericDevice* model, const char* spanDevName);
        // bool updateView(void); //  { this->updateView(); return true;}

    public:
               
        /// @brief destructor
        virtual ~ThermostatView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);


        static void createThermostatView(GenericDevice* model, const char* spanDevName); 

};

#endif