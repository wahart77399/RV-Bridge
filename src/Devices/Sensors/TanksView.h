#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef TANKS_VIEW_H
#define TANKS_VIEW_H // once I'm ready to define this, move this below ifndef
#include "Arduino.h"

//#include "TanksDefinition.h"
#include "SpanView.h"

class Tanks;

class TanksView : SpanView {
    private:

            
        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        struct Tank : Service::TemperatureSensor {
            Characteristic::CurrentTemperature*      percent;
            Characteristic::StatusLowBattery*        statusLowTank;
            // Characteristic::ConfiguredName*    configuredName; // add ConfiguredName characteristic 
            Characteristic::Name                     name;
            uint16_t                                 sizeValue = 0; // size of tank in gallons  
            
            Tank(const char* nm) : Service::TemperatureSensor(), percent(nullptr), name(nm) {
                percent = new Characteristic::CurrentTemperature();
                statusLowTank = new Characteristic::StatusLowBattery();
                Serial.print("Tank:Service::BatteryService Configuring Tanks");                 // initialization message
                Serial.print("\n");
                percent->setDescription("Tank Percent");
                name.setDescription("Tank Name");
                
                percent->setRange(tempCfromTempF(ZERO_PERCENT_DEGREE_F), tempCfromTempF(ONE_HUNDRED_PERCENT_DEGREE_F)); // percent 
                name.setString("Tank"); // default name
                new Characteristic::ConfiguredName("Tank"); // add ConfiguredName characteristic

                new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );

            }

            void setTankLevel(const uint16_t levelPercent) { 
                double adjLevelPercent = tempCfromTempF(levelPercent);
                if (percent != nullptr) {
                    // percent.setVal(levelPercent);
                   percent->setVal(adjLevelPercent);
                }
 } 
            void setTankSize(const uint16_t tankSize) { sizeValue = tankSize; }
            void setName(const char* nm) { name.setString(nm); }
            void setDescription(const char* desc) { name.setDescription(desc); percent->setDescription(desc); }

            void setTankFullState(const TankFullState state) {
                if (statusLowTank != nullptr) {
                    statusLowTank->setVal(state);
                }
            }
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
#endif // ifdef HOME_KIT_2