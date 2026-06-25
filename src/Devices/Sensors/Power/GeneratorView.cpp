#include "RVConstants.h"
#ifdef HOME_KIT_2
#include "Generator.h"
#include "GeneratorView.h"
#include "PacketQueue.h"
// #include "ChassisMobility.h"

bool GeneratorView::updateView(void) {
        // 
    // printf("DoorLockView::updateView called\n");
    bool updated = false;
    uint8_t instance = indexOfModel();   
    uint8_t index = -1;
    Generator* mdl = (Generator* )getModel();
    if (mdl != nullptr)  {
        uint8_t* rawData = mdl->getCurrentData();
        double volt = mdl->rmsVoltage();
        // if (volt > 300.0) {
        //     volt = 125.0; 
        //    printf("GeneratorView::updateView - voltage was greater than 300, set it to 125\n");
        // }
        double amp = mdl->rmsCurrent();

        //if (amp > 200.0)
        //     amp = 122.0;
        boolean vFault = (mdl->isOpenNeutralFault() || mdl->isOpenGroundFault() || mdl->isReversePolarityFault());
        boolean aFault = mdl->isGroundCurrentFault();

        uint8_t line = mdl->line();
        if ((leg1VoltMeter != nullptr) && (leg2VoltMeter != nullptr) &&
            (leg1AmpMeter != nullptr) && (leg2AmpMeter != nullptr)) {
            if (line == Generator::GENERATOR_LINE_1_OUTPUT) {   
                leg1VoltMeter->setVoltage(volt);
                leg1AmpMeter->setLegCurrent(amp);
                // printf("GeneratorView::updateView: leg1 Voltage=%f, leg1 Current=%f\n", volt, amp);
            } else if (line == Generator::GENERATOR_LINE_2_OUTPUT) {
                leg2VoltMeter->setVoltage(volt);
                leg2AmpMeter->setLegCurrent(amp);
                // printf("GeneratorView::updateView: leg2 Voltage=%f, leg2 Current=%f\n", volt, amp);
            } else {
                leg1VoltMeter->setVoltage(volt);
                leg2VoltMeter->setVoltage(volt);
                leg1AmpMeter->setLegCurrent(amp);
                leg2AmpMeter->setLegCurrent(amp);
                printf("GeneratorView::updateView invalid line: leg1 Voltage=%f, leg1 Current=%f, leg2 Voltage=%f, leg2 Current=%f\n", volt, amp, volt, amp);
            }
        }
        PacketQueue::clearLastPacketReceiveTime();
        updated = true; 
    }
    // printf("DoorLockView::updateView completed \n"); 
    return updated;
 }

GeneratorView::GeneratorView(GenericDevice* model, const char* spanDevName)
    : SpanView(model),
      leg1VoltMeter(nullptr),
      leg2VoltMeter(nullptr),
      leg1AmpMeter(nullptr),
      leg2AmpMeter(nullptr),
      spanDeviceName(spanDevName) {

    printf("GeneratorView::GeneratorView: created view for %s\n", spanDeviceName);

}

 void GeneratorView::createGeneratorView(GenericDevice* model, const char* spanDevName) {
    GeneratorView* vw = new GeneratorView(model, spanDevName);

    // GeneratorView::createCategory();
    const char* vappend = " Volt Meter";
    const char* vappend1 = " 1";
    const char* vappend2 = " 2";
    const char* aappend = " Amp Meter";
    const char* aappend1 = " 1";
    const char* aappend2 = " 2";

    size_t buffer_size_1 = strlen(spanDevName) + strlen(vappend) + strlen(vappend1) + 1; 
    char* voltName = new char[buffer_size_1];
    strcpy(voltName, spanDevName);
    strcat(voltName, vappend);
    size_t buffer_size_2 = strlen(spanDevName) + strlen(aappend) + strlen(aappend1) + 1; 
    char* ampName = new char[buffer_size_2];
    strcpy(ampName, spanDevName);
    strcat(ampName, aappend);
    strcat(voltName, vappend1);
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    GeneratorView::LegVoltage* volt1 = new GeneratorView::LegVoltage(voltName); // vw, (Generator* )model, voltName);

    delete voltName;   
    voltName = new char[buffer_size_1];
    strcpy(voltName, spanDevName);
    strcat(voltName, vappend);
    strcat(voltName, vappend2);
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(voltName);
    GeneratorView::LegVoltage* volt2 = new GeneratorView::LegVoltage(voltName);
    vw->setLegVoltages(volt1, volt2);

    strcat(ampName, aappend1);
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(ampName);
    GeneratorView::LegAmperage* amp1 = new GeneratorView::LegAmperage(ampName);

    delete ampName;   
    ampName = new char[buffer_size_2];
    strcpy(ampName, spanDevName);
    strcat(ampName, aappend);
    strcat(ampName, aappend2);
    new SpanAccessory(); 
    new Service::AccessoryInformation(); 
    new Characteristic::Identify();
    new Characteristic::Name(ampName);
    GeneratorView::LegAmperage* amp2 = new GeneratorView::LegAmperage(ampName);
    vw->setLegAmperages(amp1, amp2);
    
    if (vw != nullptr)
        printf("GeneratorView::createGeneratorView: tmp created successfully\n");
    else
        printf("GeneratorView::createGeneratorView: tmp creation failed\n");   
    printf("GeneratorView::createGeneratorView completed\n");
 }

 #endif // ifdef HOME_KIT_2