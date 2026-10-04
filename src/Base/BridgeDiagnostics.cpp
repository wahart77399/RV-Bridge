#include "BridgeDiagnostics.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <driver/twai.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <cstring>

#include "LearnRecord.h"

namespace {
    constexpr size_t MAX_DIAGNOSTICS_JSON_BYTES = 32768;
    constexpr uint32_t MIN_FREE_HEAP_FOR_JSON = 24576;

    void appendPayloadHex(char* out, size_t outSize, const uint8_t* data, uint8_t length) {
        static const char digits[] = "0123456789ABCDEF";
        size_t written = 0;
        if (out != nullptr && outSize > 0) {
            out[0] = '\0';
            for (uint8_t index = 0; data != nullptr && index < length && written + 2 < outSize; ++index) {
                out[written++] = digits[data[index] >> 4];
                out[written++] = digits[data[index] & 0x0F];
            }
            out[written] = '\0';
        }
    }

    bool payloadAllFf(const uint8_t* data, uint8_t length) {
        bool allFf = data != nullptr && length > 0;
        for (uint8_t index = 0; allFf && index < length; ++index) {
            if (data[index] != 0xFF) allFf = false;
        }
        return allFf;
    }

    struct PgnDescription {
        uint32_t value;
        const char* protocol;
        const char* name;
        bool hasDestinationAddress;
        uint8_t destinationAddress;
    };

    PgnDescription describeUnknownPgn(uint32_t rawDgn) {
        PgnDescription description = {rawDgn, "Unknown", "", false, 0};
        uint8_t pduFormat = static_cast<uint8_t>((rawDgn >> 8) & 0xFF);
        bool pdu1 = pduFormat < 0xF0;
        uint32_t pgn = pdu1 ? rawDgn & 0x1FF00 : rawDgn;
        if (pdu1) {
            description.destinationAddress = static_cast<uint8_t>(rawDgn & 0xFF);
        }

        if (rawDgn == 0xFECA) {
            description.protocol = "J1939";
            description.name = "DM1";
        } else if (rawDgn == 0xFEF5) {
            description.protocol = "J1939";
            description.name = "AMBIENT_CONDITIONS_1";
        } else if (rawDgn == 0xFEFC) {
            description.protocol = "J1939";
            description.name = "DASH_DISPLAY";
        } else if (rawDgn == 0x1F809) {
            description.protocol = "NMEA 2000";
            description.name = "TIME_AND_DATE";
        } else if (pdu1 && pgn == 0xE800) {
            description.protocol = "J1939";
            description.name = "ACKNOWLEDGMENT";
            description.value = pgn;
            description.hasDestinationAddress = true;
        } else if (pdu1 && pgn == 0xEA00) {
            description.protocol = "J1939";
            description.name = "REQUEST";
            description.value = pgn;
            description.hasDestinationAddress = true;
        } else if (pdu1 && pgn == 0xEE00) {
            description.protocol = "J1939";
            description.name = "ADDRESS_CLAIMED";
            description.value = pgn;
            description.hasDestinationAddress = true;
        } else if (pdu1 && pgn == 0xEF00) {
            description.protocol = "J1939";
            description.name = "PROPRIETARY_A";
            description.value = pgn;
            description.hasDestinationAddress = true;
        } else if (!pdu1 && pduFormat == 0xFF && rawDgn <= 0x0FFFF) {
            description.protocol = "J1939";
            description.name = "PROPRIETARY_B";
        }

        return description;
    }
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

BridgeDiagnostics::UnmappedActivity* BridgeDiagnostics::findUnmapped(uint32_t dgn,
                                                                     uint8_t sourceAddress,
                                                                     uint8_t instanceIndex) {
    UnmappedActivity* activity = nullptr;
    for (size_t index = 0; index < unmappedCount_; ++index) {
        if (unmapped_[index].dgn == dgn &&
            unmapped_[index].sourceAddress == sourceAddress &&
            unmapped_[index].instanceIndex == instanceIndex) {
            activity = &unmapped_[index];
            break;
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

void BridgeDiagnostics::observeUnmapped(RVC_DGN dgn, uint8_t sourceAddress, uint8_t instanceIndex,
                                        bool instanceVerified, const uint8_t* data, uint8_t dataLength) {
    BridgeDiagnostics& diagnostics = instance();
    do {
        uint32_t key = static_cast<uint32_t>(dgn);
        UnmappedActivity* activity = diagnostics.findUnmapped(key, sourceAddress, instanceIndex);
        if (activity == nullptr) {
            if (diagnostics.unmappedCount_ >= MAX_UNMAPPED) {
                if (diagnostics.unmappedOverflow_ < UINT32_MAX) ++diagnostics.unmappedOverflow_;
                break;
            }
            activity = &diagnostics.unmapped_[diagnostics.unmappedCount_++];
            activity->dgn = key;
            activity->sourceAddress = sourceAddress;
            activity->instanceIndex = instanceIndex;
        }

        activity->instanceVerified = instanceVerified;
        activity->lastSeenMs = nowMs();
        uint8_t sampleLength = data == nullptr ? 0 : (dataLength > 8 ? 8 : dataLength);
        activity->sampleLength = sampleLength;
        for (uint8_t index = 0; index < 8; ++index) {
            activity->sample[index] = index < sampleLength ? data[index] : 0;
        }
        activity->lastSampleAllFf = payloadAllFf(activity->sample, activity->sampleLength);
        if (activity->hitCount < UINT32_MAX) ++activity->hitCount;
    } while (false);
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
    bus["unmappedOverflowHits"] = diagnostics.unmappedOverflow_;
    document["handledFramesMeaning"] = "Handler returned true; not proof of valid status data or hardware acknowledgement";
    document["unmappedTrafficMeaning"] = "Observed CAN traffic without a configured HomeKit device; not necessarily RV-C and not an enabled accessory";
    document["payloadAllFfMeaning"] = "All sampled data bytes are 0xFF; often unavailable or not reporting";
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
            row["lastDgnName"] = LearnTable::dgnName(activity.lastDgn);
            row["lastFrameAgeMs"] = now - activity.lastSeenMs;
        } else {
            row["lastSourceAddress"] = nullptr;
            row["lastFrameAgeMs"] = nullptr;
        }
        row["receivedFrames"] = activity.receivedCount;
        row["handledFrames"] = activity.handledCount;
        if (activity.handledCount > 0) row["lastHandledFrameAgeMs"] = now - activity.lastHandledMs;
        else row["lastHandledFrameAgeMs"] = nullptr;
    }
    JsonArray unmapped = document["unmappedTraffic"].to<JsonArray>();
    for (size_t index = 0; index < diagnostics.unmappedCount_; ++index) {
        const UnmappedActivity& activity = diagnostics.unmapped_[index];
        RVC_DGN dgn = static_cast<RVC_DGN>(activity.dgn);
        JsonObject row = unmapped.add<JsonObject>();
        row["dgn"] = activity.dgn;
        const char* dgnName = LearnTable::dgnName(dgn);
        row["dgnName"] = dgnName;
        if (std::strcmp(dgnName, "UNKNOWN_DGN") != 0) {
            row["protocol"] = "RV-C";
            row["pgn"] = activity.dgn;
        } else {
            PgnDescription description = describeUnknownPgn(activity.dgn);
            row["protocol"] = description.protocol;
            row["pgn"] = description.value;
            if (description.name[0] != '\0') row["pgnName"] = description.name;
            if (description.hasDestinationAddress) {
                row["destinationAddress"] = description.destinationAddress;
            }
        }
        row["family"] = LearnTable::relatedFamily(dgn);
        row["rvcIndex"] = activity.instanceIndex;
        row["instanceVerified"] = activity.instanceVerified;
        row["sourceAddress"] = activity.sourceAddress;
        char payloadHex[17];
        appendPayloadHex(payloadHex, sizeof(payloadHex), activity.sample, activity.sampleLength);
        row["payloadHex"] = payloadHex;
        row["payloadAllFf"] = activity.lastSampleAllFf;
        row["hits"] = activity.hitCount;
        row["lastFrameAgeMs"] = now - activity.lastSeenMs;
        row["inReview"] = false;
        row["note"] = activity.lastSampleAllFf
            ? "Seen on bus; payload is all 0xFF, often unavailable or not reporting"
            : "Seen on bus; no configured device for this DGN/index";
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