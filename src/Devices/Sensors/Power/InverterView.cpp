#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Inverter.h"
#include "InverterView.h"
#include "PacketQueue.h"

// #include "ChassisMobility.h"

bool InverterView::updateView(void) {
        // 
    // printf("InverterView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    Inverter* mdl = (Inverter* )getModel();
    if ((mdl != nullptr)) { // && (voltageView != nullptr) && (currentView != nullptr))  {
        uint8_t* rawData = mdl->getCurrentData();
        INVERTER_LINE_TYPE ln = mdl->getLine();
        INVERTER_IO_TYPE io = mdl->getIO();
        if ((voltageViewMap[ln][io] != nullptr) && (currentViewMap[ln][io] != nullptr)) {
            // if (instance > 1)      
            //     printf("InverterView::updateView updating view for index %d have mapped voltage and current line %d io %d\n",instance, ln, io);
            uint16_t volt = mdl->rmsVoltage();
            int16_t amp = mdl->rmsCurrent();
            boolean vFault = (mdl->isOpenNeutralFault() || mdl->isOpenGroundFault() || mdl->isReversePolarityFault());
            boolean aFault = mdl->isGroundCurrentFault();
            // printf("InverterView::updateView voltage = %f, current = %f \n", volt, amp);

            // if (volt < 250)
            voltageViewMap[ln][io]->rmsVoltage(volt);
            voltageViewMap[ln][io]->voltageFault(vFault);
            //printf("InverterView::updateView adjusted current = %f \n", adjAmp);
            // if (amp < 100)
            currentViewMap[ln][io]->rmsCurrent(amp);
            currentViewMap[ln][io]->currentFault(aFault);
            //printf("InverterView::updateView rmsVoltage = %d, voltageFault = %d, rmsCurrent = %d, currentFault = %d \n", 
            //    volt, vFault, amp, aFault);
            PacketQueue::clearLastPacketReceiveTime();
            updated = true; 
        }
    }
    // printf("InverterView::updateView completed \n"); 
    return updated;
 }

 /**
 InverterView::InverterView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), voltageViewMap(), currentViewMap() {
} */

void InverterView::createInverterView(GenericDevice* model, const char* spanDevName) {
    printf("InverterView::createInverterView called\n");
    SpanView::prepHomeSpan();
    // InverterView::createBridge();
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    InverterView* tmp = new InverterView(model, spanDevName);

    // line 1 input
    
    const char* append1 = " Voltage";
    const char* line1Append = " Line 1";
    const char* line2Append = " Line 2";
    const char* ioInputAppend = " Input";
    const char* ioOutputAppend = " Output";

    const char* append2 = " Current";
    size_t buffer_size = strlen(spanDevName) + strlen(append1) + strlen(line1Append) + strlen(ioInputAppend) + 1; 
    char* voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, line1Append);
    strcat(voltName, ioInputAppend);


    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(line1Append) + strlen(ioInputAppend) + 1;
    char* currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, line1Append);
    strcat(currentName, ioInputAppend);
    // line 1 input
    /**
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    InverterView::InverterVoltage* vvw = new InverterView::InverterVoltage(tmp, (Inverter* )model, voltName);
    tmp->setVoltageView(vvw, INVERTER_LINE_1_TYPE, INVERTER_INPUT);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    InverterView::InverterCurrent* cvw = new InverterView::InverterCurrent(tmp, (Inverter* )model, currentName);
    tmp->setCurrentView(cvw, INVERTER_LINE_1_TYPE, INVERTER_INPUT);
    */
    delete[] voltName;
    delete[] currentName;

    // line 1 output

    buffer_size = strlen(spanDevName) + strlen(append1) + strlen(line1Append) + strlen(ioOutputAppend) + 1; 
    voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, line1Append);
    strcat(voltName, ioOutputAppend);
  


    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(line1Append) + strlen(ioOutputAppend) + 1;
    currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, line1Append);
    strcat(currentName, ioOutputAppend);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    InverterView::InverterVoltage* vvw = new InverterView::InverterVoltage(tmp, (Inverter* )model, voltName);
    tmp->setVoltageView(vvw, INVERTER_LINE_1_TYPE, INVERTER_OUTPUT);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    InverterView::InverterCurrent* cvw = new InverterView::InverterCurrent(tmp, (Inverter* )model, currentName);
    tmp->setCurrentView(cvw, INVERTER_LINE_1_TYPE, INVERTER_OUTPUT);

    delete[] voltName;
    delete[] currentName;


    // line 2 input
    /**
    buffer_size = strlen(spanDevName) + strlen(append1) + strlen(line2Append) + strlen(ioInputAppend) + 1; 
    voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, line2Append);
    strcat(voltName, ioInputAppend);


    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(line2Append) + strlen(ioInputAppend) + 1;
    currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, line2Append);
    strcat(currentName, ioInputAppend);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    vvw = new InverterView::InverterVoltage(tmp, (Inverter* )model, voltName);
    tmp->setVoltageView(vvw, INVERTER_LINE_2_TYPE, INVERTER_INPUT);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    cvw = new InverterView::InverterCurrent(tmp, (Inverter* )model, currentName);
    tmp->setCurrentView(cvw, INVERTER_LINE_2_TYPE, INVERTER_INPUT);

    delete[] voltName;
    delete[] currentName;

    // line 2 output
    buffer_size = strlen(spanDevName) + strlen(append1) + strlen(line2Append) + strlen(ioOutputAppend) + 1; 
    voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    strcat(voltName, line2Append);
    strcat(voltName, ioOutputAppend);


    buffer_size = strlen(spanDevName) + strlen(append2) + strlen(line2Append) + strlen(ioOutputAppend) + 1;
    currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);
    strcat(currentName, line2Append);
    strcat(currentName, ioOutputAppend);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    vvw = new InverterView::InverterVoltage(tmp, (Inverter* )model, voltName);
    tmp->setVoltageView(vvw, INVERTER_LINE_2_TYPE, INVERTER_OUTPUT);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    cvw = new InverterView::InverterCurrent(tmp, (Inverter* )model, currentName);
    tmp->setCurrentView(cvw, INVERTER_LINE_2_TYPE, INVERTER_OUTPUT);

    delete[] voltName;
    delete[] currentName;
    */
    

    if (tmp != nullptr)
        printf("InverterView::createInverterView: tmp created successfully\n");
    else
        printf("InverterView::createInverterView: tmp creation failed\n");   
    printf("InverterView::createInverterView completed\n");
}

#endif // ifdef HOME_KIT_2