#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef GENERATOR_VIEW_H
#define GENERATOR_VIEW_H // once I'm ready to define this, move this below ifndef
#include "Arduino.h"
// #define CUSTOM_CHAR_HEADER
// #include "SpanView.h"
#include "PowerDefinition.h"
#include "SpanView.h"
#include "RVConstants.h"

class Generator;

class GeneratorView : SpanView {
    private:
            
        const char* spanDeviceName;
        static bool bridgeCreated;
        static void createBridge(void); 

        struct LegVoltage : Service::TemperatureSensor {
            SpanCharacteristic*         voltage;
            Characteristic::Name        name;
            
            LegVoltage(const char* nm) : Service::TemperatureSensor(), // model(mdl), view(vw),
                                                // leg1Voltage(125), leg2Voltage(125), 
                                                voltage(nullptr),
                                                name(nm) {
                Serial.print("Configuring Voltage Meter");                 // initialization message
                Serial.print("\n");
                voltage = new Characteristic::CurrentTemperature();
                // leg 1
                voltage->setDescription("Leg 1 Voltage");
                new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                voltage->setRange(tempCfromTempF(ZERO_VOLT_DEGREE_F),tempCfromTempF(TWO_FORTY_VOLT_DEGREE_F));
                voltage->setVal(tempCfromTempF(ZERO_VOLT_DEGREE_F));
            }

            void setVoltage(const double volt) { 
                if ((voltage != nullptr)) {
                    // leg1Voltage.setVal(volt);
                    double adjVolt = tempCfromTempF(volt);
                    if ((adjVolt >= tempCfromTempF(ZERO_VOLT_DEGREE_F)) && (adjVolt <= tempCfromTempF(TWO_FORTY_VOLT_DEGREE_F)))
                        voltage->setVal(adjVolt);
                }
            } 
        };

        LegVoltage* leg1VoltMeter;
        LegVoltage* leg2VoltMeter;
        void setLegVoltages( LegVoltage* val1, LegVoltage* val2) { leg1VoltMeter = val1; leg2VoltMeter = val2; }

        struct LegAmperage : Service::TemperatureSensor {
            // Characteristic::LegAmperage leg1Current;
            // Characteristic::LegAmperage leg2Current;
            SpanCharacteristic*         legCurrent;
            Characteristic::Name        name;

            LegAmperage(const char* nm) : Service::TemperatureSensor(), // leg1Current(), leg2Current(), 
                                                legCurrent(nullptr),name(nm) {
                Serial.print("Configuring Amperage Meter");                 // initialization message
                Serial.print("\n");
                legCurrent = new Characteristic::CurrentTemperature();
                // leg2TCurrent = new Characteristic::CurrentTemperature();
                // leg 1
                legCurrent->setDescription("Leg 1 Current");
                new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                legCurrent->setRange(tempCfromTempF(AAC_LOWER_LIMIT), tempCfromTempF(AAC_UPPER_LIMIT));
                // leg1Current.setRange(0, 50); // need to get rid of magic numbers - but from 0 to 50 amps
                // leg1Current.setDescription("Leg 1 Current");
                // leg 2   
                legCurrent->setVal(tempCfromTempF(ZERO_AMP_DEGREE_F));
            }
            void setLegCurrent(const double amps) { 
                // leg1Current.setVal(amps); 
                if (legCurrent != nullptr) {
                    double adjAmp = tempCfromTempF(amps);
                    // printf("GeneratorView::LegAmperage::setLegCurrent amps=%f, adjAmp=%f\n", amps, adjAmp);
                    if ((adjAmp >= tempCfromTempF(AAC_LOWER_LIMIT)) && (adjAmp <= tempCfromTempF(AAC_UPPER_LIMIT)))
                        legCurrent->setVal(adjAmp);
                } 
            }
        };
        LegAmperage* leg1AmpMeter;
        LegAmperage* leg2AmpMeter;
        void setLegAmperages( LegAmperage* val1, LegAmperage* val2) { leg1AmpMeter = val1; leg2AmpMeter = val2; }

    protected:
        // @brief need to review this... doesn't seem right SpanService(type, name)
        GeneratorView(GenericDevice* model, const char* spanDevName);
    public:

        // static void cmdCallback(RVC_DGN dgn, const char* buff);
        
        /// @brief destructor
        virtual ~GeneratorView(void) { 
        }

        // update HomeSpan view per changes in model
        virtual bool updateView(void);

        static void createGeneratorView(GenericDevice* model, const char* spanDevName); 
        
};
#endif  // GENERATOR_VIEW_H
#endif // ifdef HOME_KIT_2