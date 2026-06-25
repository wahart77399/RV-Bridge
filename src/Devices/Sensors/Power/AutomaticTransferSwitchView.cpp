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
        ioTypeMap[ATS_INPUT_TYPE] = "Input";
        ioTypeMap[ATS_OUTPUT_TYPE] = "Output";
        legMap[ATS_LEG_1_TYPE] = "Leg 1";
        legMap[ATS_LEG_2_TYPE] = "Leg 2";
        sourceMap[ATS_SOURCE_PRIMARY_TYPE] = "Primary";
        sourceMap[ATS_SOURCE_SECONDARY_TYPE] = "Secondary";
        sourceMap[ATS_SOURCE_NO_DATA_TYPE] = "No Data";
        mapsInitialized = true;
    }
}


bool AutomaticTransferSwitchView::updateView(void) {
        // 
    // printf("AutomaticTransferSwitchView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    ATS_LEG_TYPE leg = ATS_LEG_1_TYPE; // default to leg 1
    AutomaticTransferSwitch* mdl = (AutomaticTransferSwitch* )getModel();
    if ((mdl != nullptr)) { // && (voltageView != nullptr) && (currentView != nullptr))  {
        leg = mdl->getLeg();
        // printf("AutomaticTransferSwitchView::updateView leg %d\n", leg);
        if (voltageViewMap[leg] != nullptr && currentViewMap[leg] != nullptr) {  
            // printf("AutomaticTransferSwitchView::updateView updating view have mapped voltage and current leg\n");    
            uint8_t* rawData = mdl->getCurrentData();
            uint16_t volt = mdl->rmsVoltage();
            uint16_t amp = mdl->rmsCurrent();
            boolean vFault = (mdl->isOpenNeutralFault() || mdl->isOpenGroundFault() || mdl->isReversePolarityFault());
            boolean aFault = mdl->isGroundCurrentFault();
        // double adjVolt = tempCfromTempF(volt);
        // printf("AutomaticTransferSwitchView::updateView adjusted voltage = %f \n", adjVolt);
        // if (adjVolt < 300.0)
            voltageViewMap[leg]->rmsVoltage(volt);
        // else
            // voltageView->rmsVoltage(-17.78); // if voltage is too high, set it to 0
            voltageViewMap[leg]->voltageFault(vFault);
            voltageViewMap[leg]->setIOType(mdl->getIOType());
            voltageViewMap[leg]->setLeg(leg);
            voltageViewMap[leg]->setSource(mdl->getSource());
        // double adjAmp = tempCfromTempF(amp);
        // printf("AutomaticTransferSwitchView::updateView adjusted current = %f \n", adjAmp);
        // if (adjAmp < 100.0)
            currentViewMap[leg]->rmsCurrent(amp);
        //else
        //    currentView->rmsCurrent(-17.78); // if current is too high,
            currentViewMap[leg]->currentFault(aFault);
            currentViewMap[leg]->setIOType(mdl->getIOType());
            currentViewMap[leg]->setLeg(mdl->getLeg());
            currentViewMap[leg]->setSource(mdl->getSource());
        // printf("AutomaticTransferSwitchView::updateView rmsVoltage = %d, voltageFault = %d, rmsCurrent = %d, currentFault = %d \n", 
        //     volt, vFault, amp, aFault);
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
    size_t buffer_size = strlen(spanDevName) + strlen(append1) + strlen(AutomaticTransferSwitchView::legMap[ATS_LEG_1_TYPE].c_str())+ 1; 
    char* voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, AutomaticTransferSwitchView::legMap[ATS_LEG_1_TYPE].c_str());
    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(AutomaticTransferSwitchView::legMap[ATS_LEG_1_TYPE].c_str()) + 1;
    char* currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, AutomaticTransferSwitchView::legMap[ATS_LEG_1_TYPE].c_str());


    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    AutomaticTransferSwitchView::AutomaticTransferSwitchVoltage* vvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchVoltage(tmp, (AutomaticTransferSwitch* )model, voltName);
    tmp->setVoltageView(vvw, ATS_LEG_1_TYPE);
    delete [] voltName;

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    AutomaticTransferSwitchView::AutomaticTransferSwitchCurrent* cvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchCurrent(tmp, (AutomaticTransferSwitch* )model, currentName);
    tmp->setCurrentView(cvw, ATS_LEG_1_TYPE);
    delete [] currentName;
    // tmp->updateView(); // update the view with the current data
    buffer_size = strlen(spanDevName) + strlen(append1) + strlen(AutomaticTransferSwitchView::legMap[ATS_LEG_2_TYPE].c_str())+ 1; 
    voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, AutomaticTransferSwitchView::legMap[ATS_LEG_2_TYPE].c_str());

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    vvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchVoltage(tmp, (AutomaticTransferSwitch* )model, voltName);
    tmp->setVoltageView(vvw, ATS_LEG_2_TYPE);
    delete [] voltName;

    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(AutomaticTransferSwitchView::legMap[ATS_LEG_2_TYPE].c_str()) + 1;
    currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, AutomaticTransferSwitchView::legMap[ATS_LEG_2_TYPE].c_str());

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    cvw = new AutomaticTransferSwitchView::AutomaticTransferSwitchCurrent(tmp, (AutomaticTransferSwitch* )model, currentName);
    tmp->setCurrentView(cvw, ATS_LEG_2_TYPE);
    delete [] currentName;


    if (tmp != nullptr)
        printf("AutomaticTransferSwitchView::createAutomaticTransferSwitchView: tmp created successfully\n");
    else
        printf("AutomaticTransferSwitchView::createAutomaticTransferSwitchView: tmp creation failed\n");   
    printf("AutomaticTransferSwitchView::createAutomaticTransferSwitchView completed\n");
}

#endif // ifdef HOME_KIT_2
