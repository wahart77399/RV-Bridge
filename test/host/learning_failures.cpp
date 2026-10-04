#include "Arduino.h"
#include "LittleFS.h"
#include <ArduinoJson.h>
#include "LearnMode.h"
#include "Preferences.h"
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
    const char* profilePath = "/profiles/test-coach-1.json";
    const std::string previousConfig = R"([{"type":"Thermostat","rvcIndex":1,"sourceAddress":103,"aid":43,"name":"Owner Name","room":"Owner Room","enabled":false},{"type":"DoorLock","rvcIndex":5,"sourceAddress":191,"aid":73,"name":"Quiet Lock","room":"Storage","enabled":true}])";
    const std::string profileConfig = R"([{"type":"Thermostat","rvcIndex":1,"sourceAddress":1,"aid":6,"name":"Profile Name","room":"Profile Room","enabled":true},{"type":"Tank","rvcIndex":0,"sourceAddress":250,"aid":18,"name":"Fresh","enabled":true}])";

    void require(bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    }

    void fixture() {
        LittleFS = FakeFilesystem{};
        preferencesStore = FakePreferencesStore{};
        LearnMode::handleCommand("start 1");
        LearnMode::handleCommand("cancel");
        LittleFS = FakeFilesystem{};
        Serial.log.clear();
        ESP.restarts = 0;
        fakeMillis = 1000;
        LittleFS.put("/coach.json", R"({"year":2022,"make":"Test","model":"Coach","floorplan":"1","coachId":"test","chassisAid":61})");
        LittleFS.put("/devices.json", previousConfig);
        LittleFS.put(profilePath, profileConfig);
    }

    void startAndObserve() {
        LearnMode::handleCommand("start 1");
        require(LearnMode::isLearning(), "learning did not start");
        uint8_t payload[8] = {1, 0, 0, 0, 0, 0, 0, 0};
        for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(THERMOSTAT_STATUS_1, 104, payload);
    }

    void finish() {
        try {
            LearnMode::handleCommand("finish");
        } catch (const RestartRequested&) {
        }
    }

    JsonDocument deviceConfig() {
        JsonDocument result;
        require(!deserializeJson(result, LittleFS.content("/devices.json")), "device config is not valid JSON");
        return result;
    }

    void assertFailed() {
        require(LearnMode::isLearning(), "failed completion marked complete");
        require(ESP.restarts == 0, "failed completion rebooted");
        require(LittleFS.content("/devices.prev.json") == previousConfig, "backup was damaged");
    }
}

int main() {
    const std::vector<std::pair<const char*, std::function<void()>>> tests = {
        {"successful merge retains owner identity and quiet devices", [] {
            fixture(); startAndObserve(); finish();
            require(ESP.restarts == 1, "successful completion did not reboot");
            require(!LearnMode::isLearning(), "successful completion remained learning");
            JsonDocument config = deviceConfig();
            require(config.size() == 3, "quiet or profile device lost or duplicated");
            JsonObject owner = config[0];
            require(owner["aid"] == 43 && owner["name"] == "Owner Name" && owner["room"] == "Owner Room", "profile overwrote identity");
            require(owner["enabled"] == false && owner["sourceAddress"] == 104, "approval lost or observation not merged");
            require(config[1]["aid"] == 73 && config[1]["learned"] == false, "quiet device identity lost");
            require(config[2]["aid"].isNull(), "new device inherited a profile AID");
        }},
        {"new unprofiled device remains pending", [] {
            fixture(); startAndObserve();
            uint8_t payload[8] = {3};
            for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(THERMOSTAT_STATUS_1, 103, payload);
            finish(); JsonDocument config = deviceConfig();
            require(config[2]["rvcIndex"] == 3 && config[2]["enabled"] == false && config[2]["aid"].isNull(), "unprofiled device enabled or assigned borrowed AID");
        }},
        {"backup rename failure refuses learning", [] {
            fixture(); LittleFS.failRename.insert("/devices.json");
            LearnMode::handleCommand("start 1");
            require(!LearnMode::isLearning() && LittleFS.content("/devices.json") == previousConfig, "failed backup still started learning");
        }},
        {"starting twice preserves inventory and original backup", [] {
            fixture(); startAndObserve();
            LearnMode::handleCommand("start 1"); finish();
            require(LittleFS.content("/devices.prev.json") == previousConfig, "second start overwrote backup");
            JsonDocument config = deviceConfig();
            require(config[0]["learned"] == true, "second start cleared observed inventory");
        }},
        {"short device write is rejected", [] {
            fixture(); startAndObserve(); LittleFS.writeLimits["/devices.tmp"] = 8; finish(); assertFailed();
            require(!LittleFS.exists("/devices.json") && !LittleFS.exists("/devices.tmp"), "partial device config promoted or retained");
        }},
        {"device open failure retains learning", [] {
            fixture(); startAndObserve(); LittleFS.failOpen.insert("/devices.tmp"); finish(); assertFailed();
        }},
        {"short report write retains existing report", [] {
            fixture(); startAndObserve(); LittleFS.put("/learn_report.json", "old-report");
            LittleFS.writeLimits["/learn_report.tmp"] = 8; finish(); assertFailed();
            require(LittleFS.content("/learn_report.json") == "old-report", "short report overwrote valid report");
            require(!LittleFS.exists("/devices.json"), "devices published despite report failure");
        }},
        {"report rename failure retains old destination", [] {
            fixture(); startAndObserve(); LittleFS.put("/learn_report.json", "old-report");
            LittleFS.failRename.insert("/learn_report.tmp"); finish(); assertFailed();
            require(LittleFS.content("/learn_report.json") == "old-report", "failed report rename deleted destination");
        }},
        {"device rename failure retains old destination", [] {
            fixture(); startAndObserve(); LittleFS.put("/devices.json", previousConfig);
            LittleFS.failRename.insert("/devices.tmp"); finish(); assertFailed();
            require(LittleFS.content("/devices.json") == previousConfig, "failed device rename deleted destination");
        }},
        {"short state write cannot mark completion", [] {
            fixture(); startAndObserve(); const auto originalState = LittleFS.content("/learn.bin");
            LittleFS.writeLimits["/learn.tmp"] = 1; finish(); assertFailed();
            require(LittleFS.content("/learn.bin") == originalState, "short state write overwrote saved learning");
        }},
        {"state rename failure preserves old state", [] {
            fixture(); startAndObserve(); const auto originalState = LittleFS.content("/learn.bin");
            LittleFS.failRename.insert("/learn.tmp"); finish(); assertFailed();
            require(LittleFS.content("/learn.bin") == originalState, "failed state rename deleted original");
        }},
        {"malformed profile fails closed", [] {
            fixture(); startAndObserve(); LittleFS.put(profilePath, "{broken"); finish(); assertFailed();
            require(!LittleFS.exists("/devices.json"), "invalid profile produced device config");
        }},
        {"malformed backup fails closed", [] {
            fixture(); startAndObserve(); LittleFS.put("/devices.prev.json", "{broken"); finish();
            require(LearnMode::isLearning() && ESP.restarts == 0, "invalid backup was accepted");
        }},
        {"missing coach refuses completion", [] {
            fixture(); startAndObserve(); LittleFS.remove("/coach.json"); finish(); assertFailed();
        }},
        {"automatic retries are limited and can recover", [] {
            fixture(); startAndObserve(); fakeMillis += 3600000;
            LittleFS.writeLimits["/devices.tmp"] = 1; finish(); assertFailed();
            const unsigned attempts = LittleFS.writeAttempts["/devices.tmp"];
            LearnMode::poll(); fakeMillis += 59000; LearnMode::poll();
            require(LittleFS.writeAttempts["/devices.tmp"] == attempts, "retried before one minute");
            LittleFS.writeLimits.clear(); fakeMillis += 1000;
            try { LearnMode::poll(); } catch (const RestartRequested&) {}
            require(ESP.restarts == 1 && !LearnMode::isLearning(), "retry did not recover");
        }},
        {"cancel after failed state save restores original config", [] {
            fixture(); startAndObserve(); LittleFS.writeLimits["/learn.tmp"] = 1;
            finish(); assertFailed(); LittleFS.writeLimits.clear();
            LearnMode::handleCommand("cancel");
            require(LittleFS.content("/devices.json") == previousConfig, "cancel left a prematurely published config");
        }},
        {"cancel restore failure retains learning and backup", [] {
            fixture(); startAndObserve();
            LittleFS.failRename.insert("/devices.prev.json");
            LearnMode::handleCommand("cancel");
            require(LearnMode::isLearning(), "failed restore reported cancellation");
            require(LittleFS.content("/devices.prev.json") == previousConfig, "failed cancellation damaged backup");
        }},
        {"cancel outside learning cannot restore a stale backup", [] {
            fixture();
            LittleFS.put("/devices.prev.json", "stale-backup");
            LearnMode::handleCommand("cancel");
            require(LittleFS.content("/devices.json") == previousConfig, "idle cancellation replaced live config");
            require(LittleFS.content("/devices.prev.json") == "stale-backup", "idle cancellation consumed backup");
        }},
        {"late discovery is pending with an AID and preserved owner metadata", [] {
            fixture(); startAndObserve(); finish();
            uint8_t payload[8] = {2};
            for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(CHARGER_STATUS, 250, payload);
            fakeMillis += 60001; LearnMode::poll();
            JsonDocument config = deviceConfig();
            require(config.size() == 4, "late discovery not appended");
            require(config[3]["type"] == "Charger" && config[3]["enabled"] == false && config[3]["aid"].as<unsigned>() >= 2, "late device enabled or lacks reserved AID");
            require(config[0]["aid"] == 43 && config[0]["room"] == "Owner Room", "late discovery changed existing identity");
            require(ESP.restarts == 1, "late discovery triggered an automatic restart");
        }},
        {"unhandled traffic is report-only with an unverified index", [] {
            fixture(); startAndObserve(); finish();
            const auto original = LittleFS.content("/devices.json");
            uint8_t payload[8] = {9};
            for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(ATS_STATUS, 79, payload);
            fakeMillis += 60001; LearnMode::poll();
            require(LittleFS.content("/devices.json") == original, "unhandled traffic became a device");
            JsonDocument discovery;
            require(!deserializeJson(discovery, LearnMode::discoveryJson().c_str()), "invalid discovery snapshot");
            require(discovery["records"][1]["rvcIndex"] == 9 && discovery["records"][1]["rvcIndexVerified"] == false, "unhandled index not marked as candidate");
        }},
        {"ignored late discovery is not offered again", [] {
            fixture(); startAndObserve(); finish();
            JsonDocument config = deviceConfig();
            JsonObject ignored = config.as<JsonArray>().add<JsonObject>();
            ignored["type"] = "Charger"; ignored["rvcIndex"] = 2; ignored["sourceAddress"] = 250;
            ignored["enabled"] = false; ignored["ignored"] = true; ignored["name"] = "Ignored"; ignored["aid"] = 120;
            std::string original; serializeJson(config, original); LittleFS.put("/devices.json", original);
            uint8_t payload[8] = {2};
            for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(CHARGER_STATUS, 250, payload);
            fakeMillis += 60001; LearnMode::poll();
            require(LittleFS.content("/devices.json") == original, "ignored discovery overwritten or duplicated");
        }},
        {"short late-discovery write preserves live config", [] {
            fixture(); startAndObserve(); finish();
            const auto original = LittleFS.content("/devices.json");
            uint8_t payload[8] = {2};
            for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(CHARGER_STATUS, 250, payload);
            LittleFS.writeLimits["/devices.discovery.tmp"] = 1; fakeMillis += 60001; LearnMode::poll();
            require(LittleFS.content("/devices.json") == original, "partial discovery replaced live config");
            LittleFS.writeLimits.clear(); fakeMillis += 60001; LearnMode::poll();
            require(deviceConfig().size() == 4, "late discovery retry failed");
        }},
        {"late discovery rename failure preserves live config", [] {
            fixture(); startAndObserve(); finish();
            const auto original = LittleFS.content("/devices.json");
            uint8_t payload[8] = {2};
            for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(CHARGER_STATUS, 250, payload);
            LittleFS.failRename.insert("/devices.discovery.tmp"); fakeMillis += 60001; LearnMode::poll();
            require(LittleFS.content("/devices.json") == original, "failed discovery rename changed live config");
        }},
        {"late discovery cannot save without durable AID reservations", [] {
            fixture(); startAndObserve(); finish();
            const auto original = LittleFS.content("/devices.json");
            uint8_t payload[8] = {2};
            for (unsigned hit = 0; hit < 3; ++hit) LearnMode::observe(CHARGER_STATUS, 250, payload);
            preferencesStore.available = false; fakeMillis += 60001; LearnMode::poll();
            require(LittleFS.content("/devices.json") == original, "unpersisted AID published");
        }}
    };
    unsigned failures = 0;
    for (const auto& test : tests) {
        try {
            test.second();
            std::cout << "PASS: " << test.first << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cout << "FAIL: " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() - failures << '/' << tests.size() << " tests passed\n";
    return failures == 0 ? 0 : 1;
}