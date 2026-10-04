#pragma once

#include <Arduino.h>
#include "CanFrameTypes.h"
#include "DGN.h"

class GenericDevice;

class BridgeDiagnostics {
public:
    static void registerDevice(const GenericDevice* device, const char* type, const char* name,
                               uint8_t rvcIndex, uint8_t sourceAddress);
    static void observeFrame(const CAN_frame_t& frame);
    static void observeDevice(const GenericDevice* device, RVC_DGN dgn, uint8_t sourceAddress, bool handled);
    static void observeUnmapped(RVC_DGN dgn, uint8_t sourceAddress, uint8_t instanceIndex,
                                bool instanceVerified, const uint8_t* data, uint8_t dataLength);
    static void observeTransmit(bool accepted);
    static void observeQueueDrop();
    static String reportJson();

private:
    struct DeviceActivity {
        const GenericDevice* model = nullptr;
        char type[33] = {};
        char name[65] = {};
        uint8_t rvcIndex = 0;
        uint8_t configuredSourceAddress = 0;
        uint64_t lastSeenMs = 0;
        uint64_t lastHandledMs = 0;
        uint32_t receivedCount = 0;
        uint32_t handledCount = 0;
        RVC_DGN lastDgn = ERROR;
        uint8_t lastSourceAddress = 0;
    };

    struct UnmappedActivity {
        uint32_t dgn = 0;
        uint8_t sourceAddress = 0;
        uint8_t instanceIndex = 0;
        bool instanceVerified = false;
        uint8_t sample[8] = {};
        uint8_t sampleLength = 0;
        bool lastSampleAllFf = false;
        uint32_t hitCount = 0;
        uint64_t lastSeenMs = 0;
    };

    static constexpr size_t MAX_DEVICES = 150;
    static constexpr size_t MAX_UNMAPPED = 64;
    DeviceActivity devices_[MAX_DEVICES];
    size_t deviceCount_ = 0;
    UnmappedActivity unmapped_[MAX_UNMAPPED];
    size_t unmappedCount_ = 0;
    uint32_t unmappedOverflow_ = 0;
    uint64_t receivedCount_ = 0;
    uint64_t extendedCount_ = 0;
    uint64_t remoteCount_ = 0;
    uint64_t lastReceivedMs_ = 0;
    uint64_t transmitAccepted_ = 0;
    uint64_t transmitRejected_ = 0;
    uint64_t queueDrops_ = 0;
    uint64_t untrackedDeviceFrames_ = 0;
    uint32_t untrackedConfiguredDevices_ = 0;

    BridgeDiagnostics() = default;
    BridgeDiagnostics(const BridgeDiagnostics&) = delete;
    BridgeDiagnostics& operator=(const BridgeDiagnostics&) = delete;
    BridgeDiagnostics(BridgeDiagnostics&&) = delete;
    BridgeDiagnostics& operator=(BridgeDiagnostics&&) = delete;
    ~BridgeDiagnostics() = default;

    static BridgeDiagnostics& instance();
    static uint64_t nowMs();
    DeviceActivity* findActivity(const GenericDevice* device);
    UnmappedActivity* findUnmapped(uint32_t dgn, uint8_t sourceAddress, uint8_t instanceIndex);
};