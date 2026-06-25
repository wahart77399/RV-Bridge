#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Battery.h"
#include "BatteryView.h"
#include "PacketQueue.h"
//#include "RVConstants.h"
// #include "ChassisMobility.h"


boolean BatteryView::mapsInitialized = false;
std::map<DC_SOURCE_INSTANCE_DEFINITION, std::string> BatteryView::instanceMap;
std::map<DC_SOURCE_PRIORITY_DEFINTION,  std::string> BatteryView::priorityMap;
std::map<CHARGE_STATE, std::string>                  BatteryView::chargeStateMap;
std::map<BATTERY_TYPE, std::string>                  BatteryView::batteryTypeMap;

void BatteryView::initialize(void) {
    if (!mapsInitialized) {
        instanceMap[INVALID_SOURCE] = "Invalid Source";
        instanceMap[MAIN_HOUSE_BATTERY] = "Main House Battery Bank";
        instanceMap[CHASSIS_START_BATTERY] = "Chassis Start Battery";
        instanceMap[SECONDARY_HOUSE_BATTERY] = "Secondary House Battery Bank";
        instanceMap[GENERATOR_STARTER_BATTERY] = "Generator Starter Battery";
        instanceMap[MFG_OTHER] = "Other";

        priorityMap[BATTERY_SOC_BMS_DEVICE] = "Battery SOC/BMS Device";
        priorityMap[INVERTER_CHARGER] = "Inverter / Charger";
        priorityMap[CHARGER] = "Charger";
        priorityMap[INVERTER] = "Inverter";
        priorityMap[MULTI_METER] = "Voltmeter/Ammeter";
        priorityMap[VOLT_METER] = "Voltmeter";
        priorityMap[NO_PRIORITY_ALWAYS_RPT] = "No prioirty - always reporting";

        chargeStateMap[UNDEFINED_CHARGE_STATE] = "Undefined, charging source decides (default)";
        chargeStateMap[DO_NOT_CHARGE_STATE] = "Do not charge";
        chargeStateMap[BULK_CHARGE_STATE] = "Bulk";
        chargeStateMap[ABSORPTION_STATE] = "Absorption";
        chargeStateMap[OVERCHARGE_STATE] = "Overcharge";
        chargeStateMap[EQUALIZE_STATE] = "Equalize";
        chargeStateMap[FLOAT_STATE] = "Float";
        chargeStateMap[CONSTANT_VOLTAGE_STATE] = "Constant Voltage / Current";

        batteryTypeMap[FLOODED_BATTERY] = "Flooded";
        batteryTypeMap[GEL_BATTERY] = "Gel";
        batteryTypeMap[AGM_BATTERY] = "AGM";
        batteryTypeMap[LITHIUM_BATTERY] = "Lithium-ion-Phosphate";
        batteryTypeMap[VENDOR_DEFINED] = "Reserved for Vendor";
    }
}

bool BatteryView::updateView(void) {
        // 
    // printf("DoorLockView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    Battery* mdl = (Battery* )getModel();
    if ((mdl != nullptr) && (batteryMeter != nullptr))  {
        uint8_t* rawData1 = mdl->getSource1Data();
        uint8_t* rawData2 = mdl->getSource2Data();
        uint8_t* rawData3 = mdl->getSource3Data();
        uint8_t* rawData4 = mdl->getSource4Data();

        // status 1
        if (rawData1 != nullptr) {
            batteryMeter->setPriority((DC_SOURCE_PRIORITY_DEFINTION)rawData1[DC_SOURCE_PRIORITY_INDEX]);
        // printf("BatteryView::updateView - Status1 -index = %d, priority =%d DCVoltage = %d, DC Current = %d\n", 
        //    rawData1[0], rawData1[1], mdl->directCurrentVoltage(), mdl->directCurrentAmperage() );
            double lvl = mdl->level();
        //double adjLvl = tempCfromTempF(lvl); 
       // if ( /* (lvl != OUT_OF_RANGE_DATA) && */ (adjLvl >= -17.7778) && (adjLvl <= 37.7779)) {
            //printf("Battery::updateView battery level set to %f\n", adjLvl);
            batteryMeter->setBatteryLevel(lvl);
        //} else {
            // printf("Battery::updateView battery level set to 0\n");
        //    batteryMeter->setBatteryLevel(0.0); // set to 0 if out of range
        //} 
            double voltage = mdl->directCurrentVoltage();
            int32_t current = mdl->directCurrentAmperage();
            double val = static_cast<double>(current);
        // printf("BatteryView::updateView current = %d\n", current);
        // printf("BatteryView::updateView DCCurrent = %f\n", val);
            if ((voltage != OUT_OF_RANGE_DATA) && ((voltage >= ZERO_PERCENT_DEGREE_F) && (voltage <= TWO_FORTY_VOLT_DEGREE_F))) {
                batteryMeter->setDCVoltage(voltage);
            // printf("Battery::updateView DCVoltage = %f\n", voltage);
            }
            if ((current <= AAC_UPPER_LIMIT) && (current >= AAC_LOWER_LIMIT)) { // ) && ((current >= ZERO_PERCENT_DEGREE_F) && (current <= SIXTY_AMP_DEGREE_F))) 
            // printf("Battery::updateView DCCurrent = %f\n", val);
                batteryMeter->setDCCurrent(val);
            }
            batteryMeter->setDCInstance((DC_SOURCE_INSTANCE_DEFINITION)instance);
        // batteryMeter->setDCCurrent(mdl->directCurrentAmperage());
        }
        // status 2
        float temp = mdl->temperature();
        //if ( /* (temp != OUT_OF_RANGE_DATA) && */ (temp >= -17.7778) && (temp <= 37.7778)) {
            // printf("BatteryView::updateView - Status2 - index = %d, temperature = %f\n", 
            //          instance, temp);
            batteryMeter->setTemperature(temp);
        //}
        //} else {
        //    printf("BatteryView::updateView (else) - Status2 - index = %d, temperature out of range: %f\n", 
        //            instance, temp);
        // }
        uint8_t chgState = mdl->level();
        /* 
        if ((chgState != UNDEFINED_CHARGE_STATE) && (chgState >= ZERO_PERCENT_DEGREE_F) && (chgState <= (MAX_RVC_PERCENT * RVC_PERCENT_PRECISION))){
            
            batteryMeter->setStateOfCharge(chgState);
            //printf("BatteryView::updateView - Status2 - index = %d, state of charge = %d\n", 
            //        instance, chgState);
        }
        * REMOVED - not currently used by Newmar
        if ((mdl->timeRemaining() != OUT_OF_RANGE_DATA) && (mdl->timeRemaining() >= 0) && (mdl->timeRemaining() <= 10000)) {
            // data indicates from my observations that the 3 remaining bytes are always 0xff
            // likely not valid and thus not used
            batteryMeter->setTimeRemaining(mdl->timeRemaining());
            batteryMeter->setInterpretation(mdl->timeToEmpty());
        }
        
        // status 3
        if (mdl->health() != OUT_OF_RANGE_DATA) 
            batteryMeter->setHealth(mdl->health());
        */
        if (mdl->remainingCapacity() != OUT_OF_RANGE_DATA) {
            batteryMeter->setCapacityRemaining(mdl->remainingCapacity());
            //printf("BatteryView::updateView - Status3 - index = %d, capacity remaining = %d\n", 
                    //instance, mdl->remainingCapacity());
        }
        
        /* REMOVED - not currently used by Newmar
        if (mdl->relativeCapacity() != OUT_OF_RANGE_DATA)
            batteryMeter->setRelativeCapacity(mdl->relativeCapacity());
        */
        int16_t ripple = mdl->rmsRipple();
        if (ripple != OUT_OF_RANGE_DATA) {
            // printf("BatteryView::updateView ripple = %d\n", ripple);
            batteryMeter->setRMSRipple(static_cast<float_t>(ripple));
        }

        // status 4
        /* REMOVED - not currently used by Newmar
        if (mdl->chargeState() != UNDEFINED_CHARGE_STATE) 
            batteryMeter->setDesiredChargeState(mdl->chargeState());
        if ((mdl->desiredVoltage() != OUT_OF_RANGE_DATA) && (mdl->desiredVoltage() >= ZERO_VOLT_DEGREE_F) && (mdl->desiredVoltage() <= TWO_FORTY_VOLT_DEGREE_F))
            batteryMeter->setDesiredDCVoltage(mdl->desiredVoltage());
        int16_t dcCurrent = mdl->desiredCurrent();
        if (dcCurrent != OUT_OF_RANGE_DATA) { // && (dcCurrent >= ZERO_AMP_DEGREE_F) && (dcCurrent <= SIXTY_AMP_DEGREE_F))
            batteryMeter->setDesiredDCCurrent(static_cast<float_t>(dcCurrent));
        }
        if (mdl->type() != VENDOR_DEFINED)
           batteryMeter->setBatteryType(mdl->type());
        */

        PacketQueue::clearLastPacketReceiveTime();
        updated = true; 
    }
    // printf("DoorLockView::updateView completed \n"); 
    return updated;
}

BatteryView::BatteryView(GenericDevice* model, const char* spanDevName) : SpanView(model) {
    BatteryView::initialize();

}

 void BatteryView::createBatteryView(GenericDevice* model, const char* spanDevName) {
    BatteryView* vw = new BatteryView(model, spanDevName);

    // BatteryView::createCategory();

    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(spanDevName);
    BatteryView::BatteryState* meter = new BatteryView::BatteryState();
    if (model != nullptr) {
        meter->setDCInstance((DC_SOURCE_INSTANCE_DEFINITION)(model->index()));
        uint8_t* rawData = ((Battery*) model)->getCurrentData();
        meter->setPriority((DC_SOURCE_PRIORITY_DEFINTION)rawData[DC_SOURCE_PRIORITY_INDEX]);
    }
    vw->setBatteryMeter(meter);


    
    if (vw != nullptr)
        printf("BatteryView::createBatteryView: tmp created successfully\n");
    else
        printf("BatteryView::createBatteryView: tmp creation failed\n");   
    printf("BatteryView::createBatteryView completed\n");
 }

 #endif // ifdef HOME_KIT_2