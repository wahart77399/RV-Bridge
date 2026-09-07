#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "AutomaticTransferSwitch.h"
#include "AutomaticTransferSwitchView.h"
#include "PacketQueue.h"

std::map<ATS_IO_TYPE, std::string>     AutomaticTransferSwitchView::ioTypeMap;
std::map<ATS_LEG_TYPE, std::string>    AutomaticTransferSwitchView::legMap;
std::map<ATS_SOURCE_TYPE, std::string> AutomaticTransferSwitchView::sourceMap;
boolean                                AutomaticTransferSwitchView::mapsInitialized = false; //  = false;

void AutomaticTransferSwitchView::initialize(void) {
    if (!mapsInitialized) {
        ioTypeMap[ATS_IO_Type::Input] = "Input";
        ioTypeMap[ATS_IO_Type::Output] = "Output";
        legMap[ATSLeg::Leg1] = "Leg 1";
        legMap[ATSLeg::Leg2] = "Leg 2";
        sourceMap[ATSSource::Primary] = "Primary";
        sourceMap[ATSSource::Secondary] = "Secondary";
        sourceMap[ATSSource::NoData] = "No Data";
        mapsInitialized = true;
    }
}


bool AutomaticTransferSwitchView::updateView(void) {
        // 
    // printf("AutomaticTransferSwitchView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    ATS_LEG_TYPE leg = ATSLeg::Leg1; // default to leg 1
    AutomaticTransferSwitch* mdl = (AutomaticTransferSwitch* )getModel();
    if ((mdl != nullptr)) { // && (voltageView != nullptr) && (currentView != nullptr))  {
        leg = mdl->getLeg();
        // printf("AutomaticTransferSwitchView::updateView leg %d\n", leg);
        if (voltageViewMap[leg] != nullptr && currentViewMap[leg] != nullptr) {  
            // printf("AutomaticTransferSwitchView::updateView updating view have mapped voltage and current leg\n");    
            uint8_t* rawData = mdl->getCurrentData();
            uint16_t volt = mdl->rmsVoltage(static_cast<uint8_t>(leg));
            uint16_t amp = mdl->rmsCurrent(static_cast<uint8_t>(leg));
            boolean vFault = (mdl->isOpenNeutralFault() || mdl->isOpenGroundFault() || mdl->isReversePolarityFault());
            boolean aFault = mdl->isGroundCurrentFault();
            //printf("AutomaticTransferSwitchView::updateView leg %d, rmsVoltage = %d, rmsCurrent = %d, voltageFault = %d, currentFault = %d \n",
            //        leg, volt, amp, vFault, aFault);
            voltageViewMap[leg]->rmsVoltage(volt);
            voltageViewMap[leg]->voltageFault(vFault);
            voltageViewMap[leg]->setIOType(mdl->getIOType());
            voltageViewMap[leg]->setLeg(leg);
            voltageViewMap[leg]->setSource(mdl->getSource());
            currentViewMap[leg]->rmsCurrent(amp);
            currentViewMap[leg]->currentFault(aFault);
            currentViewMap[leg]->setIOType(mdl->getIOType());
            currentViewMap[leg]->setLeg(mdl->getLeg());
            currentViewMap[leg]->setSource(mdl->getSource());
            PacketQueue::clearLastPacketReceiveTime();
            updated = true; 
        }
    }
    // printf("AutomaticTransferSwitchView::updateView completed \n"); 
    return updated;
 }

 AutomaticTransferSwitchView::AutomaticTransferSwitchView(AutomaticTransferSwitch* model, const char* spanDevName) 
        : SpanView(model), voltageViewMap(), currentViewMap() {
    initialize(); // Initialize the maps if not already done
}

void AutomaticTransferSwitchView::createAutomaticTransferSwitchView(AutomaticTransferSwitch* model, const char* spanDevName) {
    printf("AutomaticTransferSwitchView::createAutomaticTransferSwitchView called\n");
    SpanView::prepHomeSpan();
    // AutomaticTransferSwitchView::createBridge();
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    AutomaticTransferSwitchView* tmp = new AutomaticTransferSwitchView(model, spanDevName);

    const char* append1 = " Voltage";
    const char* append2 = " Current";
    size_t buffer_size = strlen(spanDevName) + strlen(append1) + strlen(AutomaticTransferSwitchView::legMap[ATSLeg::Leg1].c_str())+ 1; 
    char* voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, AutomaticTransferSwitchView::legMap[ATSLeg::Leg1].c_str());
    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(AutomaticTransferSwitchView::legMap[ATSLeg::Leg1].c_str()) + 1;
    char* currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, AutomaticTransferSwitchView::legMap[ATSLeg::Leg1].c_str());


    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    AutomaticTransferSwitchView::AutomaticTransferSwitchVoltage* vvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchVoltage(tmp, (AutomaticTransferSwitch* )model, voltName);
    tmp->setVoltageView(vvw, ATSLeg::Leg1);
    delete [] voltName;

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    AutomaticTransferSwitchView::AutomaticTransferSwitchCurrent* cvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchCurrent(tmp, (AutomaticTransferSwitch* )model, currentName);
    tmp->setCurrentView(cvw, ATSLeg::Leg1);
    delete [] currentName;
    // tmp->updateView(); // update the view with the current data
    buffer_size = strlen(spanDevName) + strlen(append1) + strlen(AutomaticTransferSwitchView::legMap[ATSLeg::Leg2].c_str())+ 1; 
    voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, AutomaticTransferSwitchView::legMap[ATSLeg::Leg2].c_str());

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    vvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchVoltage(tmp, (AutomaticTransferSwitch* )model, voltName);
    tmp->setVoltageView(vvw, ATSLeg::Leg2);
    delete [] voltName;

    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(AutomaticTransferSwitchView::legMap[ATSLeg::Leg2].c_str()) + 1;
    currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, AutomaticTransferSwitchView::legMap[ATSLeg::Leg2].c_str());

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    cvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchCurrent(tmp, (AutomaticTransferSwitch* )model, currentName);
    tmp->setCurrentView(cvw, ATSLeg::Leg2);
    delete [] currentName;


    if (tmp != nullptr)
        printf("AutomaticTransferSwitchView::createAutomaticTransferSwitchView: tmp created successfully\n");
    else
        printf("AutomaticTransferSwitchView::createAutomaticTransferSwitchView: tmp creation failed\n");   
    printf("AutomaticTransferSwitchView::createAutomaticTransferSwitchView completed\n");
}

#endif // ifdef HOME_KIT_2
