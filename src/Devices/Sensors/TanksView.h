#include "RVConstants.h"
#ifndef TANKS_VIEW_H
#define TANKS_VIEW_H 
#include "Arduino.h"

#include "SpanView.h"
#include "debug.h"

class Tanks;

class TanksView : SpanView {
    private:

            
        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        struct Tank : Service::TemperatureSensor {
            Characteristic::CurrentTemperature*      percent;
            // Characteristic::StatusLowBattery*        statusLowTank;
            Characteristic::ConfiguredName*    configuredName;
            uint16_t                                 sizeValue = 0; // size of tank in gallons  
            
            Tank(const char* nm) : Service::TemperatureSensor(), percent(nullptr), configuredName(nullptr) {
                percent = new Characteristic::CurrentTemperature();
                // statusLowTank = new Characteristic::StatusLowBattery();
                Serial.print("Tank:Service::BatteryService Configuring Tanks");                 // initialization message
                Serial.print("\n");
                percent->setDescription("Tank Percent");
                
                percent->setRange(tempCfromTempF(ZERO_PERCENT_DEGREE_F), tempCfromTempF(ONE_HUNDRED_PERCENT_DEGREE_F)); // percent 
                configuredName = new Characteristic::ConfiguredName(nm);

            }

            void setTankLevel(const uint16_t levelPercent) { 
                double adjLevelPercent = tempCfromTempF(levelPercent);
                if (percent != nullptr) {
                    // percent.setVal(levelPercent);
                   percent->setVal(adjLevelPercent);
                }
 } 
            void setTankSize(const uint16_t tankSize) { sizeValue = tankSize; }
                void setDescription(const char* desc) { percent->setDescription(desc); }
        };
        Tank* tank;
        void setTank(Tank* tnk) { tank = tnk; }


    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        TanksView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~TanksView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createTanksView(GenericDevice* model, const char* spanDevName); 
        
};
#endif // TANKS_VIEW_H
