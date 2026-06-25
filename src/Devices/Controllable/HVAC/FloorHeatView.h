#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef FLOOR_HEAT_VIEW_H
#define FLOOR_HEAT_VIEW_H // once I'm ready to define this, move this below ifndef

#include "SpanView.h"
#include "HomeSpan.h"
#include "PacketQueue.h"
#include "DGN.h"
#include "FloorHeatDefinitions.h"
#include <map>
class FloorHeat;

class FloorHeatView : public SpanView {
    private:
        const char* spanDeviceName;

        friend class FloorHeat;
        typedef enum{
            FLOOR_HEAT_OFF = 0,
            FLOOR_HEAT_STORE,
            FLOOR_HEAT_LEVEL_1,
            FLOOR_HEAT_LEVEL_2,
            FLOOR_HEAT_LEVEL_3,
            FLOOR_HEAT_LEVEL_4,
            FLOOR_HEAT_LEVEL_5,
            FLOOR_HEAT_LEVEL_6,
            FLOOR_HEAT_LEVEL_7,
            FLOOR_HEAT_LEVEL_8,
            FLOOR_HEAT_LEVEL_9,
            FLOOR_HEAT_LEVEL_10
        } FLOOR_HEAT_LEVEL;
        std::map<FLOOR_HEAT_LEVEL, float_t> levelToTemperatureMap = {
            {FLOOR_HEAT_OFF, 0.0F},
            {FLOOR_HEAT_STORE, FLOOR_HEAT_TEMP_STORE},
            {FLOOR_HEAT_LEVEL_1, FLOOR_HEAT_TEMP_LEVEL_1},
            {FLOOR_HEAT_LEVEL_2, FLOOR_HEAT_TEMP_LEVEL_2},
            {FLOOR_HEAT_LEVEL_3, FLOOR_HEAT_TEMP_LEVEL_3},
            {FLOOR_HEAT_LEVEL_4, FLOOR_HEAT_TEMP_LEVEL_4},
            {FLOOR_HEAT_LEVEL_5, FLOOR_HEAT_TEMP_LEVEL_5},
            {FLOOR_HEAT_LEVEL_6, FLOOR_HEAT_TEMP_LEVEL_6},
            {FLOOR_HEAT_LEVEL_7, FLOOR_HEAT_TEMP_LEVEL_7},
            {FLOOR_HEAT_LEVEL_8, FLOOR_HEAT_TEMP_LEVEL_8},
            {FLOOR_HEAT_LEVEL_9, FLOOR_HEAT_TEMP_LEVEL_9},
            {FLOOR_HEAT_LEVEL_10, FLOOR_HEAT_TEMP_LEVEL_10}
        };

        float_t levelToTemperature(FLOOR_HEAT_LEVEL lvl); 
        
        float_t closestTemperature(float_t tempC) const {
            float_t closestTemp = FLOOR_HEAT_TEMP_STORE;
            float_t smallestDiff = std::abs(tempC - FLOOR_HEAT_TEMP_STORE);
            if (tempC < FLOOR_HEAT_TEMP_STORE) {
                closestTemp = FLOOR_HEAT_TEMP_OFF;
            }
            else {
                for (const auto& pair : levelToTemperatureMap) {
                    float_t diff = std::abs(tempC - pair.second);
                    if (diff < smallestDiff) {
                        smallestDiff = diff;
                        closestTemp = pair.second;
                    }
                }
            }
            return closestTemp;
        }

        struct FloorHeatController: Service::Thermostat {

            FloorHeat*              model;
            FloorHeatView*          view;

            SpanCharacteristic*     currentTemp;
            SpanCharacteristic*     targetTemp;
            SpanCharacteristic*     targetState;
            SpanCharacteristic*     currentState;
            // SpanCharacteristic*     onOff;
            SpanCharacteristic*     temperatureDisplayUnits;

            FloorHeatController(FloorHeatView* vw, FloorHeat* device) : Service::Thermostat() { 
                
                targetTemp = new Characteristic::TargetTemperature(FLOOR_HEAT_TEMP_STORE); // default to storage
                targetTemp->setRange(FLOOR_HEAT_TEMP_STORE - 1.0F, FLOOR_HEAT_TEMP_LEVEL_10, 
                                        (FLOOR_HEAT_TEMP_LEVEL_10 - FLOOR_HEAT_TEMP_LEVEL_9)); // set range from STORE to LEVEL_10 with 0.5 C steps
                currentTemp = new Characteristic::CurrentTemperature(FLOOR_HEAT_TEMP_STORE); // default to store if on
                temperatureDisplayUnits = new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit);
                targetState = new Characteristic::TargetHeatingCoolingState(FLOOR_HEAT_OFF); // off
                currentState = new Characteristic::CurrentHeatingCoolingState(FLOOR_HEAT_OFF); // off
                // onOff = new Characteristic::On(false); // default to off
                
                view = vw;
                model = device;

            }

            boolean update(void); 

            void turnOff(void) {
                // onOff->setVal(false);
                setTemperature(FLOOR_HEAT_TEMP_STORE - 1.0F); // set below store to indicate off
                // targetState->setVal(FLOOR_HEAT_OFF);
                // currentState->setVal(FLOOR_HEAT_OFF);
                PacketQueue::clearLastPacketReceiveTime();
            }

            void turnOn(void) {
                // onOff->setVal(true);
                // targetState->setVal(FLOOR_HEAT_ON); // on
                currentState->setVal(FLOOR_HEAT_ON);
                // targetTemp->setVal(FLOOR_HEAT_TEMP_STORE);
                // currentTemp->setVal(FLOOR_HEAT_TEMP_STORE);
                PacketQueue::clearLastPacketReceiveTime();
            }

            void setLevel(FLOOR_HEAT_LEVEL level) {
                float_t temp = view->levelToTemperature(level);
                setTemperature(temp);
                PacketQueue::clearLastPacketReceiveTime();
                // currentTemp->setVal(temp);
            }

            const FLOOR_HEAT_LEVEL getLevel(void) const {
                FLOOR_HEAT_LEVEL level = FLOOR_HEAT_OFF;
                float_t temp = targetTemp->getVal<float_t>();
                for (const auto& pair : view->levelToTemperatureMap) {
                    if (pair.second == temp) {
                        level = pair.first;
                        break;
                    }
                }
                return level; // default if not found
            }

            float_t getTemperature(void) const {
                // float_t temp = currentTemp->getVal<float_t>();
                float_t temp = targetTemp->getVal<float_t>();
                return temp;
            }

            void setTemperature(float_t temp) {
                if (temp >= FLOOR_HEAT_TEMP_STORE) {    
                    // onOff->setVal(true);
                    if (currentState->getVal<uint8_t>() != FLOOR_HEAT_ON) {
                        currentState->setVal(FLOOR_HEAT_ON);
                        // targetState->setVal(FLOOR_HEAT_ON); // on
                    }
                } else {
                    // onOff->setVal(false);
                    // targetState->setVal(FLOOR_HEAT_OFF);
                    currentState->setVal(FLOOR_HEAT_OFF);
                }    
                // targetTemp->setVal(temp);
                currentTemp->setVal(temp);
                PacketQueue::clearLastPacketReceiveTime();
            }

            //void setCurrentTemperature(float_t temp) {
            //    currentTemp->setVal(temp);
            // }

            

        };

        FloorHeatController*           controller;
        void setController(FloorHeatController* ctrl) {controller = ctrl; }

    protected:

        FloorHeatView(GenericDevice* mdl, const char* spanDeviceName);

    public:

        /// @brief destructor
        virtual ~FloorHeatView(void) { 
        }

        virtual bool updateView(void); // update the view based on the model's current data


        static void createFloorHeatView(GenericDevice* model, const char* spanDevName); 
};

#endif // ifndef FLOOR_HEAT_H
#endif // ifdef HOME_KIT_2