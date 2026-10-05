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
    constexpr size_t JSON_STREAM_BUFFER_BYTES = 512;

    class BufferedJsonSinkWriter {
    public:
        explicit BufferedJsonSinkWriter(DiagnosticsJsonSink& sink) : sink_(sink) {}

        size_t write(uint8_t value) {
            buffer_[buffered_++] = value;
            if (buffered_ == sizeof(buffer_)) flush();
            return failed_ ? 0 : 1;
        }

        size_t write(const uint8_t* data, size_t length) {
            size_t offset = 0;
            while (offset < length && !failed_) {
                size_t available = sizeof(buffer_) - buffered_;
                size_t copied = length - offset < available ? length - offset : available;
                memcpy(buffer_ + buffered_, data + offset, copied);
                buffered_ += copied;
                offset += copied;
                if (buffered_ == sizeof(buffer_)) flush();
            }
            return failed_ ? 0 : length;
        }

        bool flush() {
            if (!failed_ && buffered_ > 0) {
                failed_ = sink_.write(buffer_, buffered_) != buffered_;
                buffered_ = 0;
            }
            return !failed_;
        }

    private:
        DiagnosticsJsonSink& sink_;
        uint8_t buffer_[JSON_STREAM_BUFFER_BYTES] = {};
        size_t buffered_ = 0;
        bool failed_ = false;
    };

    class StringDiagnosticsJsonSink : public DiagnosticsJsonSink {
    public:
        explicit StringDiagnosticsJsonSink(String& output) : output_(output) {}
        size_t extraHeapRequired(size_t jsonLength) const override { return jsonLength; }
        bool begin(size_t jsonLength) override { return output_.reserve(jsonLength); }
        size_t write(const uint8_t* data, size_t length) override {
            return output_.concat(reinterpret_cast<const char*>(data), length) ? length : 0;
        }
        bool finish() override { return true; }

    private:
        String& output_;
    };

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

void BridgeDiagnostics::observeDeviceDetail(const GenericDevice* device, const char* detail) {
    if (device != nullptr && detail != nullptr) {
        DeviceActivity* activity = instance().findActivity(device);
        if (activity != nullptr) {
            activity->statusDetail = detail;
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

void BridgeDiagnostics::populateReport(JsonDocument& document) {
    const BridgeDiagnostics& diagnostics = instance();
    uint64_t now = nowMs();
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
        if (activity.statusDetail != nullptr && activity.statusDetail[0] != '\0') {
            row["statusDetail"] = activity.statusDetail;
        }
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
        } else {
            PgnDescription description = describeUnknownPgn(activity.dgn);
            row["protocol"] = description.protocol;
            if (description.value != activity.dgn) row["pgn"] = description.value;
            if (description.name[0] != '\0') row["pgnName"] = description.name;
            if (description.hasDestinationAddress) {
                row["destinationAddress"] = description.destinationAddress;
            }
        }
        const char* family = LearnTable::relatedFamily(dgn);
        if (std::strcmp(family, "Unknown") != 0) row["family"] = family;
        if (dgn == DM_RV) {
            row["rvcIndex"] = nullptr;
            row["instanceVerified"] = false;
            row["dsa"] = activity.sampleLength > 1 ? activity.sample[1] : 0xFF;
        } else {
            row["rvcIndex"] = activity.instanceIndex;
            row["instanceVerified"] = activity.instanceVerified;
        }
        row["sourceAddress"] = activity.sourceAddress;
        char payloadHex[17];
        appendPayloadHex(payloadHex, sizeof(payloadHex), activity.sample, activity.sampleLength);
        row["payloadHex"] = payloadHex;
        row["payloadAllFf"] = activity.lastSampleAllFf;
        row["hits"] = activity.hitCount;
        row["lastFrameAgeMs"] = now - activity.lastSeenMs;
        row["inReview"] = false;
        if (dgn == DM_RV && activity.sampleLength == 8) {
            const uint8_t operating = activity.sample[0];
            const uint8_t powerCode = operating & 0x03;
            const uint8_t activityCode = (operating >> 2) & 0x03;
            JsonObject details = row["dmRv"].to<JsonObject>();
            details["powerStateCode"] = powerCode;
            details["powerState"] = powerCode == 0 ? "Off" : powerCode == 1 ? "On" : "Reserved";
            details["activityStateCode"] = activityCode;
            details["activityState"] = activityCode == 0 ? "Standby" : activityCode == 1 ? "Active" : "Reserved";
            details["yellowLampCode"] = (operating >> 4) & 0x03;
            details["redLampCode"] = (operating >> 6) & 0x03;
            details["dsa"] = activity.sample[1];
            details["spnMsb"] = activity.sample[2];
            details["spnIntermediate"] = activity.sample[3];
            details["spnLsb"] = (activity.sample[4] >> 5) & 0x07;
            details["fmi"] = activity.sample[4] & 0x1F;
            uint8_t occurrenceCount = activity.sample[5] & 0x7F;
            details["occurrenceCountAvailable"] = occurrenceCount != 0x7F;
            if (occurrenceCount != 0x7F) details["occurrenceCount"] = occurrenceCount;
            details["occurrenceReservedBitSet"] = (activity.sample[5] & 0x80) != 0;
            details["dsaExtension"] = activity.sample[6];
            details["dsaExtensionDefined"] = activity.sample[6] != 0xFF;
            uint8_t bankSelect = activity.sample[7] & 0x0F;
            details["bankSelect"] = bankSelect;
            details["bankSelectionSupported"] = bankSelect <= 13;
        }
    }
}

bool BridgeDiagnostics::writeReport(DiagnosticsJsonSink& sink) {
    JsonDocument document;
    populateReport(document);
    if (document.overflowed()) return false;

    size_t expected = measureJson(document);
    if (expected == 0 || expected > MAX_DIAGNOSTICS_JSON_BYTES ||
        ESP.getFreeHeap() < MIN_FREE_HEAP_FOR_JSON + sink.extraHeapRequired(expected)) {
        return false;
    }
    if (!sink.begin(expected)) return false;

    BufferedJsonSinkWriter writer(sink);
    size_t written = serializeJson(document, writer);
    bool complete = writer.flush() && written == expected;
    return sink.finish() && complete;
}

String BridgeDiagnostics::reportJson() {
    String json;
    StringDiagnosticsJsonSink sink(json);
    if (!writeReport(sink)) json = "";
    return json;
}