#include "RVConstants.h"
#ifdef HOME_KIT_2
#ifndef AUTOMATIC_TRANSFER_SWITCH_VIEW_H
#define AUTOMATIC_TRANSFER_SWITCH_VIEW_H // once I'm ready to define this, move this below if
// #include "AutomaticTransferSwitchView.h"
#include "SpanView.h"
#include "ATS_Definitions.h"
#define CUSTOM_CHAR_HEADER
#include "ATSView_Definitions.h"
#include "HomeSpan.h"
#include "RVConstants.h"
#include "PacketQueue.h"
#include "DGN.h"
#include <map>
#include <string>
#include "elapsedMillis.h"


class AutomaticTransferSwitch;
constexpr uint8_t DETECTED = 0;
constexpr uint8_t NOT_DETECTED = 1;

class AutomaticTransferSwitchView : public SpanView {
    private:
        
        //const char* spanDeviceName;
        //static bool bridgeCreated;
        //static void createBridge(void); 
        static std::map<ATS_IO_TYPE, std::string> ioTypeMap;
        static std::map<ATS_LEG_TYPE, std::string> legMap;
        static std::map<ATS_SOURCE_TYPE, std::string> sourceMap;
        static boolean mapsInitialized; //  = false;
        static void initialize(void);

        friend class AutomaticTransferSwitch;

        struct AutomaticTransferSwitchVoltage: Service::TemperatureSensor {

                AutomaticTransferSwitch*           model;
                AutomaticTransferSwitchView*       view;
                SpanCharacteristic*                voltage;
                SpanCharacteristic*                fault;
                Characteristic::ATSIOType*         ioType;
                Characteristic::ATSLeg*            leg;
                Characteristic::ATSSource*         source;

                AutomaticTransferSwitchVoltage(AutomaticTransferSwitchView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::TemperatureSensor() {
                    voltage = new Characteristic::CurrentTemperature();
                    fault = new Characteristic::StatusFault();
                    ioType = new Characteristic::ATSIOType();
                    leg = new Characteristic::ATSLeg();
                    source = new Characteristic::ATSSource();
                    this->model = (AutomaticTransferSwitch* )mdl;
                    view = vw;
                    new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                    voltage->setRange(-20, 55); // need to get rid of magic numbers - but from 110v to 135v
                }
                void rmsVoltage(double val) { /* printf("ATSView::ATSVoltage::rmsVoltage val = %f\n", tempCfromTempF(val)); */ return voltage->setVal(tempCfromTempF(val)); }
                void voltageFault(boolean faulted) { fault->setVal(faulted); }
                void setIOType(ATS_IO_TYPE type) { ioType->setString(ioTypeMap[type].c_str()); }
                void setLeg(ATS_LEG_TYPE legType) { leg->setString(legMap[legType].c_str()); }
                void setSource(ATS_SOURCE_TYPE sourceType) { source->setString(sourceMap[sourceType].c_str()); }
        }; 
        std::map<ATS_LEG_TYPE, AutomaticTransferSwitchVoltage*> voltageViewMap;
        
        void setVoltageView(AutomaticTransferSwitchVoltage* vw, ATS_LEG_TYPE leg) { voltageViewMap[leg] = vw; }

        struct AutomaticTransferSwitchCurrent: Service::TemperatureSensor {
                AutomaticTransferSwitch*           model;
                AutomaticTransferSwitchView*       view;
                SpanCharacteristic*                current;
                SpanCharacteristic*                fault;
                Characteristic::ATSIOType*         ioType;
                Characteristic::ATSLeg*            leg;
                Characteristic::ATSSource*         source;



                AutomaticTransferSwitchCurrent(AutomaticTransferSwitchView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::TemperatureSensor() {
                    current = new Characteristic::CurrentTemperature();
                    fault = new Characteristic::StatusFault();
                    ioType = new Characteristic::ATSIOType();
                    leg = new Characteristic::ATSLeg();
                    source = new Characteristic::ATSSource();
                    this->model = (AutomaticTransferSwitch* )mdl;
                    view = vw;
                    new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                    current->setRange(AAC_LOWER_LIMIT, AAC_UPPER_LIMIT); // shore current is limited to 50 amps, but generator is not 
                }
                void rmsCurrent(double val) { /* printf("ATSView::ATSCurrent::rmsCurrent val = %f\n", tempCfromTempF(val)); */ current->setVal(tempCfromTempF(val)); }
                void currentFault(boolean faulted) { fault->setVal(faulted); }
                void setIOType(ATS_IO_TYPE type) { ioType->setString(ioTypeMap[type].c_str()); }
                void setLeg(ATS_LEG_TYPE legType) { leg->setString(legMap[legType].c_str()); }
                void setSource(ATS_SOURCE_TYPE sourceType) { source->setString(sourceMap[sourceType].c_str()); }
        }; 
        std::map<ATS_LEG_TYPE, AutomaticTransferSwitchCurrent* > currentViewMap;
        void setCurrentView(AutomaticTransferSwitchCurrent* vw, ATS_LEG_TYPE leg) { currentViewMap[leg] = vw; }

        /*
        struct AutomaticTransferSwitchStatus: Service::ContactSensor {
                AutomaticTransferSwitch*            model;
                AutomaticTransferSwitchView*        view;
                Characteristic::ContactSensorState* status;
                

                AutomaticTransferSwitchStatus(AutomaticTransferSwitchView* vw, GenericDevice* mdl, const char* spanDeviceName) : Service::ContactSensor() {
                    status = new Characteristic::ContactSensorState(DETECTED);
                    this->model = (AutomaticTransferSwitch* )mdl;
                    view = vw;
                    // new Characteristic::TemperatureDisplayUnits(homeKitTemperatureDisplayFahrenheit );
                    // voltage->setRange(-20, 55); // need to get rid of magic numbers - but from 110v to 135v
                }
                void setStatus(std::map<ATS_LEG_TYPE, AutomaticTransferSwitchView::AutomaticTransferSwitchVoltage*> voltageMap) { 
                    boolean allGood = true;
                    for (const auto& [key, val] : voltageMap) {
                        if (val->fault->getVal()) {
                            allGood = false;
                            break;
                        }
                    }
                    if (allGood)
                        status->setVal(Characteristic::ContactSensorState::NOT_DETECTED);
                    else
                        status->setVal(Characteristic::ContactSensorState::DETECTED);
                }

        };
        */

        AutomaticTransferSwitchView(AutomaticTransferSwitch* mdl, const char* spanDeviceName);
    public:

        virtual boolean updateView(void);

                /// @brief destructor
        virtual ~AutomaticTransferSwitchView(void) { 
        }

        static void createAutomaticTransferSwitchView(AutomaticTransferSwitch* model, const char* spanDevName); 
};
#endif
#endif // ifdef HOME_KIT_2