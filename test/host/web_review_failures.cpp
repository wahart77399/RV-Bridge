#include "SmartCoachWeb.h"
#include "WifiCredentials.h"
#include "LearnMode.h"
#include "EmailReports.h"
#include "BridgeDiagnostics.h"
#include "Preferences.h"
#include <driver/twai.h>
#include <esp_timer.h>
#include <ArduinoJson.h>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

bool WifiCredentials::clear() { return true; }

namespace {
    const std::string initial = R"([{"type":"Thermostat","rvcIndex":1,"sourceAddress":103,"name":"Existing","room":"Owner Room","enabled":true,"aid":43},{"type":"DC_Switch","rvcIndex":81,"sourceAddress":141,"name":"Pending Light","room":"","enabled":false,"aid":76},{"type":"DC_Switch","rvcIndex":83,"sourceAddress":141,"name":"Ignored Light","room":"","enabled":false,"ignored":true,"aid":77}])";
    void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
    void fixture() {
        LittleFS = FakeFilesystem{};
        preferencesStore = FakePreferencesStore{};
        ESP.restarts = 0;
        fakeMillis = 1000;
        LittleFS.put("/coach.json", R"({"year":2022,"make":"Test","model":"Coach","floorplan":"1","coachId":"test","chassisAid":61,"ownerEmails":["owner@example.com"],"emailReports":true,"diagnosticEmails":false,"shareWithSupport":false,"tanks":[],"coverTimes":{},"batteries":[]})");
        LittleFS.put("/devices.json", initial);
    }
    JsonDocument config() {
        JsonDocument document;
        require(!deserializeJson(document, LittleFS.content("/devices.json")), "invalid saved configuration");
        return document;
    }
    void review(const char* action, bool confirmed = true, const char* type = "DC_Switch", unsigned index = 81, unsigned address = 141) {
        JsonDocument request;
        request["action"] = action; request["confirmed"] = confirmed; request["type"] = type;
        request["rvcIndex"] = index; request["sourceAddress"] = address;
        String body; serializeJson(request, body);
        WebServer::active->request("/devices/review", HTTP_POST, body);
    }
    void saveEmailSettings(const char* address, const char* endpoint = "", const char* token = "") {
        JsonDocument request;
        request["emailReports"] = true;
        request["diagnosticEmails"] = false;
        request["shareWithSupport"] = false;
        request["endpoint"] = endpoint;
        request["token"] = token;
        request["ownerEmails"].to<JsonArray>().add(address);
        String body; serializeJson(request, body);
        WebServer::active->request("/email/settings", HTTP_POST, body);
    }
}

int main(int argc, char** argv) {
    SmartCoachWebServer::instance().begin();
    int exitCode = 0;
    if (argc > 1 && std::string(argv[1]) == "--preview") {
        fixture();
        uint8_t thermostat[8] = {1};
        uint8_t light[8] = {81, 0, 50};
        uint8_t unknown[8] = {9, 2, 3, 4, 5, 6, 7, 8};
        for (unsigned hit = 0; hit < 3; ++hit) {
            LearnMode::observe(THERMOSTAT_STATUS_1, 103, thermostat);
            LearnMode::observe(DC_DIMMER_STATUS_3, 141, light);
            LearnMode::observe(ATS_STATUS, 79, unknown);
        }
        std::string line;
        while (std::getline(std::cin, line)) {
            JsonDocument request;
            JsonDocument response;
            if (!deserializeJson(request, line)) {
                const char* route = request["path"] | "/status";
                int method = request["method"] == "POST" ? HTTP_POST : HTTP_GET;
                const char* body = request["body"] | "";
                try { WebServer::active->request(route, method, body); }
                catch (const RestartRequested&) {}
                response["code"] = WebServer::active->responseCode;
                response["body"] = WebServer::active->responseBody;
            } else {
                response["code"] = 400;
                response["body"] = "{}";
            }
            std::string output; serializeJson(response, output);
            std::cout << output << std::endl;
        }
    } else {
        const std::vector<std::pair<const char*, std::function<void()>>> tests = {
            {"approval requires explicit identity confirmation", [] {
                fixture(); review("approve", false);
                require(WebServer::active->responseCode == 400 && LittleFS.content("/devices.json") == initial, "unconfirmed approval was saved");
            }},
            {"approval retains existing IDs and does not reboot", [] {
                fixture(); review("approve"); JsonDocument document = config();
                require(WebServer::active->responseCode == 200 && document[1]["enabled"] == true, "approval failed");
                require(document[1]["aid"] == 76 && document[0]["aid"] == 43 && document[0]["room"] == "Owner Room", "approval renumbered existing devices");
                require(ESP.restarts == 0, "approval rebooted immediately");
                WebServer::active->request("/status", HTTP_GET);
                JsonDocument status; deserializeJson(status, WebServer::active->responseBody.c_str());
                require(status["restart_required"] == true, "approval omitted restart state");
            }},
            {"ignore/reopen retains pending reservation", [] {
                fixture(); review("ignore"); JsonDocument ignored = config();
                require(WebServer::active->responseCode == 200 && ignored[1]["ignored"] == true && ignored[1]["aid"] == 76, "ignore lost reservation");
                review("reopen"); JsonDocument reopened = config();
                require(WebServer::active->responseCode == 200 && reopened[1]["ignored"] == false && reopened[1]["enabled"] == false, "reopen unexpectedly enabled device");
            }},
            {"ignore cannot remove an enabled accessory", [] {
                fixture(); review("ignore", true, "Thermostat", 1, 103);
                require(WebServer::active->responseCode == 409 && LittleFS.content("/devices.json") == initial, "enabled accessory disabled by review");
            }},
            {"short approval write preserves config", [] {
                fixture(); LittleFS.writeLimits["/devices.json.web.tmp"] = 4; review("approve");
                require(WebServer::active->responseCode == 500 && LittleFS.content("/devices.json") == initial, "short approval write accepted");
            }},
            {"approval rename failure preserves config", [] {
                fixture(); LittleFS.failRename.insert("/devices.json.web.tmp"); review("approve");
                require(WebServer::active->responseCode == 500 && LittleFS.content("/devices.json") == initial, "failed approval rename changed config");
            }},
            {"approval refuses failed NVS reservation", [] {
                fixture(); preferencesStore.available = false; review("approve");
                require(WebServer::active->responseCode == 409 && LittleFS.content("/devices.json") == initial, "unpersisted approval accepted");
            }},
            {"approval cannot exceed 150 accessories", [] {
                fixture(); JsonDocument document = config();
                for (unsigned index = 0; index < 144; ++index) {
                    JsonObject device = document.as<JsonArray>().add<JsonObject>();
                    device["type"] = "Tank"; device["rvcIndex"] = index; device["sourceAddress"] = 250;
                    device["enabled"] = true; device["aid"] = 200 + index; device["name"] = "Tank";
                }
                std::string before; serializeJson(document, before); LittleFS.put("/devices.json", before);
                review("approve");
                require(WebServer::active->responseCode == 409 && LittleFS.content("/devices.json") == before, "oversized approval accepted");
            }},
            {"review refuses active learning", [] {
                fixture(); LearnMode::handleCommand("start 1"); review("approve");
                require(WebServer::active->responseCode == 409 && LearnMode::isLearning(), "review interfered with learning");
                LearnMode::handleCommand("cancel");
            }},
            {"email settings save validated owner and preserve relay token", [] {
                fixture();
                require(EmailReports::configureRelay("https://script.google.com/macros/s/AKfycb1234567890abcdef/exec", String(std::string(64, 'a'))), "test relay setup failed");
                saveEmailSettings("owner@example.com");
                JsonDocument coach; deserializeJson(coach, LittleFS.content("/coach.json"));
                require(WebServer::active->responseCode == 200 && coach["ownerEmails"][0] == "owner@example.com", "owner recipient was not saved");
                require(coach["shareWithSupport"] == false && coach["emailReports"] == true, "email consent preferences were not saved");
                require(EmailReports::relayConfigured(), "saving owner address lost the relay token");
            }},
            {"invalid owner address cannot be saved", [] {
                fixture();
                saveEmailSettings("not-an-email");
                JsonDocument coach; deserializeJson(coach, LittleFS.content("/coach.json"));
                require(WebServer::active->responseCode == 400 && coach["ownerEmails"][0] == "owner@example.com", "invalid recipient changed coach configuration");
            }},
            {"email stage falls back to explicitly labeled current inventory", [] {
                fixture();
                LittleFS.put("/devices.json", R"([{"type":"DC_Switch","rvcIndex":81,"sourceAddress":141,"name":"Galley Light","enabled":true,"aid":76}])");
                WebServer::active->request("/email/stage", HTTP_POST);
                JsonDocument report; deserializeJson(report, LittleFS.content("/email_outbox.json"));
                require(WebServer::active->responseCode == 200 && report["reportKind"] == "currentInventory", "inventory fallback was not staged");
                require(report["report"]["devices"][0]["name"] == "Galley Light", "inventory snapshot content missing");
            }},
            {"diagnostics route returns passive downloadable report", [] {
                fixture();
                fakeEspTimerMicros = 1000000;
                fakeTwaiStatusReads = 0;
                Serial.log.clear();
                WebServer::active->request("/diagnostics", HTTP_GET);
                JsonDocument report; deserializeJson(report, WebServer::active->responseBody.c_str());
                require(WebServer::active->responseCode == 200, "diagnostics route failed");
                require(WebServer::active->responseHeaders["Cache-Control"] == "no-store", "diagnostics response can be cached");
                require(WebServer::active->responseHeaders["Content-Disposition"].find("attachment") != std::string::npos, "diagnostics response is not a download");
                require(report["bus"]["receivedFrames"] == 0 && report["configuredDevices"].size() == 0, "unexpected diagnostics data");
                require(fakeTwaiStatusReads == 1, "diagnostics request made an unexpected number of driver reads");
                require(Serial.log.find("diagnostics-report freeHeap=") != std::string::npos, "diagnostics heap baseline missing");
            }},
            {"root injects diagnostics link without changing filesystem", [] {
                fixture();
                const std::string original = "<html><body><footer class=\"footer\"><span>SmartCoach</span></footer></body></html>";
                LittleFS.put("/SmartCoachDevicePortal/index.html", original);
                LittleFS.files["/SmartCoachDevicePortal/index.html"]->availableAtEof = true;
                Serial.log.clear();
                WebServer::active->request("/", HTTP_GET);
                const std::string response = WebServer::active->responseBody.c_str();
                size_t linkPosition = response.find("href=\"/diagnostics-page\"");
                require(WebServer::active->responseCode == 200 && response.find("</html>") != std::string::npos, "root portal response was incomplete");
                require(response.find("<span>SmartCoach</span>") != std::string::npos && linkPosition != std::string::npos, "existing portal or diagnostics link missing");
                require(response.find("href=\"/email\">Email reports</a>") != std::string::npos, "email setup link missing");
                require(response.find("href=\"/diagnostics-page\"", linkPosition + 1) == std::string::npos, "diagnostics link duplicated");
                require(LittleFS.content("/SmartCoachDevicePortal/index.html") == original, "portal filesystem was modified");
                require(Serial.log.find("root-page freeHeap=") != std::string::npos, "root-page heap baseline missing");
            }},
            {"root streams original portal when heap reserve is low", [] {
                fixture();
                const std::string original = "<html><body><footer><span>SmartCoach</span></footer></body></html>";
                LittleFS.put("/SmartCoachDevicePortal/index.html", original);
                ESP.freeHeapBytes = 1000;
                WebServer::active->request("/", HTTP_GET);
                ESP.freeHeapBytes = 120000;
                require(WebServer::active->responseCode == 200 && std::string(WebServer::active->responseBody.c_str()) == original, "low-heap root did not stream the original portal");
            }},
            {"root does not duplicate existing diagnostics link", [] {
                fixture();
                const std::string original = "<html><body><footer><button id=\"diagnostics-open\">Diagnostics</button></footer></body></html>";
                LittleFS.put("/SmartCoachDevicePortal/index.html", original);
                WebServer::active->request("/", HTTP_GET);
                const std::string response = WebServer::active->responseBody.c_str();
                require(response.find("href=\"/diagnostics-page\"") == std::string::npos, "existing portal got a duplicate link");
                require(LittleFS.content("/SmartCoachDevicePortal/index.html") == original, "existing portal was modified");
            }},
            {"firmware diagnostics page route is available", [] {
                fixture();
                WebServer::active->request("/diagnostics-page", HTTP_GET);
                require(WebServer::active->responseCode == 200 && WebServer::active->responseBody.indexOf("SmartCoach Diagnostics") >= 0, "firmware diagnostics page was unavailable");
                require(WebServer::active->responseBody.indexOf("Unmapped CAN traffic") >= 0 && WebServer::active->responseBody.indexOf("Review status") >= 0, "unmapped DGN table was unavailable");
            }}
        };
        unsigned failures = 0;
        for (const auto& test : tests) {
            try { test.second(); std::cout << "PASS: " << test.first << '\n'; }
            catch (const std::exception& error) { ++failures; std::cout << "FAIL: " << test.first << ": " << error.what() << '\n'; }
        }
        std::cout << tests.size() - failures << '/' << tests.size() << " web review tests passed\n";
        if (failures != 0) exitCode = 1;
    }
    return exitCode;
}