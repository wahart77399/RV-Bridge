#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef BATTERY_VIEW_H
#define BATTERY_VIEW_H
#define CUSTOM_CHAR_HEADER
#include "ATSView_Definitions.h"
#include "HomeSpan.h"
// #include "DC_SourceDefintions.h"
// #include "PowerDefinition.h"
#include "BatteryDefinitions.h"
#include "SpanView.h"
#include <map>
#include <string>


class Battery;

class BatteryView : SpanView {
    private:
        static std::map<DC_SOURCE_INSTANCE_DEFINITION, std::string> instanceMap;
        static std::map<DC_SOURCE_PRIORITY_DEFINTION,  std::string> priorityMap;
        static std::map<CHARGE_STATE, std::string>                  chargeStateMap;
        static std::map<BATTERY_TYPE, std::string>                  batteryTypeMap;
        static boolean mapsInitialized; //  = false;

        static void initialize(void);

    
        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        struct BatteryState : Service::TemperatureSensor {
            Characteristic::BatteryInstance             inst;
            Characteristic::BatteryPriority             priority;
            Characteristic::CurrentTemperature          batteryLevel;
            Characteristic::Voltage                     dcVoltage;
            Characteristic::Amperage                    dcCurrent;
            // DC_SOURCE_STATUS_2
            Characteristic::BatteryTemperature          temperature;
            Characteristic::CapacityRemaining           capacityRemaining;
            Characteristic::RMSAmperage                 rmsRipple;

            
            BatteryState() : Service::TemperatureSensor(), 
                                inst(), priority(),batteryLevel(tempCfromTempF(ONE_HUNDRED_PERCENT_DEGREE_F)), dcVoltage(12), dcCurrent(), temperature(72), 
                                capacityRemaining(),  rmsRipple() {
                inst.setDescription("Battery Instance");
                priority.setDescription("Battery Priority");
                batteryLevel.setDescription("Battery Level");
                dcVoltage.setDescription("DC Voltage");
                dcCurrent.setDescription("DC Current");
                temperature.setDescription("Battery Temperature");
                capacityRemaining.setDescription("Capacity Remaining");
                rmsRipple.setDescription("RMS Ripple");
                new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                batteryLevel.setRange(tempCfromTempF(ZERO_PERCENT_DEGREE_F), tempCfromTempF(ONE_HUNDRED_PERCENT_DEGREE_F * 1.25), 1); // allow for over 100% charge
                temperature.setRange(tempCfromTempF(ZERO_PERCENT_DEGREE_F), tempCfromTempF(ONE_HUNDRED_PERCENT_DEGREE_F * 2.0), 0.1); // allow for 200F
                capacityRemaining.setRange(0, 1000, 1); // 0 Ah to 
            }

            // status 1
            void setDCInstance(const DC_SOURCE_INSTANCE_DEFINITION instanceName) { inst.setString(instanceMap[instanceName].c_str()); }
            void setPriority(const DC_SOURCE_PRIORITY_DEFINTION priorityName)  { priority.setString(priorityMap[priorityName].c_str()); }
            void setBatteryLevel(const float_t val) { 
                float adjVal = tempCfromTempF(val);
                if ((adjVal >= (tempCfromTempF(ZERO_AMP_DEGREE_F))) && (adjVal <= tempCfromTempF(ONE_HUNDRED_PERCENT_DEGREE_F * 1.25))) {
                    batteryLevel.setVal(adjVal);
                    // printf("BatteryView::setBatteryLevel set to %f\n", adjVal); 
                }
            }
            void setDCVoltage(const uint8_t val)                               { dcVoltage.setVal(val); }
            void setDCCurrent(const float_t val)                               { /* printf("BatteryView::setDCCurrent val=%f\n", val); */ dcCurrent.setVal(val); }
            // status 2
            void setTemperature(const float_t val) { 
                if ((val >= (tempCfromTempF(ZERO_AMP_DEGREE_F))) && (val <= tempCfromTempF(ONE_HUNDRED_PERCENT_DEGREE_F * 2.0))) {
                    float adjVal = (val * 9.0/5.0) + 32.0; // celsius to fahrenheit
                    if (adjVal > static_cast<float_t>(ONE_HUNDRED_PERCENT_DEGREE_F)) 
                        adjVal = static_cast<float_t>(ONE_HUNDRED_PERCENT_DEGREE_F - 7.0); // limit to 93F for now - need to check with HomeKit spec 
                }
            } 
            void setCapacityRemaining(const uint16_t val)                      { /* printf("BatteryView::capacityRemaining %d\n", val); */ capacityRemaining.setVal(val); }
            void setRMSRipple(const float_t val)                               { /* printf("Battery:setRMSRipple val=%f\n", val); */ rmsRipple.setVal(val/1000.0); } // this is millivolts - need volts

        };
        BatteryState* batteryMeter;
        
        void setBatteryMeter( BatteryState* val) { batteryMeter = val; }

    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        BatteryView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~BatteryView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createBatteryView(GenericDevice* model, const char* spanDevName); 
        
};

#endif  // BATTERY_VIEW_H
#endif // ifdef HOME_KIT_2