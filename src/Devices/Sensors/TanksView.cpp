#include "RVConstants.h"
#include "Tanks.h"
#include "TanksView.h"
#include "PacketQueue.h"
#include "debug.h"
// #include "ChassisMobility.h"

bool TanksView::updateView(void) {
        // 
    // RV_PRINTF("DoorLockView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    Tanks* mdl = (Tanks* )getModel();
    if ((mdl != nullptr) && (tank != nullptr)) {
        uint8_t* rawData = mdl->getCurrentData();
        uint16_t tankSize = mdl->size();
        uint8_t levelPercent = mdl->levelPercent();
        bool hasTankLevel = levelPercent != OUT_OF_RANGE_DATA;
        // RV_PRINTF("TankView::updateView: index = %d, levelPercent=%d, absolute level=%d, size=%d \n", mdl->index(), levelPercent, absoluteLevel, tankSize );
         // convert to Celsius if needed
        if (tankSize > 0 && tankSize <= MAX_RVC_TANK_VALUE) {
            tank->setTankSize(tankSize);
        }
        if (hasTankLevel && levelPercent < 100) {
            tank->setTankLevel(levelPercent);
            // tank->setTankFullState(FILLING_TANK); // set the tank full state to filling
        } else if (hasTankLevel) {
            tank->setTankLevel(100); // set to 100% if the level is greater than 100%
            // tank->setTankFullState(FULL_TANK); // set the tank full state
        }
        if (mdl->hasAutoFillOperatingStatus() && autoFillActiveChar_ != nullptr) {
            autoFillActiveChar_->setVal(mdl->autoFillOperating()
                ? Characteristic::ContactSensorState::DETECTED
                : Characteristic::ContactSensorState::NOT_DETECTED);
        }
        if (mdl->hasAutoFillValveStatus() && autoFillValveChar_ != nullptr) {
            autoFillValveChar_->setVal(mdl->autoFillValveOpen()
                ? Characteristic::ContactSensorState::DETECTED
                : Characteristic::ContactSensorState::NOT_DETECTED);
        }
        if (mdl->hasAutoFillResult() && autoFillResultFaultChar_ != nullptr) {
            autoFillResultFaultChar_->setVal(mdl->autoFillResultFailed()
                ? Characteristic::ContactSensorState::DETECTED
                : Characteristic::ContactSensorState::NOT_DETECTED);
        }
        PacketQueue::clearLastPacketReceiveTime();
        updated = true; 
    }
    // RV_PRINTF("DoorLockView::updateView completed \n"); 
    return updated;
 }

TanksView::TanksView(GenericDevice* model, const char* spanDevName)
    : SpanView(model)
    , tank(nullptr)
    , autoFillActiveChar_(nullptr)
    , autoFillValveChar_(nullptr)
    , autoFillResultFaultChar_(nullptr) {

}

void TanksView::createTanksView(GenericDevice* model, const char* spanDevName) {
    TanksView* vw = new TanksView(model, spanDevName);

    SpanView::createAccessory();
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);

    const String desc = String("Monitoring levels of ") + spanDevName;
    
    TanksView::Tank* tank = new TanksView::Tank(spanDevName);
    tank->setDescription(desc.c_str());
    vw->setTank(tank);
    if ((model != nullptr) && (model->index() == Tanks::FRESH_WATER_INSTANCE)) {
        new Service::ContactSensor();
        new Characteristic::ConfiguredName("AutoFill Active");
        vw->autoFillActiveChar_ = new Characteristic::ContactSensorState();

        new Service::ContactSensor();
        new Characteristic::ConfiguredName("AutoFill Diverter Valve");
        vw->autoFillValveChar_ = new Characteristic::ContactSensorState();

        new Service::ContactSensor();
        new Characteristic::ConfiguredName("AutoFill Last Operation");
        vw->autoFillResultFaultChar_ = new Characteristic::ContactSensorState();
    }
    // vw->updateView();

    
    if (vw != nullptr)
        RV_PRINTF("TanksView::createTanksView: tmp created successfully\n");
    else
        RV_PRINTF("TanksView::createTanksView: tmp creation failed\n");   
    RV_PRINTF("TanksView::createTanksView completed\n");
 }
