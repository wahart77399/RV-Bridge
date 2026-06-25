#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "PowerSensor.h"
#include "PowerSensorView.h"
#include "PacketQueue.h"

// #include "ChassisMobility.h"

bool PowerSensorView::updateView(void) {
        // 
    // printf("PowerSensorView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    PowerSensor* mdl = (PowerSensor* )getModel();
    if ((mdl != nullptr) && (voltageView != nullptr) && (currentView != nullptr))  {
        uint8_t* rawData = mdl->getCurrentData();
        uint16_t volt = mdl->rmsVoltage();
        int16_t amp = mdl->rmsCurrent();
        boolean vFault = (mdl->isOpenNeutralFault() || mdl->isOpenGroundFault() || mdl->isReversePolarityFault());
        boolean aFault = mdl->isGroundCurrentFault();
        //printf("PowerSensorView::updateView adjusted voltage = %f \n", adjVolt);
        // if (volt < 250)
        voltageView->rmsVoltage(volt);
        voltageView->voltageFault(vFault);
        //printf("PowerSensorView::updateView adjusted current = %f \n", adjAmp);
        // if (amp < 100)
        currentView->rmsCurrent(amp);
        currentView->currentFault(aFault);
        //printf("PowerSensorView::updateView rmsVoltage = %d, voltageFault = %d, rmsCurrent = %d, currentFault = %d \n", 
        //    volt, vFault, amp, aFault);
        PacketQueue::clearLastPacketReceiveTime();
        updated = true; 
    }
    // printf("PowerSensorView::updateView completed \n"); 
    return updated;
 }

 PowerSensorView::PowerSensorView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), voltageView(nullptr), currentView(nullptr) {
}

void PowerSensorView::createPowerSensorView(GenericDevice* model, const char* spanDevName) {
    printf("PowerSensorView::createPowerSensorView called\n");
    SpanView::prepHomeSpan();
    // PowerSensorView::createBridge();
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    PowerSensorView* tmp = new PowerSensorView(model, spanDevName);

    const char* append1 = " Voltage";
    const char* append2 = " Current";
    size_t buffer_size = strlen(spanDevName) + strlen(append1) + 1; 
    char* voltName = new char[buffer_size];
    strcpy(voltName, spanDevName);
    strcat(voltName, append1);
    buffer_size = strlen(spanDevName) + strlen(append2) + 1;
    char* currentName = new char[buffer_size];
    strcpy(currentName, spanDevName);
    strcat(currentName, append2);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    PowerSensorView::PowerSensorVoltage* vvw = new PowerSensorView::PowerSensorVoltage(tmp, (PowerSensor* )model, voltName);
    tmp->setVoltageView(vvw);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(currentName);
    PowerSensorView::PowerSensorCurrent* cvw = new PowerSensorView::PowerSensorCurrent(tmp, (PowerSensor* )model, currentName);
    tmp->setCurrentView(cvw);


    if (tmp != nullptr)
        printf("PowerSensorView::createPowerSensorView: tmp created successfully\n");
    else
        printf("PowerSensorView::createPowerSensorView: tmp creation failed\n");   
    printf("PowerSensorView::createPowerSensorView completed\n");
}
#endif // ifdef HOME_KIT_2