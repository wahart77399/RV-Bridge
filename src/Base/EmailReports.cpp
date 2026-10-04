#include "EmailReports.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <cstring>
#include <esp_random.h>

EmailReports::Result EmailReports::stageDiscoveryReport() {
    return stage("discovery", "/learn_report.json", "");
}

EmailReports::Result EmailReports::stageCurrentInventoryReport() {
    return stage("discovery", "/devices.json", "Current device inventory snapshot; no new learning run was performed.");
}

EmailReports::Result EmailReports::stageDiagnosticReport(const String& summary) {
    Result result = Result::InvalidReport;
    if (!summary.isEmpty() && summary.length() <= 1024) {
        result = stage("diagnostics", nullptr, summary);
    }
    return result;
}

EmailReports::Result EmailReports::stage(const char* category, const char* reportPath, const String& summary) {
    Result result = Result::InvalidReport;
    do {
        if (!LittleFS.begin(false)) {
            result = Result::StorageError;
            break;
        }
        if (LittleFS.exists(OUTBOX_PATH)) {
            result = Result::PendingReportExists;
            break;
        }
        File coachFile = LittleFS.open("/coach.json", "r");
        if (!coachFile || coachFile.size() > MAX_REPORT_BYTES) break;
        JsonDocument coach;
        DeserializationError error = deserializeJson(coach, coachFile);
        coachFile.close();
        if (error || !coach.is<JsonObject>()) break;
        JsonArrayConst owners = coach["ownerEmails"].as<JsonArrayConst>();
        if (owners.isNull() || owners.size() == 0 || owners.size() > 5 ||
            (reportPath != nullptr && !(coach["emailReports"] | false)) ||
            (reportPath == nullptr && !(coach["diagnosticEmails"] | false))) {
            result = Result::NotConfigured;
            break;
        }

        JsonDocument envelope;
        char requestId[17];
        snprintf(requestId, sizeof(requestId), "%08lX%08lX",
                 static_cast<unsigned long>(esp_random()), static_cast<unsigned long>(esp_random()));
        envelope["schemaVersion"] = 1;
        envelope["requestId"] = requestId;
        envelope["category"] = category;
        envelope["ownerEmails"].set(owners);
        envelope["ownerEmailConsent"] = coach["emailReports"] | false;
        envelope["diagnosticEmailConsent"] = coach["diagnosticEmails"] | false;
        envelope["shareWithSupport"] = coach["shareWithSupport"] | false;
        envelope["summary"] = summary;
        bool currentInventory = reportPath != nullptr && std::strcmp(reportPath, "/devices.json") == 0;
        if (reportPath != nullptr) envelope["reportKind"] = currentInventory ? "currentInventory" : "learningDiscovery";
        JsonObject identity = envelope["coach"].to<JsonObject>();
        for (const char* field : {"year", "make", "model", "floorplan", "coachId"}) {
            identity[field].set(coach[field]);
        }
        if (reportPath != nullptr) {
            File reportFile = LittleFS.open(reportPath, "r");
            if (!reportFile || reportFile.size() > MAX_REPORT_BYTES) break;
            JsonDocument report;
            error = deserializeJson(report, reportFile);
            reportFile.close();
            if (error) break;
            if (currentInventory) {
                if (!report.is<JsonArray>()) break;
                JsonDocument inventory;
                inventory["devices"].set(report.as<JsonArrayConst>());
                envelope["report"].set(inventory.as<JsonObjectConst>());
            } else {
                if (!report.is<JsonObject>()) break;
                envelope["report"].set(report.as<JsonObjectConst>());
            }
            envelope["appleHomeInstructions"] = "On the coach Wi-Fi, open Apple Home, Add Accessory, More Options, select SmartCoach, enter its setup code and assign rooms. Keep a supported Apple TV or HomePod on the coach network as a home hub for remote access and automations.";
        }
        size_t expected = measureJson(envelope);
        if (envelope.overflowed() || expected > MAX_REPORT_BYTES) break;
        result = Result::StorageError;
        File output = LittleFS.open("/email_outbox.tmp", "w");
        if (!output) break;
        size_t written = serializeJson(envelope, output);
        output.flush();
        output.close();
        if (written != expected || !LittleFS.rename("/email_outbox.tmp", OUTBOX_PATH)) {
            LittleFS.remove("/email_outbox.tmp");
            break;
        }
        result = Result::Staged;
    } while (false);
    return result;
}

bool EmailReports::configureRelay(const String& endpoint, const String& token) {
    bool configured = false;
    do {
        if (!validEndpoint(endpoint) || !validToken(token)) break;
        JsonDocument document;
        document["endpoint"] = endpoint;
        document["token"] = token;
        String serialized;
        size_t expected = measureJson(document);
        size_t written = serializeJson(document, serialized);
        if (document.overflowed() || written != expected) break;
        Preferences preferences;
        if (!preferences.begin("sc-email", false)) break;
        size_t stored = preferences.putString("relay", serialized);
        preferences.end();
        configured = stored == serialized.length();
    } while (false);
    return configured;
}

bool EmailReports::relayConfigured() {
    String endpoint;
    String token;
    bool configured = loadRelaySettings(endpoint, token);
    return configured;
}

bool EmailReports::loadRelaySettings(String& endpoint, String& token) {
    bool loaded = false;
    do {
        Preferences preferences;
        if (!preferences.begin("sc-email", true)) break;
        String serialized = preferences.getString("relay", "");
        preferences.end();
        if (serialized.isEmpty()) break;
        JsonDocument document;
        if (deserializeJson(document, serialized) || !document["endpoint"].is<const char*>() ||
            !document["token"].is<const char*>()) break;
        endpoint = document["endpoint"].as<String>();
        token = document["token"].as<String>();
        loaded = validEndpoint(endpoint) && validToken(token);
    } while (false);
    return loaded;
}

bool EmailReports::validEndpoint(const String& endpoint) {
    bool valid = endpoint.length() >= 48 && endpoint.length() <= 256 &&
        endpoint.startsWith("https://script.google.com/macros/s/") && endpoint.endsWith("/exec") &&
        endpoint.indexOf('?') < 0 && endpoint.indexOf('#') < 0 && endpoint.indexOf('@') < 0;
    return valid;
}

bool EmailReports::validToken(const String& token) {
    bool valid = token.length() == 64;
    for (size_t index = 0; valid && index < token.length(); ++index) {
        char value = token[index];
        valid = (value >= '0' && value <= '9') || (value >= 'a' && value <= 'f') || (value >= 'A' && value <= 'F');
    }
    return valid;
}

EmailReports::Result EmailReports::postRelayRequest(const String& endpoint, const String& payload) {
    Result result = Result::RetryableFailure;
    do {
        WiFiClientSecure secureClient;
        secureClient.useBuiltinCACertBundle();
        HTTPClient request;
        const char* responseHeaders[] = {"Location"};
        if (!request.begin(secureClient, endpoint)) break;
        request.setTimeout(15000);
        request.setConnectTimeout(10000);
        request.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
        request.collectHeaders(responseHeaders, 1);
        request.addHeader("Content-Type", "application/json");
        int responseCode = request.POST(payload);
        String responseBody;
        if (responseCode == 302 || responseCode == 303) {
            String location = request.header("Location");
            request.end();
            if (!location.startsWith("https://script.googleusercontent.com/macros/echo")) {
                result = Result::Rejected;
                break;
            }
            HTTPClient redirected;
            if (!redirected.begin(secureClient, location)) break;
            redirected.setTimeout(15000);
            redirected.setConnectTimeout(10000);
            responseCode = redirected.GET();
            if (responseCode >= 200 && responseCode < 300) responseBody = redirected.getString();
            redirected.end();
        } else {
            if (responseCode >= 200 && responseCode < 300) responseBody = request.getString();
            request.end();
        }
        if (responseCode < 200 || responseCode >= 300 || responseBody.isEmpty() || responseBody.length() > 1024) break;
        JsonDocument response;
        if (deserializeJson(response, responseBody) || !response["ok"].is<bool>()) break;
        if (response["ok"].as<bool>()) {
            result = Result::Delivered;
        } else {
            const char* code = response["code"] | "";
            bool retryable = response["retryable"] | false;
            if (strcmp(code, "delivery_uncertain") == 0) result = Result::DeliveryUncertain;
            else if (!retryable) result = Result::Rejected;
        }
    } while (false);
    return result;
}

EmailReports::Result EmailReports::sendPendingReport() {
    Result result = Result::NotConfigured;
    String endpoint;
    String token;
    do {
        if (!loadRelaySettings(endpoint, token)) break;
        if (WiFi.status() != WL_CONNECTED || !LittleFS.begin(false)) {
            result = Result::RetryableFailure;
            break;
        }
        File outbox = LittleFS.open(OUTBOX_PATH, "r");
        if (!outbox) {
            result = Result::NoPendingReport;
            break;
        }
        if (outbox.size() > MAX_REPORT_BYTES) {
            outbox.close();
            result = Result::InvalidReport;
            break;
        }
        JsonDocument request;
        DeserializationError error = deserializeJson(request, outbox);
        outbox.close();
        if (error || !request.is<JsonObject>() || request.overflowed()) {
            result = Result::InvalidReport;
            break;
        }
        request["relayToken"] = token;
        size_t expected = measureJson(request);
        if (request.overflowed() || expected == 0 || expected > MAX_REPORT_BYTES) {
            result = Result::InvalidReport;
            break;
        }
        String payload;
        if (!payload.reserve(expected) || serializeJson(request, payload) != expected) {
            result = Result::StorageError;
            break;
        }
        result = postRelayRequest(endpoint, payload);
        if (result == Result::Delivered && !LittleFS.remove(OUTBOX_PATH)) result = Result::StorageError;
    } while (false);
    return result;
}