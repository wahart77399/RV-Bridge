#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "FloorHeatView.h"
#include "FloorHeat.h"
#include "Packet.h"
#include "DGN.h"

float_t FloorHeatView::levelToTemperature(FLOOR_HEAT_LEVEL lvl) {
            return levelToTemperatureMap[lvl];
} 


boolean FloorHeatView::FloorHeatController::update(void) {
    boolean updated = false;
    if (/* (onOff != nullptr) && */ (targetTemp != nullptr) /* && (currentTemp != nullptr) */ && (model != nullptr) && (view != nullptr)) {
        view->dontUpdateTheView();
        if (/* onOff->updated()) { //  || */ targetTemp->updated() || targetState->updated()) {
            float_t closestTemp = view->closestTemperature(targetTemp->getNewVal<float_t>());
            //printf("FloorHeatView::FloorHeatController::update  - onOff: %d, targetTemp - %f\n", 
            //   onOff->getNewVal(), closestTemp);
            if (/* onOff->getNewVal() == false) { // || */ (targetState->getNewVal() == FLOOR_HEAT_OFF) || (closestTemp < FLOOR_HEAT_TEMP_STORE)) {
                model->turnFloorHeatOff();
                turnOff();
            } else {
                model->turnFloorHeatOn(closestTemp);
                turnOn();
                setTemperature(closestTemp);
            }
            updated = true;
        } 
        view->updateTheView();
    }
    return (updated);  
}


FloorHeatView::FloorHeatView(GenericDevice* model, const char* spanDevName) 
        : SpanView(model), controller(nullptr) {
}


bool FloorHeatView::updateView(void) {
    // printf("FloorHeatView::updateView called\n");
    bool updated = false;
    if (isNeedToUpdateView()) { // don't mess with the state of the lock when the change is is initiated by the controller and not the model
        uint8_t instance = indexOfModel();   
        uint8_t index = -1;
        FloorHeat* mdl = (FloorHeat* )getModel();
        if ((mdl != nullptr) && (controller != nullptr)) { 
            // printf("FloorHeatView::updateView - on = %d, & temp is %f\n", mdl->isTurnedOn(), mdl->currentFloorHeatSetting());
            if ((mdl->currentFloorHeatSetting() >= FLOOR_HEAT_TEMP_STORE) || mdl->isTurnedOn()) {
                //printf("FloorHeatView::updateView - on with temp = %f\n", mdl->currentFloorHeatSetting());
                controller->turnOn();
                controller->setTemperature(mdl->currentFloorHeatSetting() >= FLOOR_HEAT_TEMP_STORE ? 
                                            mdl->currentFloorHeatSetting() : FLOOR_HEAT_TEMP_STORE);

            } else {
                //printf("FloorHeatView::updateView - off\n");
                controller->turnOff();
            }
            updated = true;
        }        
    }

    // printf("FloorHeatView::updateView completed \n"); 
    return updated;
}

void FloorHeatView::createFloorHeatView(GenericDevice* model, const char* spanDevName) {
    printf("FloorHeatView::createAwningView called\n");

    FloorHeatView* vw = new FloorHeatView(model, spanDevName);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    FloorHeatView::FloorHeatController* ctrl = new FloorHeatView::FloorHeatController(vw, (FloorHeat* )model);
    vw->setController(ctrl);

    if (vw != nullptr)
        printf("FloorHeatView::createAwningView: tmp created successfully\n");
    else
        printf("FloorHeatView::createAwningView: tmp creation failed\n");   
    printf("FloorHeatView::createAwningView completed\n");
}

#endif // ifdef HOME_KIT_2