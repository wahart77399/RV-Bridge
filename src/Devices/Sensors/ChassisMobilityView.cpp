#include "ChassisMobility.h"
#include "ChassisMobilityView.h"
#include "PacketQueue.h"


bool ChassisMobilityView::updateView(void) {
        // 
    // printf("ChassisMobilityView::updateView called\n");
    bool updated = false;
    // uint8_t instance = indexOfModel();   
    // uint8_t index = -1;
    ChassisMobility* mdl = (ChassisMobility* )getModel();
    if ((mdl != nullptr) && (chassisMobilitySensor != nullptr))  {
        // uint8_t* rawData = mdl->getCurrentData();
        MOTION_STATE motion = (mdl->isParked()) ? NOT_IN_MOTION : IN_MOTION;
        // printf("ChassisMobility::updateView Motion State = %d\n", motion);
        chassisMobilitySensor->detected(motion);
        updated = true; 
    }
    // printf("ChassisMobilityView::updateView completed \n"); 
    return updated;
 }

 ChassisMobilityView::ChassisMobilityView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), chassisMobilitySensor(nullptr) {
}

void ChassisMobilityView::createChassisMobilityView(GenericDevice* model, const char* spanDevName) {
    printf("ChassisMobilityView::createChassisMobilityView called\n");
    SpanView::prepHomeSpan();
    // ChassisMobilityView::createBridge();
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    ChassisMobilityView* tmp = new ChassisMobilityView(model, spanDevName);


    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    ChassisMobilityView::ChassisMobilityMotion* mvw = new ChassisMobilityView::ChassisMobilityMotion(tmp, (ChassisMobility* )model, spanDevName);
    tmp->setSensorView(mvw);



    if (tmp != nullptr)
        printf("ChassisMobilityView::createChassisMobilityView: tmp created successfully\n");
    else
        printf("ChassisMobilityView::createChassisMobilityView: tmp creation failed\n");   
    printf("ChassisMobilityView::createChassisMobilityView completed\n");
}
