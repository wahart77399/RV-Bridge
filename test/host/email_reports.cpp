#include "EmailReports.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
    const char* report = R"({"devices":[],"unknownTraffic":[]})";
    const String endpoint = "https://script.google.com/macros/s/AKfycb1234567890abcdef/exec";
    const String token(std::string(64, 'a'));

    void require(bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    }

    void fixture(bool ownerConsent, bool diagnosticConsent, bool supportConsent) {
        LittleFS = FakeFilesystem{};
        preferencesStore = FakePreferencesStore{};
        fakeHttp = FakeHttpEnvironment{};
        WiFi.statusCode = WL_CONNECTED;
        JsonDocument coach;
        coach["year"] = 2022;
        coach["make"] = "Test";
        coach["model"] = "Coach";
        coach["floorplan"] = "1";
        coach["coachId"] = "test-coach";
        coach["emailReports"] = ownerConsent;
        coach["diagnosticEmails"] = diagnosticConsent;
        coach["shareWithSupport"] = supportConsent;
        coach["ownerEmails"].to<JsonArray>().add("owner@example.com");
        std::string serialized;
        serializeJson(coach, serialized);
        LittleFS.put("/coach.json", serialized);
        LittleFS.put("/learn_report.json", report);
        LittleFS.put("/devices.json", R"([{"type":"DC_Switch","rvcIndex":81,"sourceAddress":141,"name":"Galley Light","enabled":true,"aid":76}])");
    }

    JsonDocument outbox() {
        JsonDocument document;
        require(!deserializeJson(document, LittleFS.content("/email_outbox.json")), "invalid outbox JSON");
        return document;
    }

    void configureRelay() {
        require(EmailReports::configureRelay(endpoint, token), "valid relay configuration was rejected");
    }

    void putPendingReport() {
        LittleFS.put("/email_outbox.json", R"({"schemaVersion":1,"requestId":"0123456789ABCDEF","category":"discovery","ownerEmailConsent":true,"diagnosticEmailConsent":false,"shareWithSupport":false,"ownerEmails":["owner@example.com"],"coach":{"year":2022,"make":"Test","model":"Coach","floorplan":"1","coachId":"test-coach"},"report":{"devices":[]}})");
    }
}

int main() {
    const std::vector<std::pair<const char*, std::function<void()>>> tests = {
        {"discovery report requires explicit owner consent", [] {
            fixture(false, false, false);
            require(EmailReports::stageDiscoveryReport() == EmailReports::Result::NotConfigured, "discovery report staged without consent");
            require(!LittleFS.exists("/email_outbox.json"), "outbox created without consent");
        }},
        {"discovery report records owner and support consent", [] {
            fixture(true, false, false);
            require(EmailReports::stageDiscoveryReport() == EmailReports::Result::Staged, "consented discovery report was not staged");
            JsonDocument document = outbox();
            require(document["ownerEmailConsent"] == true && document["diagnosticEmailConsent"] == false, "category consent was not preserved");
            require(document["shareWithSupport"] == false && document["ownerEmails"][0] == "owner@example.com", "recipient preferences were not preserved");
            require(document["requestId"].as<const char*>() != nullptr && std::strlen(document["requestId"].as<const char*>()) == 16, "request ID missing");
        }},
        {"current inventory report requires explicit owner consent", [] {
            fixture(false, false, false);
            require(EmailReports::stageCurrentInventoryReport() == EmailReports::Result::NotConfigured, "inventory report staged without consent");
            require(!LittleFS.exists("/email_outbox.json"), "inventory outbox created without consent");
        }},
        {"current inventory report is labeled and contains current devices", [] {
            fixture(true, false, false);
            require(EmailReports::stageCurrentInventoryReport() == EmailReports::Result::Staged, "consented inventory report was not staged");
            JsonDocument document = outbox();
            require(document["reportKind"] == "currentInventory", "inventory snapshot was mislabeled");
            require(document["report"]["devices"][0]["name"] == "Galley Light", "current inventory was not included");
            require(document["summary"] == "Current device inventory snapshot; no new learning run was performed.", "inventory snapshot disclosure missing");
        }},
        {"diagnostic report requires diagnostic consent", [] {
            fixture(true, false, false);
            require(EmailReports::stageDiagnosticReport("Bus diagnostics") == EmailReports::Result::NotConfigured, "diagnostic report staged without consent");
            require(!LittleFS.exists("/email_outbox.json"), "diagnostic outbox created without consent");
        }},
        {"diagnostic report records independent consent", [] {
            fixture(false, true, true);
            require(EmailReports::stageDiagnosticReport("Bus diagnostics") == EmailReports::Result::Staged, "consented diagnostic report was not staged");
            JsonDocument document = outbox();
            require(document["ownerEmailConsent"] == false && document["diagnosticEmailConsent"] == true, "diagnostic consent fields are incorrect");
            require(document["shareWithSupport"] == true, "support preference was not preserved");
        }},
        {"pending outbox is never overwritten", [] {
            fixture(true, false, false);
            LittleFS.put("/email_outbox.json", "existing");
            require(EmailReports::stageDiscoveryReport() == EmailReports::Result::PendingReportExists, "pending outbox was replaced");
            require(LittleFS.content("/email_outbox.json") == "existing", "existing outbox content changed");
        }},
        {"empty diagnostic summary is rejected", [] {
            fixture(false, true, false);
            require(EmailReports::stageDiagnosticReport("") == EmailReports::Result::InvalidReport, "empty diagnostic summary accepted");
        }},
        {"relay configuration rejects invalid credentials", [] {
            fixture(true, false, false);
            require(!EmailReports::configureRelay("http://example.com/exec", token), "non-HTTPS endpoint accepted");
            require(!EmailReports::configureRelay(endpoint, "short"), "short relay token accepted");
            require(!EmailReports::relayConfigured(), "invalid relay configuration persisted");
        }},
        {"acknowledged relay delivery removes outbox", [] {
            fixture(true, false, false); configureRelay(); putPendingReport();
            fakeHttp.responses.push_back({302, "", "https://script.googleusercontent.com/macros/echo?key=abc"});
            fakeHttp.responses.push_back({200, R"({"ok":true,"code":"accepted"})", ""});
            require(EmailReports::sendPendingReport() == EmailReports::Result::Delivered, "successful relay response was not accepted");
            require(!LittleFS.exists("/email_outbox.json"), "acknowledged outbox was retained");
            require(fakeHttp.caBundleUsed, "TLS used no trusted root bundle");
            require(fakeHttp.requests.size() == 2 && fakeHttp.requests[0].method == "POST" && fakeHttp.requests[1].method == "GET", "Apps Script redirect flow was incorrect");
            require(std::strstr(fakeHttp.requests[0].body.c_str(), token.c_str()) != nullptr, "relay token missing from request");
            require(LittleFS.content("/coach.json").find(token.c_str()) == std::string::npos, "relay token leaked into coach JSON");
        }},
        {"untrusted redirect retains outbox", [] {
            fixture(true, false, false); configureRelay(); putPendingReport();
            fakeHttp.responses.push_back({302, "", "https://attacker.example/collect"});
            require(EmailReports::sendPendingReport() == EmailReports::Result::Rejected, "untrusted redirect accepted");
            require(LittleFS.exists("/email_outbox.json"), "untrusted redirect removed outbox");
        }},
        {"uncertain delivery retains outbox", [] {
            fixture(true, false, false); configureRelay(); putPendingReport();
            fakeHttp.responses.push_back({200, R"({"ok":false,"code":"delivery_uncertain","retryable":false})", ""});
            require(EmailReports::sendPendingReport() == EmailReports::Result::DeliveryUncertain, "uncertain delivery state was misclassified");
            require(LittleFS.exists("/email_outbox.json"), "uncertain delivery removed outbox");
        }},
        {"HTTP failure retains outbox for retry", [] {
            fixture(true, false, false); configureRelay(); putPendingReport();
            fakeHttp.responses.push_back({-1, "", ""});
            require(EmailReports::sendPendingReport() == EmailReports::Result::RetryableFailure, "HTTP failure was not retryable");
            require(LittleFS.exists("/email_outbox.json"), "HTTP failure removed outbox");
        }},
        {"sending without a pending report is explicit", [] {
            fixture(true, false, false); configureRelay();
            require(EmailReports::sendPendingReport() == EmailReports::Result::NoPendingReport, "missing outbox was misclassified");
        }}
    };
    unsigned failures = 0;
    for (const auto& test : tests) {
        try { test.second(); std::cout << "PASS: " << test.first << '\n'; }
        catch (const std::exception& error) { ++failures; std::cout << "FAIL: " << test.first << ": " << error.what() << '\n'; }
    }
    std::cout << tests.size() - failures << '/' << tests.size() << " email staging tests passed\n";
    return failures == 0 ? 0 : 1;
}