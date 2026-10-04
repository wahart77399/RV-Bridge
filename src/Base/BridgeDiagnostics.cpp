#include "BridgeDiagnostics.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <driver/twai.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <cstring>

namespace {
    constexpr size_t MAX_DIAGNOSTICS_JSON_BYTES = 32768;
    constexpr uint32_t MIN_FREE_HEAP_FOR_JSON = 24576;
}

BridgeDiagnostics& BridgeDiagnostics::instance() {
    static BridgeDiagnostics diagnostics;
    return diagnostics;
}

uint64_t BridgeDiagnostics::nowMs() {
    return static_cast<uint64_t>(esp_timer_get_time()) / 1000;
}

BridgeDiagnostics::DeviceActivity* BridgeDiagnostics::findActivity(const GenericDevice* device) {
    DeviceActivity* activity = nullptr;
    if (device != nullptr) {
        for (size_t index = 0; index < deviceCount_; ++index) {
            if (devices_[index].model == device) {
                activity = &devices_[index];
                break;
            }
        }
    }
    return activity;
}

void BridgeDiagnostics::registerDevice(const GenericDevice* device, const char* type, const char* name,
                                      uint8_t rvcIndex, uint8_t sourceAddress) {
    if (device != nullptr) {
        BridgeDiagnostics& diagnostics = instance();
        DeviceActivity* activity = diagnostics.findActivity(device);
        if (activity == nullptr && diagnostics.deviceCount_ < MAX_DEVICES) {
            activity = &diagnostics.devices_[diagnostics.deviceCount_++];
            activity->model = device;
        }
        if (activity != nullptr) {
            std::strncpy(activity->type, type != nullptr ? type : "", sizeof(activity->type) - 1);
            activity->type[sizeof(activity->type) - 1] = '\0';
            std::strncpy(activity->name, name != nullptr ? name : "", sizeof(activity->name) - 1);
            activity->name[sizeof(activity->name) - 1] = '\0';
            activity->rvcIndex = rvcIndex;
            activity->configuredSourceAddress = sourceAddress;
        } else if (diagnostics.untrackedConfiguredDevices_ < UINT32_MAX) {
            ++diagnostics.untrackedConfiguredDevices_;
        }
    }
}

void BridgeDiagnostics::observeFrame(const CAN_frame_t& frame) {
    BridgeDiagnostics& diagnostics = instance();
    ++diagnostics.receivedCount_;
    diagnostics.lastReceivedMs_ = nowMs();
    if (frame.FIR.B.FF == CAN_frame_ext) ++diagnostics.extendedCount_;
    if (frame.FIR.B.RTR == CAN_RTR) ++diagnostics.remoteCount_;
}

void BridgeDiagnostics::observeDevice(const GenericDevice* device, RVC_DGN dgn, uint8_t sourceAddress, bool handled) {
    if (device != nullptr) {
        BridgeDiagnostics& diagnostics = instance();
        DeviceActivity* activity = diagnostics.findActivity(device);
        if (activity == nullptr) {
            ++diagnostics.untrackedDeviceFrames_;
        } else {
            activity->lastSeenMs = nowMs();
            activity->lastDgn = dgn;
            activity->lastSourceAddress = sourceAddress;
            if (activity->receivedCount < UINT32_MAX) ++activity->receivedCount;
            if (handled) {
                activity->lastHandledMs = activity->lastSeenMs;
                if (activity->handledCount < UINT32_MAX) ++activity->handledCount;
            }
        }
    }
}

void BridgeDiagnostics::observeTransmit(bool accepted) {
    BridgeDiagnostics& diagnostics = instance();
    if (accepted) ++diagnostics.transmitAccepted_;
    else ++diagnostics.transmitRejected_;
}

void BridgeDiagnostics::observeQueueDrop() {
    ++instance().queueDrops_;
}

String BridgeDiagnostics::reportJson() {
    const BridgeDiagnostics& diagnostics = instance();
    uint64_t now = nowMs();
    JsonDocument document;
    document["schemaVersion"] = 1;
    document["uptimeMs"] = now;
    document["resetReason"] = static_cast<int>(esp_reset_reason());
    document["freeHeapBytes"] = ESP.getFreeHeap();
    document["minimumFreeHeapBytes"] = ESP.getMinFreeHeap();
    document["wifiConnected"] = WiFi.status() == WL_CONNECTED;
    if (WiFi.status() == WL_CONNECTED) document["wifiRssiDbm"] = WiFi.RSSI();
    JsonObject bus = document["bus"].to<JsonObject>();
    bus["receivedFrames"] = diagnostics.receivedCount_;
    bus["extendedFrames"] = diagnostics.extendedCount_;
    bus["remoteFrames"] = diagnostics.remoteCount_;
    if (diagnostics.receivedCount_ > 0) bus["lastFrameAgeMs"] = now - diagnostics.lastReceivedMs_;
    else bus["lastFrameAgeMs"] = nullptr;
    bus["averageFramesPerSecondSinceBoot"] = now > 0 ? diagnostics.receivedCount_ * 1000.0 / now : 0.0;
    bus["transmitAcceptedByDriver"] = diagnostics.transmitAccepted_;
    bus["transmitRejectedByDriver"] = diagnostics.transmitRejected_;
    bus["softwareTransmitQueueDrops"] = diagnostics.queueDrops_;
    bus["untrackedDeviceFrames"] = diagnostics.untrackedDeviceFrames_;
    bus["untrackedConfiguredDevices"] = diagnostics.untrackedConfiguredDevices_;
    document["handledFramesMeaning"] = "Handler returned true; not proof of valid status data or hardware acknowledgement";
    twai_status_info_t status = {};
    bool available = twai_get_status_info(&status) == ESP_OK;
    bus["driverStatusAvailable"] = available;
    if (available) {
        bus["driverState"] = static_cast<int>(status.state);
        bus["busOff"] = status.state == TWAI_STATE_BUS_OFF;
        bus["txErrorCounter"] = status.tx_error_counter;
        bus["rxErrorCounter"] = status.rx_error_counter;
        bus["driverTransmitFailures"] = status.tx_failed_count;
        bus["receiveMissedCount"] = status.rx_missed_count;
        bus["receiveOverrunCount"] = status.rx_overrun_count;
        bus["arbitrationLostCount"] = status.arb_lost_count;
        bus["busErrorCount"] = status.bus_error_count;
    }
    JsonArray devices = document["configuredDevices"].to<JsonArray>();
    for (size_t index = 0; index < diagnostics.deviceCount_; ++index) {
        const DeviceActivity& activity = diagnostics.devices_[index];
        JsonObject row = devices.add<JsonObject>();
        row["type"] = activity.type;
        row["name"] = activity.name;
        row["rvcIndex"] = activity.rvcIndex;
        row["sourceAddress"] = activity.configuredSourceAddress;
        row["observed"] = activity.receivedCount > 0;
        if (activity.receivedCount > 0) {
            row["lastSourceAddress"] = activity.lastSourceAddress;
            row["lastDgn"] = static_cast<uint32_t>(activity.lastDgn);
            row["lastFrameAgeMs"] = now - activity.lastSeenMs;
        } else {
            row["lastSourceAddress"] = nullptr;
            row["lastDgn"] = nullptr;
            row["lastFrameAgeMs"] = nullptr;
        }
        row["receivedFrames"] = activity.receivedCount;
        row["handledFrames"] = activity.handledCount;
        if (activity.handledCount > 0) row["lastHandledFrameAgeMs"] = now - activity.lastHandledMs;
        else row["lastHandledFrameAgeMs"] = nullptr;
    }
    String json;
    size_t expected = 0;
    bool canSerialize = !document.overflowed();
    if (canSerialize) {
        expected = measureJson(document);
        canSerialize = expected > 0 && expected <= MAX_DIAGNOSTICS_JSON_BYTES &&
                       ESP.getFreeHeap() >= expected + MIN_FREE_HEAP_FOR_JSON;
    }
    if (canSerialize && json.reserve(expected)) {
        if (serializeJson(document, json) != expected) json = "";
    }
    return json;
}