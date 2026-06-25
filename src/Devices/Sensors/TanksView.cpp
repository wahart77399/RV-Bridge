#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Tanks.h"
#include "TanksView.h"
#include "PacketQueue.h"
// #include "ChassisMobility.h"

bool TanksView::updateView(void) {
        // 
    // printf("DoorLockView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    Tanks* mdl = (Tanks* )getModel();
    if ((mdl != nullptr) && (tank != nullptr)) {
        uint8_t* rawData = mdl->getCurrentData();
        uint16_t tankSize = mdl->size();
        uint16_t absoluteLevel = mdl->level();
        uint16_t levelPercent = (100 * absoluteLevel)/tankSize;
        // printf("TankView::updateView: index = %d, levelPercent=%d, absolute level=%d, size=%d \n", mdl->index(), levelPercent, absoluteLevel, tankSize );
         // convert to Celsius if needed
        tank->setTankSize(tankSize);
        if (levelPercent < 100) {
            tank->setTankLevel(levelPercent);
            tank->setTankFullState(FILLING_TANK); // set the tank full state to filling
        } else {
            tank->setTankLevel(100); // set to 100% if the level is greater than 100%
            tank->setTankFullState(FULL_TANK); // set the tank full state
        }
        PacketQueue::clearLastPacketReceiveTime();
        updated = true; 
    }
    // printf("DoorLockView::updateView completed \n"); 
    return updated;
 }

TanksView::TanksView(GenericDevice* model, const char* spanDevName) : SpanView(model) {

}

void TanksView::createTanksView(GenericDevice* model, const char* spanDevName) {
    TanksView* vw = new TanksView(model, spanDevName);

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);

    const char* prepend = "Monitoring levels of ";
    size_t buffer_size = strlen(spanDevName) + strlen(prepend) + 1; 
    char* desc = new char[buffer_size];
    strcpy(desc, prepend);
    strcat(desc, spanDevName);
    
    TanksView::Tank* tank = new TanksView::Tank(spanDevName);
    tank->setName(spanDevName);
    tank->setDescription(desc);
    vw->setTank(tank);
    // vw->updateView();

    
    if (vw != nullptr)
        printf("TanksView::createTanksView: tmp created successfully\n");
    else
        printf("TanksView::createTanksView: tmp creation failed\n");   
    printf("TanksView::createTanksView completed\n");
 }

 #endif // ifdef HOME_KIT_2