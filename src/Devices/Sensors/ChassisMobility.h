#pragma once

#include "Arduino.h"
#include "GenericDevice.h"
#include "DGN.h"
#include "Packet.h"
#include "debug.h"

constexpr uint8_t DEFAULT_CHASSIS_INDEX = 0;
constexpr uint8_t DEFAULT_CHASSIS_SOURCE_ADDRESS = 148;

class ChassisMobilityView;

class ChassisMobility : public GenericDevice {
private:
    friend class ChassisMobilityView;

    static constexpr uint8_t BRAKE_BYTE_INDEX = 4;
    static constexpr uint8_t BRAKE_MASK = 0x01;
    static constexpr uint8_t BRAKE_ENGAGED = 0x01;
    static constexpr uint8_t BRAKE_RELEASED = 0x00;
    static constexpr uint8_t INVALID_BYTE = 0xFF;
    static constexpr uint8_t SPEED_LSB_INDEX = 2;
    static constexpr uint8_t SPEED_MSB_INDEX = 3;
    static constexpr uint16_t SPEED_ERROR = 0xFFFE;
    static constexpr uint16_t SPEED_UNKNOWN = 0xFFFF;

    static ChassisMobility* instance_;
    bool statusReceived_;
    uint16_t vehicleSpeedRaw_;
    char statusDetail_[128] = {};

    ChassisMobility();
    explicit ChassisMobility(uint8_t sourceAddress);

    ChassisMobility(const ChassisMobility&) = delete;
    ChassisMobility& operator=(const ChassisMobility&) = delete;
    ChassisMobility(ChassisMobility&&) = delete;
    ChassisMobility& operator=(ChassisMobility&&) = delete;

    bool statusReceived() const;
    void statusReceived(bool value);
    bool isBrakeEngaged() const;
    bool isMoving() const;

    void applyStatus(const uint8_t* data);

protected:
    virtual void setData(RVC_DGN dgn, uint8_t* data) override;
    virtual CAN_frame_t* buildCommand(RVC_DGN dgn) override;

public:
    static ChassisMobility* getInstance();

    virtual ~ChassisMobility() override;

    bool hasValidStatus() const;
    static bool isParked();

    virtual boolean executeCommand(RVC_DGN dgn,
                                   const uint8_t* data = nullptr,
                                   uint8_t sourceAddress = SOURCE_ADDRESS) override;
};




