#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>
#include "Arduino.h"
#include "DGN.h"
#include "power-constants.h"
#include "ats-constants.h"
#include "battery-constants.h"
#include "charger-states.h"
#include "inverter-states.h"

constexpr uint8_t MAX_LINES = 3;
constexpr uint8_t NUMIO = 2;
constexpr uint8_t OUTPUT_LINE = 0;
constexpr uint8_t INPUT_LINE = 1;
constexpr uint8_t NO_LINE = 0xFF;
constexpr uint8_t DATA_SIZE = 8;
constexpr uint8_t INVALID_DATA = 0xFF;
constexpr float AAC_LOWER_LIMIT = -81;
constexpr float AAC_UPPER_LIMIT = 81;
using boolean = bool;

class PowerSensor {
public:
    uint8_t readings_[MAX_LINES][NUMIO][DATA_SIZE];
    uint16_t lastValidVolts_[MAX_LINES][NUMIO] = {};
    int16_t lastValidAmps_[MAX_LINES][NUMIO] = {};
    uint32_t voltReadingElapsedTime_[MAX_LINES][NUMIO] = {};
    uint32_t ampReadingElapsedTime_[MAX_LINES][NUMIO] = {};
    uint8_t buffer_[DATA_SIZE] = {};
    PowerSensor() { std::memset(readings_, 0xFF, sizeof(readings_)); }
    PowerSensor(uint8_t, uint8_t) : PowerSensor() {}
    virtual ~PowerSensor() = default;
    uint8_t* dataBuffer() { return buffer_; }
    static bool isValidSlot(uint8_t line, uint8_t io) { return line < MAX_LINES && io < NUMIO; }
    uint16_t lastValidVolts(uint8_t line, uint8_t io) const { return lastValidVolts_[line][io]; }
    void lastValidVolts(uint8_t line, uint8_t io, uint16_t value) { lastValidVolts_[line][io] = value; }
    int16_t lastValidAmps(uint8_t line, uint8_t io) const { return lastValidAmps_[line][io]; }
    void lastValidAmps(uint8_t line, uint8_t io, int16_t value) { lastValidAmps_[line][io] = value; }
    static uint16_t getLilEndian(uint8_t low, uint8_t high) { return low | (uint16_t(high) << 8); }
    virtual uint8_t lineOf(RVC_DGN, const uint8_t*) const { return 0; }
    virtual uint8_t ioOf(RVC_DGN, const uint8_t*) const { return INPUT_LINE; }
    uint16_t getACPointValue(const uint8_t*, AC_POINT_DATA_INDECES, AC_POINT_DATA_INDECES) const;
    uint16_t validateVolts(uint8_t, uint8_t, float);
    int16_t validateAmps(uint8_t, uint8_t, float);
    virtual void setData(RVC_DGN, uint8_t*);
    void clearReadings();
    uint16_t rmsVoltage(uint8_t line = 0, uint8_t io = INPUT_LINE);
    int16_t rmsCurrent(uint8_t line = 0, uint8_t io = INPUT_LINE);
    boolean isOpenGroundFault(uint8_t line = 0) const;
    boolean isOpenNeutralFault(uint8_t line = 0) const;
    boolean isReversePolarityFault(uint8_t line = 0) const;
    boolean isGroundCurrentFault(uint8_t line = 0) const;
};

class Charger : public PowerSensor {
public:
#include "charger-constants.inc"
    uint8_t statusData_[DATA_SIZE];
    uint8_t status2Data_[DATA_SIZE];
    uint8_t configurationData_[DATA_SIZE];
    uint8_t configuration2Data_[DATA_SIZE];
    uint8_t configuration3Data_[DATA_SIZE];
    uint8_t configuration4Data_[DATA_SIZE];
    Charger(uint8_t address = 0, uint8_t instance = 1);
    uint8_t* statusData() { return statusData_; }
    const uint8_t* statusData() const { return statusData_; }
    uint8_t* status2Data() { return status2Data_; }
    const uint8_t* status2Data() const { return status2Data_; }
    void copyBuffer(const uint8_t*, uint8_t*);
    void setData(RVC_DGN, uint8_t*) override;
    static const char* operatingStateName(uint8_t);
    uint8_t rawOperatingStateByte() const;
    ChargerOperatingState operatingState() const;
    uint16_t rawMeasuredChargeVoltage() const;
    float measuredChargeVoltage() const;
    float rawMeasuredChargeCurrent() const;
    float measuredChargeCurrent() const;
    uint8_t lineOf(RVC_DGN, const uint8_t*) const override;
    uint8_t ioOf(RVC_DGN, const uint8_t*) const override;
};

class Inverter : public PowerSensor {
public:
    static const char* statusName(uint8_t);
    uint8_t lineOf(RVC_DGN, const uint8_t*) const override;
    uint8_t ioOf(RVC_DGN, const uint8_t*) const override;
};

class AutomaticTransferSwitch : public PowerSensor {
public:
    static constexpr uint8_t ATS_BYTE_0 = 0;
    static constexpr uint8_t ATS_STATUS_INDEX_MASK = 0x07;
    static constexpr uint8_t SOURCE_COUNT = 7;
    static constexpr uint8_t LEG_COUNT = 2;
    uint8_t selectedSource_ = 0xFF;
    uint8_t sourceReadings_[SOURCE_COUNT][LEG_COUNT][NUMIO][DATA_SIZE] = {};
    bool sourceReadingReceived_[SOURCE_COUNT][LEG_COUNT][NUMIO] = {};
    void setData(RVC_DGN, uint8_t*) override;
#include "ats-methods.inc"
};

constexpr uint16_t INVALID_TANK_SIZE = 0xFFFF;
constexpr uint16_t MAX_RVC_TANK_VALUE = 65530;
constexpr uint8_t OUT_OF_RANGE_DATA = 255;
constexpr uint8_t MAX_PERCENT = 100;
constexpr uint8_t MAX_RVC_PERCENT = 250;
constexpr float RVC_PERCENT_PRECISION = 0.5f;
#define RV_PRINTF(...) ((void)0)

class Battery {
public:
    mutable uint8_t source1[8];
    mutable uint8_t source2[8];
    mutable uint8_t source3[8];
    Battery() {
        std::memset(source1, 0xFF, sizeof(source1));
        std::memset(source2, 0xFF, sizeof(source2));
        std::memset(source3, 0xFF, sizeof(source3));
    }
    uint8_t* getSource1Data() const { return source1; }
    uint8_t* getSource2Data() const { return source2; }
    uint8_t* getSource3Data() const { return source3; }
    static uint16_t getLilEndian(uint8_t low, uint8_t high) { return low | (uint16_t(high) << 8); }
#include "battery-methods.inc"
};

struct BatteryView {
    struct BatteryState {
        struct Reading {
            float value = 0;
            void setVal(float next) { value = next; }
        } dcVoltage, rmsRipple;
#include "battery-view-methods.inc"
    };
};

class Tanks {
public:
    enum : uint8_t {
        FRESH_WATER_INSTANCE = 0,
        TANKS_RELATIVE_LEVEL = 1, TANKS_RESOLUTION = 2,
        TANKS_ABSOLUTE_LEVEL_MSB = 3, TANKS_ABSOLUTE_LEVEL_LSB = 4,
        TANKS_SIZE_MSB = 5, TANKS_SIZE_LSB = 6
    };
    mutable uint8_t buffer_[8];
    uint16_t tankSize = INVALID_TANK_SIZE;
    Tanks() { std::memset(buffer_, 0xFF, sizeof(buffer_)); }
    uint8_t* getCurrentData() const { return buffer_; }
    static uint16_t getLilEndian(uint8_t low, uint8_t high) { return low | (uint16_t(high) << 8); }
    uint8_t levelPercent() const;
#include "tank-methods.inc"
};

class ChassisMobility {
public:
#include "chassis-constants.inc"
    inline static ChassisMobility* instance_ = nullptr;
    bool statusReceived_ = false;
    uint16_t vehicleSpeedRaw_ = SPEED_UNKNOWN;
    char statusDetail_[128] = {};
    mutable uint8_t buffer_[8] = {};
    uint8_t* getCurrentData() const { return buffer_; }
    bool statusReceived() const;
    void statusReceived(bool);
    bool hasValidStatus() const;
    bool isBrakeEngaged() const;
    bool isMoving() const;
    static bool isParked();
    void applyStatus(const uint8_t*);
};

constexpr uint8_t AWNING_STATUS_POSITION_INDEX = 2;
constexpr float COVER_PERCENT_PRECISION = 0.5f;
constexpr uint8_t COVER_MAX_PERCENT = 100;
constexpr uint8_t AWNING_FULLY_EXTENDED_PCT = 100;
constexpr uint8_t AWNING_FULLY_RETRACTED_PCT = 0;
constexpr uint8_t SHADES_FULLY_CLOSED_PCT = 100;
constexpr uint8_t SHADES_FULLY_OPEN_PCT = 0;
enum class CoverKind { Awning, Shade };
struct CoverDevice {
    unsigned stops = 0;
    void stop() { ++stops; }
};

struct CoverTiming {
    uint16_t extendSec = 40;
    uint16_t retractSec = 45;
};
struct CoverTimingPair {
    String key;
    CoverTiming timing;
};
struct CoachSpec {
    std::vector<CoverTimingPair> coverTimings;
#include "coach-methods.inc"
};

struct TestCharacteristic {
    uint8_t value = 0;
    uint8_t requested = 0;
    bool pending = false;
    bool writtenWhilePending = false;
    void setVal(uint8_t next) { writtenWhilePending |= pending; value = next; }
    bool updated() const { return pending; }
    template <typename Value> Value getVal() const { return static_cast<Value>(value); }
    template <typename Value> Value getNewVal() const { return static_cast<Value>(requested); }
};

class CoverView {
public:
    CoverKind kind_ = CoverKind::Shade;
    CoverKind kind() const { return kind_; }
    bool canOperate() const;
    void dontUpdateTheView() {}
    void updateTheView() {}
    struct CoverController {
        static constexpr uint8_t POSITION_DECREASING = 0;
        static constexpr uint8_t POSITION_INCREASING = 1;
        static constexpr uint8_t POSITION_STOPPED = 2;
        float currentPos_ = 0;
        CoverDevice* model_ = nullptr;
        CoverView* view_ = nullptr;
        float timeExtend_ = 9000;
        float timeRetract_ = 9000;
        float targetPos_ = 0;
        float startPos_ = 0;
        uint32_t startTime_ = 0;
        uint32_t travelTime_ = 0;
        bool moving_ = false;
        bool extendCmd_ = false;
        bool retractCmd_ = false;
        bool stopCmd_ = false;
        TestCharacteristic current;
        TestCharacteristic target;
        TestCharacteristic motion;
        TestCharacteristic* currentState_ = &current;
        TestCharacteristic* targetState_ = &target;
        TestCharacteristic* positionState_ = &motion;
        void publishAwningStatus(const uint8_t*);
        bool isMoving() const { return moving_; }
        bool isReadyToExtend() const { return extendCmd_; }
        void clearCommands() { extendCmd_ = false; retractCmd_ = false; stopCmd_ = false; }
        void readyToExtend() { extendCmd_ = true; retractCmd_ = false; stopCmd_ = false; }
        void readyToRetract() { extendCmd_ = false; retractCmd_ = true; stopCmd_ = false; }
        void readyToStop() { extendCmd_ = false; retractCmd_ = false; stopCmd_ = true; }
        void setStopCmd(bool value) { stopCmd_ = value; }
        void moveTo(float);
        void requestPosition(float);
        void stopMoving();
        bool isReadyToStop();
        bool update();
        void requestFullExtend();
        void requestFullRetract();
    };
    struct CoverExtendRetractController {
        CoverView* view_ = nullptr;
        CoverController* coverCtrl_ = nullptr;
        TestCharacteristic out;
        TestCharacteristic* out_ = &out;
        bool update();
    };
};

constexpr uint8_t WATER_PUMP_INDEX = 0;
constexpr uint8_t DEFAULT_CHASSIS_INDEX = 0;
enum class GeneratorInstance : uint8_t {
    GENERATOR_INSTANCE_0_INVALID = 0,
    GENERATOR_INSTANCE_11_INVALID = 11
};
struct Generator {
    static constexpr uint8_t GENERATOR_BYTE_0 = 0;
    static constexpr uint8_t GENERATOR_OUTPUT_INDEX_MASK = 0x0F;
};
struct Packet {
    static uint8_t getIndex(uint8_t* data) { return data[0]; }
};
struct DeviceFactory {
    bool instanceFromData(RVC_DGN, uint8_t*, uint8_t&);
};

float tempCfromTempF(float value) { return (value - 32.0f) / 1.8f; }
struct FloatCharacteristic {
    float value = 0;
    void setVal(float next) { value = next; }
};
struct PowerSensorView {
    bool updateView() { return true; }
};
struct ChargerView : PowerSensorView {
    Charger* model_ = nullptr;
    FloatCharacteristic* measuredVoltChar_ = nullptr;
    FloatCharacteristic* measuredCurrChar_ = nullptr;
    FloatCharacteristic* stateChar_ = nullptr;
    bool needsUpdate = true;
    bool isNeedToUpdateView() { return needsUpdate; }
    void dontUpdateTheView() { needsUpdate = false; }
    bool updateView();
};

struct BridgeDiagnostics {
    inline static const void* detailedDevice = nullptr;
    inline static const char* detail = nullptr;
    static void observeDeviceDetail(const void* device, const char* text) {
        detailedDevice = device;
        detail = text;
    }
};

#include "rvc-production.inc"

int main() {
    Charger charger;
    Inverter inverter;
    uint8_t line1[8] = {0x01, 0x60, 0x09, 0x2C, 0x7E, 0, 0, 0x01};
    uint8_t line2[8] = {0x11, 0xC0, 0x12, 0x00, 0x7D, 0, 0, 0x04};
    charger.setData(CHARGER_AC_STATUS_1, line1);
    charger.setData(CHARGER_AC_STATUS_1, line2);
    assert(charger.rmsVoltage(0) == 120);
    assert(charger.rmsVoltage(1) == 240);
    assert(charger.rmsCurrent(0) == 15);
    assert(charger.rmsCurrent(1) == 0);
    assert(charger.isOpenGroundFault(0));
    assert(!charger.isOpenGroundFault(1));
    assert(!charger.isOpenNeutralFault(0));
    assert(charger.isOpenNeutralFault(1));
    std::cout << "PASS: charger lines retain independent voltages, currents, and faults\n";

    uint8_t output[8] = {0x41, 0xC0, 0x12, 0x00, 0x7D, 0, 0, 0};
    inverter.setData(INVERTER_AC_STATUS_1, line1);
    inverter.setData(INVERTER_AC_STATUS_1, output);
    assert(inverter.rmsVoltage(0, INPUT_LINE) == 120);
    assert(inverter.rmsVoltage(0, OUTPUT_LINE) == 240);
    assert(charger.ioOf(CHARGER_AC_STATUS_1, output) == OUTPUT_LINE);
    std::cout << "PASS: inverter input and output use distinct storage slots\n";

    uint8_t reserved[8] = {0x21, 0, 0, 0, 0x7D, 0, 0, 0};
    assert(charger.lineOf(CHARGER_AC_STATUS_1, reserved) == NO_LINE);
    assert(inverter.lineOf(INVERTER_AC_STATUS_1, reserved) == NO_LINE);
    reserved[0] = 0x81;
    assert(charger.ioOf(CHARGER_AC_STATUS_1, reserved) == NUMIO);
    assert(inverter.ioOf(INVERTER_AC_STATUS_1, reserved) == NUMIO);
    assert(charger.lineOf(CHARGER_STATUS, line1) == NO_LINE);
    assert(inverter.lineOf(INVERTER_STATUS, line1) == NO_LINE);
    assert(charger.lineOf(CHARGER_AC_STATUS_1, nullptr) == NO_LINE);
    std::cout << "PASS: reserved AC fields and non-AC status cannot select a reading slot\n";

    line1[1] = 0;
    line1[2] = 0;
    line1[3] = 0;
    line1[4] = 0x7D;
    charger.setData(CHARGER_AC_STATUS_1, line1);
    assert(charger.rmsVoltage(0) == 0);
    assert(charger.rmsCurrent(0) == 0);
    line1[1] = 0xFF;
    line1[2] = 0xFF;
    charger.setData(CHARGER_AC_STATUS_1, line1);
    assert(charger.rmsVoltage(0) == 0);
    assert(!charger.isOpenGroundFault(MAX_LINES));
    std::cout << "PASS: valid zero readings clear stale data and unavailable voltage is rejected\n";

    Tanks tank;
    assert(tank.levelPercent() == OUT_OF_RANGE_DATA);
    tank.buffer_[1] = 3;
    tank.buffer_[2] = 4;
    assert(tank.levelPercent() == 75);
    tank.buffer_[1] = 0;
    assert(tank.levelPercent() == 0);
    tank.buffer_[1] = 4;
    assert(tank.levelPercent() == 100);
    std::cout << "PASS: relative tank fractions work without absolute capacity\n";

    tank.buffer_[1] = 255;
    tank.buffer_[2] = 0;
    tank.buffer_[3] = 0x58;
    tank.buffer_[4] = 0x02;
    assert(tank.level() == 600);
    assert(tank.levelPercent() == OUT_OF_RANGE_DATA);
    tank.tankSize = 1000;
    assert(tank.size() == 1000);
    assert(tank.levelPercent() == 60);
    tank.buffer_[5] = 0xB0;
    tank.buffer_[6] = 0x04;
    assert(tank.size() == 1200);
    assert(tank.levelPercent() == 50);
    tank.buffer_[3] = 0xFE;
    tank.buffer_[4] = 0xFF;
    assert(tank.levelPercent() == OUT_OF_RANGE_DATA);
    std::cout << "PASS: tank absolute fallback accepts over 500 L and rejects reserved data\n";

    ChassisMobility chassis;
    uint8_t chassisData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    assert(!chassis.hasValidStatus());
    assert(!chassis.isBrakeEngaged());
    chassis.applyStatus(chassisData);
    assert(chassis.hasValidStatus());
    assert(!chassis.isMoving());
    assert(!chassis.isBrakeEngaged());
    chassisData[2] = 1;
    chassis.applyStatus(chassisData);
    assert(chassis.vehicleSpeedRaw_ == 1);
    assert(chassis.isMoving());
    chassisData[2] = 0xFE;
    chassisData[3] = 0xFF;
    chassis.applyStatus(chassisData);
    assert(!chassis.hasValidStatus());
    assert(!chassis.isMoving());
    chassisData[2] = 0xFF;
    chassis.applyStatus(chassisData);
    assert(!chassis.hasValidStatus());
    for (uint8_t brake = 0; brake < 4; ++brake) {
        chassisData[4] = brake;
        chassis.applyStatus(chassisData);
        assert(chassis.isBrakeEngaged() == (brake == 1));
    }
    std::cout << "PASS: chassis little-endian speed, error values, and two-bit brake safety\n";

    CoverView::CoverController awning;
    uint8_t awningData[8] = {1, 1, 100, 0, 0, 0, 0, 0};
    awning.publishAwningStatus(awningData);
    assert(awning.current.value == 50);
    assert(awning.motion.value == awning.POSITION_INCREASING);
    awningData[1] = 2;
    awningData[2] = 200;
    awning.publishAwningStatus(awningData);
    assert(awning.current.value == 100);
    assert(awning.motion.value == awning.POSITION_DECREASING);
    awningData[1] = 0;
    awningData[2] = 0;
    awning.publishAwningStatus(awningData);
    assert(awning.current.value == 0);
    assert(awning.motion.value == awning.POSITION_STOPPED);
    awningData[1] = 255;
    awningData[2] = 255;
    awning.publishAwningStatus(awningData);
    assert(awning.current.value == 0);
    assert(awning.motion.value == awning.POSITION_STOPPED);
    std::cout << "PASS: awning half-percent position and motion ignore unavailable data\n";

    AutomaticTransferSwitch ats;
    uint8_t primary[8] = {0x01, 0x60, 0x09, 0x2C, 0x7E, 0, 0, 0x01};
    uint8_t secondary[8] = {0x11, 0xC0, 0x12, 0, 0x7D, 0, 0, 0};
    uint8_t sourceStatus[8] = {1, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ats.setData(ATS_AC_STATUS_1, primary);
    ats.setData(ATS_AC_STATUS_1, secondary);
    assert(ats.rmsVoltage() == 0);
    ats.setData(ATS_STATUS, sourceStatus);
    assert(ats.rmsVoltage() == 120);
    assert(ats.rmsCurrent() == 15);
    assert(ats.isOpenGroundFault());
    ats.setData(ATS_AC_STATUS_1, secondary);
    assert(ats.rmsVoltage() == 120);
    sourceStatus[1] = 1;
    ats.setData(ATS_STATUS, sourceStatus);
    assert(ats.rmsVoltage() == 240);
    assert(ats.rmsCurrent() == 0);
    assert(!ats.isOpenGroundFault());
    assert(ats.rmsVoltage(1) == 0);
    std::cout << "PASS: ATS inactive sources cannot overwrite selected-source readings or faults\n";
    sourceStatus[1] = 253;
    ats.setData(ATS_STATUS, sourceStatus);
    assert(ats.rmsVoltage() == 0);
    assert(ats.rmsCurrent() == 0);
    ats.setData(ATS_AC_STATUS_1, primary);
    assert(ats.rmsVoltage() == 0);
    sourceStatus[1] = 0;
    ats.setData(ATS_STATUS, sourceStatus);
    assert(ats.rmsCurrent() == 15);
    sourceStatus[1] = 2;
    ats.setData(ATS_STATUS, sourceStatus);
    assert(ats.rmsCurrent() == 0);
    assert(ats.rmsVoltage() == 0);
    std::cout << "PASS: ATS no-source and uncached-source transitions clear stale readings\n";

    DeviceFactory factory;
    uint8_t decoded = 99;
    uint8_t point[8] = {0};
    for (uint8_t instance = 0; instance < 16; ++instance) {
        point[0] = 0x50 | instance;
        bool expected = instance >= 1 && instance <= 13;
        assert(factory.instanceFromData(CHARGER_AC_STATUS_1, point, decoded) == expected);
        assert(factory.instanceFromData(INVERTER_AC_STATUS_1, point, decoded) == expected);
        if (expected) {
            assert(decoded == instance);
        }
    }
    point[0] = 200;
    assert(factory.instanceFromData(INVERTER_STATUS, point, decoded));
    assert(decoded == 200);
    assert(factory.instanceFromData(CHARGER_STATUS, point, decoded));
    assert(decoded == 200);
    point[0] = 0x99;
    assert(factory.instanceFromData(ATS_AC_STATUS_1, point, decoded));
    assert(decoded == 1);
    std::cout << "PASS: factory separates packed AC IDs from ordinary full-width status IDs\n";

    Battery battery;
    assert(battery.level() == OUT_OF_RANGE_DATA);
    assert(std::isnan(battery.directCurrentVoltage()));
    assert(std::isnan(battery.directCurrentAmperage()));
    battery.source2[4] = 0;
    assert(battery.level() == 0);
    battery.source2[4] = 200;
    assert(battery.level() == 100);
    battery.source2[4] = 250;
    assert(battery.level() == 125);
    for (unsigned rawSoc = 251; rawSoc <= 255; ++rawSoc) {
        battery.source2[4] = static_cast<uint8_t>(rawSoc);
        assert(battery.level() == OUT_OF_RANGE_DATA);
    }
    std::cout << "PASS: battery SOC reserved fields remain unavailable instead of zero or full\n";

    battery.source1[2] = 0xFD;
    battery.source1[3] = 0;
    BatteryView::BatteryState batteryView;
    batteryView.setDCVoltage(battery.directCurrentVoltage());
    assert(std::fabs(batteryView.dcVoltage.value - 12.65f) < 0.001f);
    battery.source1[2] = 0;
    assert(battery.directCurrentVoltage() == 0);
    for (int32_t milliamps : {0, 12500, -12500}) {
        uint32_t rawCurrent = static_cast<uint32_t>(static_cast<int64_t>(ADC_ZERO) + milliamps);
        for (uint8_t byteIndex = 0; byteIndex < 4; ++byteIndex) {
            battery.source1[4 + byteIndex] = static_cast<uint8_t>(rawCurrent >> (8 * byteIndex));
        }
        assert(std::fabs(battery.directCurrentAmperage() - milliamps * 0.001f) < 0.001f);
    }
    std::cout << "PASS: battery fractional voltage and signed 32-bit current retain their precision\n";

    Charger unseenCharger;
    FloatCharacteristic chargerVoltage;
    FloatCharacteristic chargerCurrent;
    FloatCharacteristic chargerState;
    chargerVoltage.value = 19;
    chargerCurrent.value = 20;
    chargerState.value = 21;
    ChargerView chargerView;
    chargerView.model_ = &unseenCharger;
    chargerView.measuredVoltChar_ = &chargerVoltage;
    chargerView.measuredCurrChar_ = &chargerCurrent;
    chargerView.stateChar_ = &chargerState;
    chargerView.updateView();
    assert(chargerVoltage.value == 19);
    assert(chargerCurrent.value == 20);
    assert(chargerState.value == 21);
    for (unsigned rawState = 0; rawState <= 10; ++rawState) {
        unseenCharger.statusData_[6] = static_cast<uint8_t>(rawState);
        chargerView.needsUpdate = true;
        chargerView.updateView();
        assert(std::fabs(chargerState.value - tempCfromTempF(rawState <= 7 ? rawState : 7)) < 0.001f);
    }
    std::cout << "PASS: charger startup telemetry and reserved operating states are not published\n";

    assert(battery.rmsRipple() == INVALID_RMS_RIPPLE);
    for (uint16_t rippleMillivolts : {0, 255, 40000, 65530}) {
        battery.source3[6] = static_cast<uint8_t>(rippleMillivolts);
        battery.source3[7] = static_cast<uint8_t>(rippleMillivolts >> 8);
        assert(battery.rmsRipple() == rippleMillivolts);
        batteryView.setRMSRipple(battery.rmsRipple());
        assert(std::fabs(batteryView.rmsRipple.value - rippleMillivolts / 1000.0f) < 0.001f);
    }
    for (unsigned reservedRipple = 65531; reservedRipple <= 65535; ++reservedRipple) {
        battery.source3[6] = static_cast<uint8_t>(reservedRipple);
        battery.source3[7] = static_cast<uint8_t>(reservedRipple >> 8);
        assert(battery.rmsRipple() == INVALID_RMS_RIPPLE);
    }
    std::cout << "PASS: battery ripple preserves unsigned millivolts and rejects reserved values\n";

    const char* chargerStates[] = {
        "Disabled", "Not Charging", "Bulk", "Absorption", "Overcharge", "Equalize",
        "Float", "Constant Voltage/Current"
    };
    const char* inverterStates[] = {
        "Disabled", "Inverting", "AC Pass-through", "Auxiliary Power Only",
        "Load Sense", "Waiting to Invert", "Generator Support"
    };
    for (uint8_t state = 0; state < 8; ++state) {
        assert(std::strcmp(Charger::operatingStateName(state), chargerStates[state]) == 0);
    }
    for (uint8_t state = 0; state < 7; ++state) {
        assert(std::strcmp(Inverter::statusName(state), inverterStates[state]) == 0);
    }
    assert(std::strcmp(Charger::operatingStateName(8), "Reserved") == 0);
    assert(std::strcmp(Inverter::statusName(7), "Reserved") == 0);
    assert(std::strcmp(Charger::operatingStateName(255), "Unknown") == 0);
    assert(std::strcmp(Inverter::statusName(255), "Unknown") == 0);
    std::cout << "PASS: charger and inverter states decode to readable text\n";

    uint8_t floatStatus[8] = {1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 6, 0xFF};
    charger.setData(CHARGER_STATUS, floatStatus);
    assert(BridgeDiagnostics::detailedDevice == &charger);
    assert(std::strcmp(BridgeDiagnostics::detail, "Float") == 0);
    charger.setData(CHARGER_AC_STATUS_1, line1);
    assert(std::strcmp(BridgeDiagnostics::detail, "Float") == 0);
    floatStatus[6] = 255;
    charger.setData(CHARGER_STATUS, floatStatus);
    assert(std::strcmp(BridgeDiagnostics::detail, "Unknown") == 0);
    std::cout << "PASS: charger diagnostics publish state text without AC packets overwriting it\n";

    CoverDevice shadeModel;
    CoverView shadeView;
    CoverView::CoverController shade;
    shade.model_ = &shadeModel;
    shade.view_ = &shadeView;
    shade.moveTo(100);
    assert(shade.travelTime_ == 9000);
    shade.moving_ = true;
    shade.startTime_ = 8999;
    assert(!shade.isReadyToStop());
    assert(shadeModel.stops == 0);
    shade.startTime_ = 9000;
    assert(shade.isReadyToStop());
    assert(shadeModel.stops == 1);
    assert(shade.current.value == 100);
    assert(shade.isReadyToStop());
    assert(shadeModel.stops == 1);
    shade.moveTo(0);
    assert(shade.travelTime_ == 9000);
    shade.moving_ = true;
    shade.startTime_ = 8999;
    assert(!shade.isReadyToStop());
    shade.startTime_ = 9000;
    assert(shade.isReadyToStop());
    assert(shadeModel.stops == 2);
    assert(shade.current.value == 0);
    std::cout << "PASS: full shade travel stops at 9 seconds in both directions, once\n";

    shade.moveTo(50);
    assert(shade.travelTime_ == 4500);
    shade.moving_ = true;
    shade.startTime_ = 4500;
    assert(shade.isReadyToStop());
    assert(shadeModel.stops == 3);
    shade.currentPos_ = 0;
    shade.timeExtend_ = 120000;
    shade.moveTo(100);
    assert(shade.travelTime_ == 120000);
    shade.moving_ = true;
    shade.startTime_ = 119999;
    assert(!shade.isReadyToStop());
    shade.startTime_ = 120000;
    assert(shade.isReadyToStop());
    assert(shadeModel.stops == 4);
    std::cout << "PASS: partial shade travel scales timing and long travel cannot overflow\n";

    CoachSpec coach;
    coach.coverTimings.push_back({"Living Room Night Shades", {9, 9}});
    CoverTiming selectedTiming;
    assert(coach.findCoverTiming("Living Room Night Shade", selectedTiming));
    assert(selectedTiming.extendSec == 9 && selectedTiming.retractSec == 9);
    assert(!coach.findCoverTiming("Bedroom Night Shade", selectedTiming));
    coach.coverTimings.push_back({"Living Room Night Shade", {13, 15}});
    assert(coach.findCoverTiming("Living Room Night Shade", selectedTiming));
    assert(selectedTiming.extendSec == 13 && selectedTiming.retractSec == 15);
    CoachSpec singularCoach;
    singularCoach.coverTimings.push_back({"Living Room Night Shade", {9, 9}});
    assert(singularCoach.findCoverTiming("Living Room Night Shades", selectedTiming));
    assert(selectedTiming.extendSec == 9 && selectedTiming.retractSec == 9);
    std::cout << "PASS: coach timing accepts Shade/Shades aliases and prioritizes exact matches\n";

    ChassisMobility parkedChassis;
    ChassisMobility::instance_ = &parkedChassis;
    uint8_t parkedStatus[8] = {0, 0, 0, 0, 1, 0, 0, 0};
    parkedChassis.applyStatus(parkedStatus);
    assert(std::strstr(BridgeDiagnostics::detail, "Park brake engaged; brake=1; payload=00 00 00 00 01 00 00 00") != nullptr);
    for (CoverKind kind : {CoverKind::Awning, CoverKind::Shade}) {
        CoverView view;
        view.kind_ = kind;
        CoverDevice model;
        CoverView::CoverController control;
        control.model_ = &model;
        control.view_ = &view;
        control.target.pending = true;
        control.target.requested = 50;
        assert(control.update());
        assert(control.targetPos_ == 50);
        assert(!control.target.writtenWhilePending);
        control.requestFullExtend();
        assert(!control.target.writtenWhilePending);
        assert(control.targetPos_ == 50);
        control.requestFullRetract();
        assert(!control.target.writtenWhilePending);
        assert(control.targetPos_ == 50);
    }
    std::cout << "PASS: parked cover writes preserve a pending TargetPosition during Out/In requests\n";

    for (CoverKind kind : {CoverKind::Awning, CoverKind::Shade}) {
        CoverView view;
        view.kind_ = kind;
        CoverDevice model;
        CoverView::CoverController control;
        control.model_ = &model;
        control.view_ = &view;
        CoverView::CoverExtendRetractController switchControl;
        switchControl.view_ = &view;
        switchControl.coverCtrl_ = &control;
        switchControl.out.pending = true;
        switchControl.out.requested = 1;
        parkedStatus[4] = 1;
        parkedChassis.applyStatus(parkedStatus);
        assert(switchControl.update());
        assert(control.target.value == 100 && control.targetPos_ == 100);
        switchControl.out.requested = 0;
        assert(switchControl.update());
        assert(control.target.value == 0 && control.targetPos_ == 0);
        control.target.pending = true;
        control.target.requested = 50;
        switchControl.out.requested = 1;
        assert(switchControl.update());
        assert(control.targetPos_ == 50);
        assert(!control.target.writtenWhilePending);
        assert(control.update());
        assert(control.targetPos_ == 50);
        for (uint8_t brakeStatus : {0, 2, 3, 255}) {
            parkedStatus[4] = brakeStatus;
            parkedChassis.applyStatus(parkedStatus);
            const char* expectedDetail = brakeStatus == 0 ? "Park brake released" : "Park brake unavailable";
            assert(std::strncmp(BridgeDiagnostics::detail, expectedDetail, std::strlen(expectedDetail)) == 0);
            control.clearCommands();
            const unsigned stopsBefore = model.stops;
                bool diagnosticBypass = false;
        #if defined(SMARTCOACH_AWNING_PARK_BYPASS) && SMARTCOACH_AWNING_PARK_BYPASS
                diagnosticBypass = kind == CoverKind::Awning;
        #endif
                assert(control.update() == diagnosticBypass);
                assert(switchControl.update() == diagnosticBypass);
                assert(control.extendCmd_ == diagnosticBypass && !control.retractCmd_);
            assert(model.stops == stopsBefore);
        }
        parkedChassis.statusReceived_ = false;
            bool diagnosticBypass = false;
        #if defined(SMARTCOACH_AWNING_PARK_BYPASS) && SMARTCOACH_AWNING_PARK_BYPASS
            diagnosticBypass = kind == CoverKind::Awning;
        #endif
            assert(control.update() == diagnosticBypass);
            assert(switchControl.update() == diagnosticBypass);
    }
    ChassisMobility::instance_ = nullptr;
    std::cout << "PASS: cover writes follow the park policy; diagnostic bypass affects awnings only\n";
}