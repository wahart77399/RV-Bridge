#include "BridgeDiagnostics.h"
#include "GenericDevice.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <cmath>
#include <driver/twai.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <functional>
#include <iostream>
#include <stdexcept>
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
        require(first["lastSourceAddress"] == 104 && first["lastDgn"] == static_cast<uint32_t>(THERMOSTAT_STATUS_1), "latest device observation missing");
        require(first["receivedFrames"] == 1 && first["handledFrames"] == 1, "device counters incorrect");
        require(first["lastFrameAgeMs"] == 2000 && first["lastHandledFrameAgeMs"] == 2000, "device age fields incorrect");
        JsonObjectConst second = report["configuredDevices"][1].as<JsonObjectConst>();
        require(second["observed"] == false && second["lastDgn"].isNull() && second["lastFrameAgeMs"].isNull(), "unobserved device fields are not null");
        require(fakeTwaiStatusReads == 1, "report performed unexpected driver polling");
        ESP.freeHeapBytes = 1000;
        require(BridgeDiagnostics::reportJson().isEmpty(), "diagnostics serialized without the reserved heap margin");
        ESP.freeHeapBytes = 120000;
    }
}

int main() {
    int exitCode = 0;
    try {
        testBoundedPassiveReport();
        std::cout << "PASS: bounded passive diagnostics report and counters\n";
    } catch (const std::exception& error) {
        std::cout << "FAIL: diagnostics report: " << error.what() << '\n';
        exitCode = 1;
    }
    return exitCode;
}