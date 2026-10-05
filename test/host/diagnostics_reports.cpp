#include "BridgeDiagnostics.h"
#include "GenericDevice.h"
#include "LearnRecord.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <cmath>
#include <driver/twai.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    void require(bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    }

    void testBoundedPassiveReport() {
        GenericDevice observed;
        std::vector<GenericDevice> registered(149);
        GenericDevice overflow;
        BridgeDiagnostics::registerDevice(&observed, "Thermostat", "Living Thermostat", 3, 103);
        for (size_t index = 0; index < registered.size(); ++index) {
            BridgeDiagnostics::registerDevice(&registered[index], "Tank", "Tank", static_cast<uint8_t>(index), 120);
        }
        BridgeDiagnostics::registerDevice(&overflow, "Tank", "Untracked Tank", 250, 120);

        fakeEspTimerMicros = 1000000;
        CAN_frame_t frame;
        frame.FIR.B.FF = CAN_frame_ext;
        frame.FIR.B.RTR = CAN_RTR;
        BridgeDiagnostics::observeFrame(frame);

        fakeEspTimerMicros = 3000000;
        BridgeDiagnostics::observeDevice(&observed, THERMOSTAT_STATUS_1, 104, true);
        BridgeDiagnostics::observeDeviceDetail(&observed, "ATS source: Genset (Manual)");
        BridgeDiagnostics::observeDevice(&overflow, TANK_STATUS, 120, false);
        BridgeDiagnostics::observeTransmit(true);
        BridgeDiagnostics::observeTransmit(false);
        BridgeDiagnostics::observeQueueDrop();

        fakeEspTimerMicros = 5000000;
        fakeTwaiStatus.state = TWAI_STATE_BUS_OFF;
        fakeTwaiStatus.tx_error_counter = 4;
        fakeTwaiStatus.rx_error_counter = 5;
        fakeTwaiStatus.tx_failed_count = 6;
        fakeTwaiStatus.rx_missed_count = 7;
        fakeTwaiStatus.rx_overrun_count = 8;
        fakeTwaiStatus.arb_lost_count = 9;
        fakeTwaiStatus.bus_error_count = 10;
        WiFi.statusCode = WL_CONNECTED;

        String serialized = BridgeDiagnostics::reportJson();
        JsonDocument report;
        require(!serialized.isEmpty() && !deserializeJson(report, serialized.c_str()), "diagnostics JSON unavailable");
        require(report["uptimeMs"] == 5000 && report["resetReason"] == 7, "system runtime fields incorrect");
        require(report["freeHeapBytes"] == 120000 && report["minimumFreeHeapBytes"] == 90000, "heap fields incorrect");
        require(report["wifiConnected"] == true && report["wifiRssiDbm"] == -35, "Wi-Fi fields incorrect");
        require(report["configuredDevices"].size() == 150, "configured-device collection exceeded its bound");
        require(report["bus"]["untrackedConfiguredDevices"] == 1, "overflowed registrations were not counted");
        require(report["bus"]["untrackedDeviceFrames"] == 1, "untracked device traffic was not counted");
        require(report["bus"]["receivedFrames"] == 1 && report["bus"]["extendedFrames"] == 1 && report["bus"]["remoteFrames"] == 1, "bus frame counters incorrect");
        require(report["bus"]["lastFrameAgeMs"] == 4000, "bus frame age incorrect");
        double averageFramesPerSecond = report["bus"]["averageFramesPerSecondSinceBoot"].as<double>();
        require(std::fabs(averageFramesPerSecond - 0.2) < 0.0001, "average bus frame rate incorrect");
        require(report["bus"]["transmitAcceptedByDriver"] == 1 && report["bus"]["transmitRejectedByDriver"] == 1 && report["bus"]["softwareTransmitQueueDrops"] == 1, "transmit counters incorrect");
        require(report["bus"]["driverStatusAvailable"] == true && report["bus"]["busOff"] == true, "TWAI status was not reported");
        require(report["bus"]["txErrorCounter"] == 4 && report["bus"]["rxErrorCounter"] == 5 && report["bus"]["driverTransmitFailures"] == 6, "TWAI error counters incorrect");
        require(report["bus"]["receiveMissedCount"] == 7 && report["bus"]["receiveOverrunCount"] == 8 && report["bus"]["arbitrationLostCount"] == 9 && report["bus"]["busErrorCount"] == 10, "TWAI receive/error counters incorrect");
        require(report["handledFramesMeaning"].as<const char*>() != nullptr, "handled semantics disclaimer missing");
        JsonObjectConst first = report["configuredDevices"][0].as<JsonObjectConst>();
        require(first["name"] == "Living Thermostat" && first["observed"] == true, "registered device identity missing");
        require(first["lastSourceAddress"] == 104 && first["lastDgnName"] == "THERMOSTAT_STATUS_1", "latest device observation missing");
        require(first["statusDetail"] == "ATS source: Genset (Manual)", "device status detail missing");
        require(first["lastDgn"].isNull(), "numeric DGN should not be the primary label");
        require(first["receivedFrames"] == 1 && first["handledFrames"] == 1, "device counters incorrect");
        require(first["lastFrameAgeMs"] == 2000 && first["lastHandledFrameAgeMs"] == 2000, "device age fields incorrect");
        JsonObjectConst second = report["configuredDevices"][1].as<JsonObjectConst>();
        require(second["observed"] == false && second["lastDgnName"].isNull() && second["lastFrameAgeMs"].isNull(), "unobserved device fields are not null");
        require(fakeTwaiStatusReads == 1, "report performed unexpected driver polling");
        ESP.freeHeapBytes = 1000;
        require(BridgeDiagnostics::reportJson().isEmpty(), "diagnostics serialized without the reserved heap margin");
        ESP.freeHeapBytes = 120000;
    }

        void resetHostFakes() {
        fakeEspTimerMicros = 0;
        fakeTwaiStatus = twai_status_info_t{};
        fakeTwaiStatusResult = ESP_OK;
        fakeTwaiStatusReads = 0;
        WiFi.statusCode = 0;
        ESP.freeHeapBytes = 120000;
        ESP.minimumFreeHeapBytes = 90000;
        }

        void testUnmappedTrafficNameIndexPayloadAndAllFf() {
        resetHostFakes();
            const struct FixedDgnName {
                uint32_t value;
                const char* name;
            } fixedDgnNames[] = {
                {0x1FFE5, "SLIDE_MOTOR_STATUS"},
                {0x1FEF6, "THERMOSTAT_SCHEDULE_STATUS_2"},
                {0x1FECA, "DM-RV"},
                {0x1FEBC, "HYDRAULIC_PUMP_COMMAND"},
                {0x1FF99, "CHARGER_EQUALIZATION_STATUS"},
                {0x1FF98, "CHARGER_EQUALIZATION_CONFIGURATION_STATUS"},
                {0x1FEC2, "GENERATOR_DC_EQUALIZATION_STATUS"},
                {0x1FDC2, "DC_LIGHTING_CONTROLLER_STATUS_1"},
                {0x1FEB3, "SOLAR_CONTROLLER_STATUS_1"},
                {0x1FEF1, "TIRE_RAW_STATUS"},
                {0x1FEEC, "TIRE_PRESSURE_CONFIGURATION_COMMAND"},
                {0x1FEE9, "TIRE_ID_COMMAND"},
                {0x1FEE4, "LOCK_COMMAND"},
                {0x1FFB2, "WATER_PUMP_COMMAND"},
                {0x1FFA3, "ALTIMETER_STATUS"},
                {0x1FDE7, "DC_SOURCE_STATUS_13"},
                {0x1FDE6, "CAN_BUS_STATUS"},
                {0x1FDCA, "CHARGER_STATUS_3"},
                {0x1FDAE, "UNKNOWN_DGN"},
                {0x17F42, "UNKNOWN_DGN"}
            };
            for (const FixedDgnName& entry : fixedDgnNames) {
                require(std::strcmp(LearnTable::dgnName(static_cast<RVC_DGN>(entry.value)), entry.name) == 0,
                        "fixed DGN name or unknown fallback incorrect");
            }
            require(static_cast<uint32_t>(THERMOSTAT_SCHEDULE_STATUS_2) == 0x1FEF6,
                    "thermostat schedule status 2 value incorrect");
            require(static_cast<uint32_t>(DC_SOURCE_STATUS_13) == 0x1FDE7,
                    "DC source status 13 value incorrect");
            require(static_cast<uint32_t>(CHARGER_DC_STATUS) == 0x1FFC1,
                    "charger DC bus alias value incorrect");
                require(static_cast<uint32_t>(CHARGER_EQUALIZATION_STATUS) == 0x1FF99 &&
                    static_cast<uint32_t>(CHARGER_EQUALIZATION_CONFIGURATION_STATUS) == 0x1FF98,
                    "charger equalization DGN values incorrect");
                require(static_cast<uint32_t>(GENERATOR_DC_EQUALIZATION_STATUS) == 0x1FEC2,
                    "generator DC equalization DGN value incorrect");
            require(std::string(LearnTable::dgnName(static_cast<RVC_DGN>(0x12345))) == "UNKNOWN_DGN",
                "unlisted DGN did not use UNKNOWN_DGN fallback");
        uint8_t livePayload[8] = {0x02, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70};
        fakeEspTimerMicros = 1000000;
        BridgeDiagnostics::observeUnmapped(DC_SOURCE_STATUS_1, 33, 2, true, livePayload, 8);

        uint8_t updatedPayload[8] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
        fakeEspTimerMicros = 2000000;
        BridgeDiagnostics::observeUnmapped(DC_SOURCE_STATUS_1, 33, 2, true, updatedPayload, 8);

        uint8_t otherInstance[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        BridgeDiagnostics::observeUnmapped(DC_SOURCE_STATUS_1, 33, 3, true, otherInstance, 8);

        uint8_t allFf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        fakeEspTimerMicros = 3000000;
        BridgeDiagnostics::observeUnmapped(CHARGER_STATUS, 40, 1, false, allFf, 8);

        fakeEspTimerMicros = 3200000;
        BridgeDiagnostics::observeUnmapped(static_cast<RVC_DGN>(0xE894), 42, 0x94,
                           false, allFf, 8);
        fakeEspTimerMicros = 3400000;
        BridgeDiagnostics::observeUnmapped(static_cast<RVC_DGN>(0xEAFF), 43, 0xFF,
                           false, allFf, 8);
        fakeEspTimerMicros = 3600000;
        BridgeDiagnostics::observeUnmapped(static_cast<RVC_DGN>(0xFECA), 44, 1,
                           false, allFf, 8);
        fakeEspTimerMicros = 3800000;
        BridgeDiagnostics::observeUnmapped(static_cast<RVC_DGN>(0x1F809), 45, 1,
                           false, allFf, 8);
        fakeEspTimerMicros = 3900000;
        BridgeDiagnostics::observeUnmapped(static_cast<RVC_DGN>(0x12345), 41, 9,
                           false, allFf, 8);
        uint8_t dmRvPayload[8] = {0xF5, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        fakeEspTimerMicros = 3950000;
        BridgeDiagnostics::observeUnmapped(DM_RV, 141, 0x87, true, dmRvPayload, 8);

        fakeEspTimerMicros = 4000000;
        WiFi.statusCode = WL_CONNECTED;
        String serialized = BridgeDiagnostics::reportJson();
        JsonDocument report;
        require(!serialized.isEmpty() && !deserializeJson(report, serialized.c_str()),
            "unmapped diagnostics JSON unavailable");
        require(report["unmappedTrafficMeaning"].as<const char*>() != nullptr,
            "unmapped semantics disclaimer missing");
        require(report["payloadAllFfMeaning"].as<const char*>() != nullptr,
            "payloadAllFf semantics disclaimer missing");
        require(report["bus"]["unmappedOverflowHits"] == 0,
            "overflow counter should be zero under capacity");

        JsonArrayConst unmapped = report["unmappedTraffic"].as<JsonArrayConst>();
        require(unmapped.size() == 9, "expected nine unmapped rows");
        JsonObjectConst first = unmapped[0].as<JsonObjectConst>();
        require(first["dgnName"] == "DC_SOURCE_STATUS_1", "DGN enum name missing or incorrect");
        require(first["dgn"] == static_cast<uint32_t>(DC_SOURCE_STATUS_1),
            "raw DGN value missing from unmapped report");
        require(first["protocol"] == "RV-C" && first["family"] == "Battery" && first["pgn"].isNull(),
            "fixed RV-C protocol/family fields incorrect");
        require(first["rvcIndex"] == 2 && first["instanceVerified"] == true,
            "verified instance index incorrect");
        require(first["sourceAddress"] == 33 && first["hits"] == 2,
            "unmapped key or hit aggregation incorrect");
        require(first["payloadHex"] == "0211223344556677" && first["payloadAllFf"] == false,
            "latest payload sample incorrect");
        require(first["inReview"] == false && first["lastFrameAgeMs"] == 2000,
            "unmapped review state or frame age incorrect");

        JsonObjectConst second = unmapped[1].as<JsonObjectConst>();
        require(second["rvcIndex"] == 3 && second["hits"] == 1,
            "different instances were not kept separate");

        JsonObjectConst allFfRow = unmapped[2].as<JsonObjectConst>();
        require(allFfRow["dgnName"] == "CHARGER_STATUS" && allFfRow["instanceVerified"] == false,
            "unverified DGN metadata incorrect");
        require(allFfRow["payloadAllFf"] == true && allFfRow["payloadHex"] == "FFFFFFFFFFFFFFFF",
            "all-0xFF payload not reported");
        JsonObjectConst unknownRow = unmapped[3].as<JsonObjectConst>();
        require(unknownRow["dgn"] == 0xE894 && unknownRow["dgnName"] == "UNKNOWN_DGN",
            "unmapped PDU1 DGN value was not retained");
        require(unknownRow["protocol"] == "J1939" && unknownRow["pgnName"] == "ACKNOWLEDGMENT",
            "J1939 acknowledgement was not identified");
        require(unknownRow["family"].isNull(), "unknown RV-C family should be omitted for compactness");
        require(unknownRow["pgn"] == 0xE800 && unknownRow["destinationAddress"] == 0x94,
            "PDU1 PGN or destination address was not normalized");

        JsonObjectConst requestRow = unmapped[4].as<JsonObjectConst>();
        require(requestRow["pgnName"] == "REQUEST" && requestRow["pgn"] == 0xEA00 &&
                requestRow["destinationAddress"] == 0xFF,
            "J1939 global request was not identified");

        JsonObjectConst dm1Row = unmapped[5].as<JsonObjectConst>();
        require(dm1Row["pgnName"] == "DM1" && dm1Row["protocol"] == "J1939",
            "J1939 DM1 was not identified");

        JsonObjectConst nmeaRow = unmapped[6].as<JsonObjectConst>();
        require(nmeaRow["pgnName"] == "TIME_AND_DATE" && nmeaRow["protocol"] == "NMEA 2000",
            "NMEA 2000 time/date PGN was not identified");

        JsonObjectConst unclassifiedRow = unmapped[7].as<JsonObjectConst>();
        require(unclassifiedRow["dgn"] == 0x12345 && unclassifiedRow["dgnName"] == "UNKNOWN_DGN" &&
            unclassifiedRow["protocol"] == "Unknown" && unclassifiedRow["pgn"].isNull(),
            "unknown DGN value was not retained for identification");

        JsonObjectConst dmRvRow = unmapped[8].as<JsonObjectConst>();
        JsonObjectConst dmRv = dmRvRow["dmRv"].as<JsonObjectConst>();
        require(dmRvRow["dgnName"] == "DM-RV" && dmRv["powerState"] == "On" &&
                    dmRv["activityState"] == "Active" && dmRv["yellowLampCode"] == 3 &&
                    dmRv["redLampCode"] == 3 && dmRv["dsa"] == 0x87,
                "DM-RV status bits were not decoded");
        require(dmRvRow["rvcIndex"].isNull() && dmRvRow["instanceVerified"] == false &&
                    dmRvRow["dsa"] == 0x87,
                "DM-RV DSA was mislabeled as an RVC instance");
        require(dmRv["spnMsb"] == 0xFF && dmRv["spnIntermediate"] == 0xFF &&
                    dmRv["spnLsb"] == 7 && dmRv["fmi"] == 31 &&
                    dmRv["occurrenceCountAvailable"] == false &&
                    dmRv["dsaExtensionDefined"] == false &&
                    dmRv["bankSelectionSupported"] == false,
                "DM-RV diagnostic fields or sentinels were not decoded");
        }

        void testUnmappedOverflowBound() {
        resetHostFakes();
        uint8_t payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        for (uint16_t index = 0; index < 64; ++index) {
            BridgeDiagnostics::observeUnmapped(TANK_STATUS, static_cast<uint8_t>(index),
                               static_cast<uint8_t>(index), true, payload, 8);
        }
        BridgeDiagnostics::observeUnmapped(TANK_STATUS, 200, 200, true, payload, 8);

        String serialized = BridgeDiagnostics::reportJson();
        JsonDocument report;
        require(!serialized.isEmpty() && !deserializeJson(report, serialized.c_str()),
            "overflow diagnostics JSON unavailable");
        require(report["unmappedTraffic"].size() == 64, "unmapped table exceeded bound");
        require(report["bus"]["unmappedOverflowHits"] == 1, "overflow hits not counted");
        }

    void testLiveScaleDiagnosticsReportFits() {
        resetHostFakes();
        GenericDevice observed;
        std::vector<GenericDevice> registered(32);
        BridgeDiagnostics::registerDevice(&observed, "Thermostat", "Observed", 1, 103);
        for (size_t index = 0; index < registered.size(); ++index) {
            BridgeDiagnostics::registerDevice(&registered[index], "Battery", "Battery",
                                              static_cast<uint8_t>(index), 250);
        }
        uint8_t payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
        for (uint16_t index = 0; index < 64; ++index) {
            BridgeDiagnostics::observeUnmapped(static_cast<RVC_DGN>(0x12300 + index),
                                               static_cast<uint8_t>(index),
                                               static_cast<uint8_t>(index), false, payload, 8);
        }

        String serialized = BridgeDiagnostics::reportJson();
        JsonDocument report;
        require(!serialized.isEmpty() && !deserializeJson(report, serialized.c_str()),
                "live-scale diagnostics JSON unavailable");
        require(report["configuredDevices"].size() == 33 && report["unmappedTraffic"].size() == 64,
                "live-scale diagnostics rows were truncated");
    }
}

    int main(int argc, char** argv) {
    try {
        if (argc > 1 && std::strcmp(argv[1], "--unmapped") == 0) {
            testUnmappedTrafficNameIndexPayloadAndAllFf();
            std::cout << "PASS: unmapped traffic name index payload and all-FF\n";
        } else if (argc > 1 && std::strcmp(argv[1], "--unmapped-overflow") == 0) {
            testUnmappedOverflowBound();
            std::cout << "PASS: unmapped overflow bound\n";
        } else if (argc > 1 && std::strcmp(argv[1], "--live-scale") == 0) {
            testLiveScaleDiagnosticsReportFits();
            std::cout << "PASS: live-scale diagnostics report fits\n";
        } else {
            testBoundedPassiveReport();
            std::cout << "PASS: bounded passive diagnostics report and counters\n";
        }
    } catch (const std::exception& error) {
        std::cout << "FAIL: diagnostics report: " << error.what() << '\n';
        return 1;
    }
        return 0;
}