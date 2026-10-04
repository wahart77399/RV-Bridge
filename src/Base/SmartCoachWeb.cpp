#include "SmartCoachWeb.h"
#include "WifiCredentials.h"
#include "DeviceFactory.h"
#include "LearnMode.h"
#include <ArduinoJson.h>
#include <cstring>
#include "BridgeDiagnostics.h"
#include "DiagnosticsPortal.h"
#include "EmailReports.h"

SmartCoachWebServer* SmartCoachWebServer::s_instance = nullptr;

static bool isValidCoachText(JsonVariant value, size_t maxLength)
{
    bool valid = value.is<const char*>();
    if (valid) valid = std::strlen(value.as<const char*>()) <= maxLength;
    return valid;
}

static bool isValidOwnerEmail(const char* address)
{
    bool valid = address != nullptr;
    if (valid) {
        size_t length = std::strlen(address);
        const char* at = std::strchr(address, '@');
        valid = length > 3 && length <= 254 && at != nullptr && at != address &&
                std::strchr(at + 1, '@') == nullptr && std::strchr(at + 2, '.') != nullptr;
        for (size_t index = 0; valid && index < length; ++index) {
            unsigned char character = static_cast<unsigned char>(address[index]);
            if (character <= 32 || character == ',' || character == ';' || character == '<' || character == '>') valid = false;
        }
    }
    return valid;
}

namespace {
    constexpr const char* PORTAL_DIR = "/SmartCoachDevicePortal";
    constexpr const char* EMAIL_PORTAL_HTML = R"HTML(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>SmartCoach Email Reports</title><style>body{margin:0;background:#eef2eb;color:#18392f;font:16px system-ui,sans-serif}main{max-width:620px;margin:5vh auto;padding:24px;background:#fff;border:1px solid #cbd7ca;border-radius:6px}h1{font-size:24px;margin:0 0 20px}h2{font-size:16px;margin:24px 0 12px}label{display:flex;align-items:center;gap:10px;margin:14px 0}input:not([type=checkbox]){box-sizing:border-box;width:100%;padding:11px;border:1px solid #9bac9e;border-radius:4px;font:inherit}input[type=checkbox]{width:18px;height:18px}button{padding:10px 14px;margin:8px 8px 0 0;border:1px solid #315e4b;border-radius:4px;background:#315e4b;color:#fff;font:inherit;cursor:pointer}button:disabled{opacity:.6}p{line-height:1.5;color:#52675c;overflow-wrap:anywhere}#message{min-height:1.5em;color:#315e4b}#error{color:#a4493d}</style></head><body><main><h1>Email reports</h1><p>Recipients: <span id="recipients">Loading</span></p><form id="settings"><label><input id="ownerConsent" type="checkbox"> Email me the discovery report when learning completes</label><label><input id="diagnosticConsent" type="checkbox"> Email diagnostic reports to owner addresses</label><label><input id="supportConsent" type="checkbox"> Allow opted-in reports to be copied to support</label><h2>Gmail relay</h2><label for="endpoint">Google Apps Script web app URL</label><input id="endpoint" type="url" maxlength="256" placeholder="https://script.google.com/macros/s/.../exec"><label for="token">Per-coach relay token (64 hexadecimal characters)</label><input id="token" type="password" maxlength="64" pattern="[0-9a-fA-F]{64}" autocomplete="new-password"><p>This is not your Gmail password. Use this page on trusted coach Wi-Fi; the relay token is stored on the bridge in NVS.</p><button id="save" type="submit">Save settings</button></form><button id="stage" type="button" hidden>Stage latest discovery report</button><button id="send" type="button" hidden>Send pending report</button><p id="message" role="status"></p><p id="error" role="alert"></p></main><script>const byId=id=>document.getElementById(id);async function refresh(){try{const responses=await Promise.all([fetch('/coach.json',{cache:'no-store'}),fetch('/status',{cache:'no-store'})]);if(!responses[0].ok||!responses[1].ok)throw new Error('Bridge status unavailable');const coach=await responses[0].json();const status=await responses[1].json();byId('recipients').textContent=Array.isArray(coach.ownerEmails)?coach.ownerEmails.join(', '):'No owner addresses configured';byId('ownerConsent').checked=Boolean(coach.emailReports);byId('diagnosticConsent').checked=Boolean(coach.diagnosticEmails);byId('supportConsent').checked=Boolean(coach.shareWithSupport);byId('message').textContent=status.email_relay_configured?'Relay configured':'Relay not configured';byId('stage').hidden=!(status.email_report_available&&coach.emailReports&&!status.email_report_pending);byId('send').hidden=!(status.email_relay_configured&&status.email_report_pending)}catch(error){byId('error').textContent=error.message}}byId('settings').addEventListener('submit',async event=>{event.preventDefault();const endpoint=byId('endpoint').value.trim();const token=byId('token').value.trim();if(Boolean(endpoint)!==Boolean(token)){byId('error').textContent='Enter both relay fields, or leave both blank.'}else{byId('save').disabled=true;byId('error').textContent='';try{const response=await fetch('/email/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({emailReports:byId('ownerConsent').checked,diagnosticEmails:byId('diagnosticConsent').checked,shareWithSupport:byId('supportConsent').checked,endpoint,token})});const result=await response.json();if(!response.ok)throw new Error(result.error||'Could not save settings');byId('token').value='';byId('message').textContent='Settings saved';await refresh()}catch(error){byId('error').textContent=error.message}finally{byId('save').disabled=false}}});byId('stage').addEventListener('click',async()=>{byId('stage').disabled=true;try{const response=await fetch('/email/stage',{method:'POST'});const result=await response.json();if(!response.ok)throw new Error(result.result||'Could not stage report');byId('message').textContent='Report staged';await refresh()}catch(error){byId('error').textContent=error.message}finally{byId('stage').disabled=false}});byId('send').addEventListener('click',async()=>{if(confirm('Send the pending report to the listed owner addresses?')){byId('send').disabled=true;try{const response=await fetch('/email/send',{method:'POST'});const result=await response.json();if(!response.ok)throw new Error(result.result==='delivery_uncertain'?'Delivery is uncertain. Check Gmail before retrying.':result.result||'Delivery failed');byId('message').textContent='Report sent';await refresh()}catch(error){byId('error').textContent=error.message}finally{byId('send').disabled=false}}});refresh();</script></body></html>)HTML";

    const char* validateCoach(JsonObject coach) {
        const char* error = nullptr;
        do {
            if (coach.isNull() || !coach["year"].is<int>() || coach["year"].as<int>() < 1900 || coach["year"].as<int>() > 2200 ||
                !isValidCoachText(coach["make"], 64) || !isValidCoachText(coach["model"], 64) ||
                !isValidCoachText(coach["floorplan"], 64) || !isValidCoachText(coach["coachId"], 64) ||
                !coach["tanks"].is<JsonArray>() || !coach["coverTimes"].is<JsonObject>() || !coach["batteries"].is<JsonArray>()) {
                error = "Invalid coach configuration fields";
                break;
            }
            bool tankInstances[256] = {};
            for (JsonObject tank : coach["tanks"].as<JsonArray>()) {
                if (!tank["instance"].is<int>() || !tank["capacityLiters"].is<int>() || !isValidCoachText(tank["name"], 64)) {
                    error = "Invalid tank details";
                    break;
                }
                int instance = tank["instance"].as<int>();
                int capacity = tank["capacityLiters"].as<int>();
                const char* name = tank["name"].as<const char*>();
                if (instance < 0 || instance > 255 || tankInstances[instance] || capacity < 0 || capacity > 65535 || name[0] == '\0') {
                    error = "Invalid tank details";
                    break;
                }
                tankInstances[instance] = true;
            }
            if (error != nullptr) break;
            for (JsonPair coverPair : coach["coverTimes"].as<JsonObject>()) {
                JsonObject timing = coverPair.value().as<JsonObject>();
                if (timing.isNull() || !timing["extendSec"].is<int>() || !timing["retractSec"].is<int>()) {
                    error = "Invalid cover timing details";
                    break;
                }
                int extendSeconds = timing["extendSec"].as<int>();
                int retractSeconds = timing["retractSec"].as<int>();
                if (extendSeconds < 1 || extendSeconds > 65535 || retractSeconds < 1 || retractSeconds > 65535) {
                    error = "Cover times must be between 1 and 65535 seconds";
                    break;
                }
            }
            if (error != nullptr) break;
            bool batteryInstances[256] = {};
            for (JsonObject battery : coach["batteries"].as<JsonArray>()) {
                if (!battery["instance"].is<int>() || !battery["nominalVoltage"].is<float>() || !battery["capacityAh"].is<float>() || !isValidCoachText(battery["name"], 64)) {
                    error = "Invalid battery details";
                    break;
                }
                int instance = battery["instance"].as<int>();
                float nominalVoltage = battery["nominalVoltage"].as<float>();
                float capacityAh = battery["capacityAh"].as<float>();
                const char* name = battery["name"].as<const char*>();
                if (instance < 0 || instance > 255 || batteryInstances[instance] || nominalVoltage <= 0 || nominalVoltage > 1000 || capacityAh < 0 || capacityAh > 100000 || name[0] == '\0') {
                    error = "Invalid battery details";
                    break;
                }
                batteryInstances[instance] = true;
            }
        } while (false);
        return error;
    }

    const char* persistConfiguration(const char* path, JsonDocument& document) {
        const char* error = nullptr;
        String temporaryPath = String(path) + ".web.tmp";
        do {
            if (document.overflowed()) {
                error = "Insufficient memory for configuration";
                break;
            }
            File output = LittleFS.open(temporaryPath, "w");
            if (!output) {
                error = "Could not write configuration";
                break;
            }
            size_t expected = measureJson(document);
            size_t written = serializeJson(document, output);
            output.flush();
            output.close();
            if (written != expected) {
                error = "Incomplete configuration write";
                break;
            }
            if (!LittleFS.rename(temporaryPath, path)) error = "Could not replace configuration";
        } while (false);
        if (error != nullptr) LittleFS.remove(temporaryPath);
        return error;
    }

    class ScopedHeapBaseline {
    public:
        explicit ScopedHeapBaseline(const char* operation) : operation_(operation) {}
        ScopedHeapBaseline(const ScopedHeapBaseline&) = delete;
        ScopedHeapBaseline& operator=(const ScopedHeapBaseline&) = delete;
        ScopedHeapBaseline(ScopedHeapBaseline&&) = delete;
        ScopedHeapBaseline& operator=(ScopedHeapBaseline&&) = delete;
        ~ScopedHeapBaseline() {
            Serial.printf("SmartCoachWeb: %s freeHeap=%lu minFreeHeap=%lu\n", operation_,
                          static_cast<unsigned long>(ESP.getFreeHeap()),
                          static_cast<unsigned long>(ESP.getMinFreeHeap()));
        }
    private:
        const char* operation_;
    };
}

// -----------------------------------------------------------------
// Singleton accessor
// -----------------------------------------------------------------
SmartCoachWebServer& SmartCoachWebServer::instance(uint16_t port)
{
    if (s_instance == nullptr) {
        s_instance = new SmartCoachWebServer(port);
    }
    return *s_instance;
}

// -----------------------------------------------------------------
// Private constructor
// -----------------------------------------------------------------
SmartCoachWebServer::SmartCoachWebServer(uint16_t port)
    : m_server(port)
    , m_running(false)
{
}

// -----------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------
SmartCoachWebServer::~SmartCoachWebServer()
{
    if (m_running) {
        m_server.stop();
    }
    // Note: s_instance is left as-is; the process is ending or the
    // object is being destroyed intentionally.  In an embedded
    // system we normally never delete the singleton.
}

// -----------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------
void SmartCoachWebServer::begin()
{
    if (!m_running) {
        mountFilesystem();
        installBaseRoutes();
        registerAdditionalRoutes();
        m_server.begin();
        m_running = true;
    }
}

void SmartCoachWebServer::handleClient()
{
    if (m_running) {
        m_server.handleClient();
    }
}

bool SmartCoachWebServer::isRunning() const
{
    return m_running;
}

// -----------------------------------------------------------------
// Protected
// -----------------------------------------------------------------
void SmartCoachWebServer::registerAdditionalRoutes()
{
    m_server.on("/diagnostics-page", HTTP_GET, [this]() {
        ScopedHeapBaseline heapBaseline("diagnostics-page");
        m_server.sendHeader("Cache-Control", "no-store");
        m_server.send(200, "text/html", SMARTCOACH_DIAGNOSTICS_PAGE);
    });
    m_server.on("/diagnostics", HTTP_GET, [this]() {
        ScopedHeapBaseline heapBaseline("diagnostics-report");
        String report = BridgeDiagnostics::reportJson();
        if (report.isEmpty()) {
            m_server.send(503, "application/json", "{\"error\":\"Diagnostics unavailable\"}");
        } else {
            m_server.sendHeader("Cache-Control", "no-store");
            m_server.sendHeader("Content-Disposition", "attachment; filename=smartcoach-diagnostics.json");
            m_server.send(200, "application/json", report);
        }
    });
    serveStaticFile("/style.css", "/SmartCoachDevicePortal/style.css", "text/css");
    serveStaticFile("/smartcoach-mark.svg", "/SmartCoachDevicePortal/smartcoach-mark.svg", "image/svg+xml");
    serveStaticFile("/devices.json", "/devices.json", "application/json");
    serveStaticFile("/coach.json", "/coach.json", "application/json");
    serveStaticFile("/learn_report.json", "/learn_report.json", "application/json");
    m_server.on("/devices/rename", HTTP_POST, [this]() { handleRenameDevice(); });
    m_server.on("/devices/review", HTTP_POST, [this]() { handleReviewDevice(); });
    m_server.on("/discovery", HTTP_GET, [this]() {
        ScopedHeapBaseline heapBaseline("discovery-report");
        String report = LearnMode::discoveryJson();
        m_server.sendHeader("Cache-Control", "no-store");
        if (report.isEmpty()) m_server.send(503, "application/json", "{\"error\":\"Discovery unavailable\"}");
        else m_server.send(200, "application/json", report);
    });
    m_server.on("/coach/update", HTTP_POST, [this]() { handleCoachUpdate(); });
    m_server.on("/wifi/reset", HTTP_POST, [this]() { handleWifiReset(); });
    m_server.on("/reboot", HTTP_POST, [this]() { handleReboot(); });
    m_server.on("/email/settings", HTTP_POST, [this]() { handleEmailSettings(); });
    m_server.on("/email", HTTP_GET, [this]() { handleEmailPortal(); });
    m_server.on("/email/stage", HTTP_POST, [this]() { handleEmailStage(); });
    m_server.on("/email/send", HTTP_POST, [this]() { handleEmailSend(); });
}

void SmartCoachWebServer::serveStaticFile(const char* uri,
                                          const char* path,
                                          const char* contentType)
{
    m_server.on(uri, HTTP_GET, [this, path, contentType]() {
        if (LittleFS.exists(path)) {
            File f = LittleFS.open(path, "r");
            m_server.streamFile(f, contentType);
            f.close();
        } else {
            m_server.send(404, "text/plain", "File not found");
        }
    });
}

void SmartCoachWebServer::handleRenameDevice()
{
    ScopedHeapBaseline heapBaseline("rename-device");
    int code = 400;
    const char* error = "Invalid request body";
    do {
        JsonDocument request;
        if (deserializeJson(request, m_server.arg("plain"))) break;
        const char* type = request["type"] | "";
        const char* name = request["name"] | "";
        const char* room = request["room"] | "";
        int sourceAddress = request["sourceAddress"] | -1;
        int rvcIndex = request["rvcIndex"] | -1;
        error = "Invalid rename details";
        if (type[0] == '\0' || name[0] == '\0' || std::strlen(name) > 64 || std::strlen(room) > 64 ||
            (!request["room"].isNull() && !request["room"].is<const char*>()) ||
            sourceAddress < 0 || sourceAddress > 255 || rvcIndex < 0 || rvcIndex > 255) break;

        File input = LittleFS.open("/devices.json", "r");
        code = 404;
        error = "Device configuration not found";
        if (!input) break;
        JsonDocument devicesDocument;
        DeserializationError readError = deserializeJson(devicesDocument, input);
        input.close();
        JsonArray devices = devicesDocument.as<JsonArray>();
        code = 500;
        error = "Could not read device configuration";
        if (readError || devices.isNull()) break;

        JsonObject target;
        for (JsonObject device : devices) {
            if (device["type"] == type && device["sourceAddress"].as<int>() == sourceAddress && device["rvcIndex"].as<int>() == rvcIndex) {
                target = device;
                break;
            }
        }
        code = 404;
        error = "Device not found";
        if (target.isNull()) break;
        target["name"] = name;
        if (request["room"].is<const char*>()) target["room"] = room;
        code = 500;
        error = persistConfiguration("/devices.json", devicesDocument);
        if (error == nullptr) code = 200;
    } while (false);
    JsonDocument response;
    if (error != nullptr) response["error"] = error;
    else response["saved"] = true;
    String body;
    serializeJson(response, body);
    m_server.send(code, "application/json", body);
}

void SmartCoachWebServer::handleWifiReset()
{
    if (!WifiCredentials::clear()) {
        m_server.send(500, "application/json", "{\"error\":\"Could not clear Wi-Fi credentials\"}");
    } else {
        m_server.send(200, "application/json", "{\"reset\":true}");
        delay(500);
        ESP.restart();
    }
}

void SmartCoachWebServer::handleReviewDevice()
{
    ScopedHeapBaseline heapBaseline("review-device");
    int code = 400;
    const char* error = "Invalid review request";
    do {
        if (LearnMode::isLearning()) {
            code = 409;
            error = "Finish or cancel learning before reviewing devices";
            break;
        }
        JsonDocument request;
        if (deserializeJson(request, m_server.arg("plain"))) break;
        const char* action = request["action"] | "";
        bool approve = std::strcmp(action, "approve") == 0;
        bool ignore = std::strcmp(action, "ignore") == 0;
        bool reopen = std::strcmp(action, "reopen") == 0;
        const char* type = request["type"] | "";
        int index = request["rvcIndex"] | -1;
        int address = request["sourceAddress"] | -1;
        if ((!approve && !ignore && !reopen) || type[0] == '\0' || index < 0 || index > 255 || address < 0 || address > 255) break;
        if (approve && !(request["confirmed"].is<bool>() && request["confirmed"].as<bool>())) {
            error = "Confirm the device identity before approval";
            break;
        }
        if ((!request["name"].isNull() && !request["name"].is<const char*>()) ||
            (!request["room"].isNull() && !request["room"].is<const char*>())) break;

        File input = LittleFS.open("/devices.json", "r");
        code = 404;
        error = "Device configuration not found";
        if (!input) break;
        JsonDocument document;
        DeserializationError readError = deserializeJson(document, input);
        input.close();
        JsonArray devices = document.as<JsonArray>();
        code = 500;
        error = "Could not read device configuration";
        if (readError || devices.isNull()) break;
        JsonObject target;
        for (JsonObject device : devices) {
            if (device["type"] == type && device["rvcIndex"].as<int>() == index && device["sourceAddress"].as<int>() == address) {
                target = device;
                break;
            }
        }
        code = 404;
        error = "Device not found";
        if (target.isNull()) break;
        bool wasEnabled = target["enabled"] | false;
        if (wasEnabled && !approve) {
            code = 409;
            error = "Review actions cannot disable an enabled HomeKit device";
            break;
        }
        const char* name = request["name"] | (target["name"] | "");
        const char* room = request["room"] | (target["room"] | "");
        code = 400;
        error = "Invalid device name or room";
        if (name[0] == '\0' || std::strlen(name) > 64 || std::strlen(room) > 64) break;
        target["name"] = name;
        target["room"] = room;
        target["enabled"] = approve;
        target["ignored"] = ignore;
        code = 409;
        error = "Cannot reserve configuration: unsupported identity, AID conflict, accessory limit or storage failure";
        if (!DeviceFactory::validateAndReserveConfiguration(document)) break;
        code = 500;
        error = persistConfiguration("/devices.json", document);
        if (error != nullptr) break;
        if (approve && !wasEnabled) m_restartRequired = true;
        code = 200;
    } while (false);
    JsonDocument response;
    if (error != nullptr) response["error"] = error;
    else {
        response["saved"] = true;
        response["restartRequired"] = m_restartRequired;
    }
    String body;
    serializeJson(response, body);
    m_server.send(code, "application/json", body);
}

void SmartCoachWebServer::handleCoachUpdate()
{
    ScopedHeapBaseline heapBaseline("coach-update");
    int code = 400;
    const char* error = "Invalid coach configuration";
    do {
        JsonDocument request;
        if (deserializeJson(request, m_server.arg("plain"))) break;
        JsonObject coach = request.as<JsonObject>();
        error = validateCoach(coach);
        if (error != nullptr) break;

        File input = LittleFS.open("/coach.json", "r");
        code = 404;
        error = "Coach configuration not found";
        if (!input) break;
        JsonDocument document;
        DeserializationError readError = deserializeJson(document, input);
        input.close();
        code = 500;
        error = "Could not read coach configuration";
        if (readError || !document.is<JsonObject>()) break;
        for (const char* field : {"year", "make", "model", "floorplan", "coachId", "tanks", "coverTimes", "batteries"}) {
            document[field].set(coach[field]);
        }
        error = persistConfiguration("/coach.json", document);
        if (error == nullptr) code = 200;
    } while (false);
    JsonDocument response;
    if (error != nullptr) response["error"] = error;
    else response["saved"] = true;
    String body;
    serializeJson(response, body);
    m_server.send(code, "application/json", body);
}

void SmartCoachWebServer::handleEmailSettings()
{
    ScopedHeapBaseline heapBaseline("email-settings");
    int code = 400;
    const char* error = "Invalid email settings";
    bool saved = false;
    do {
        JsonDocument request;
        if (deserializeJson(request, m_server.arg("plain")) || !request.is<JsonObject>() ||
            !request["emailReports"].is<bool>() || !request["diagnosticEmails"].is<bool>() ||
            !request["shareWithSupport"].is<bool>() || !request["endpoint"].is<const char*>() ||
            !request["token"].is<const char*>() || !request["ownerEmails"].is<JsonArray>()) break;
        JsonArrayConst ownerEmails = request["ownerEmails"].as<JsonArrayConst>();
        code = 400;
        error = "Enter 1 to 5 valid owner email addresses";
        if (ownerEmails.isNull() || ownerEmails.size() == 0 || ownerEmails.size() > 5) break;
        bool addressesValid = true;
        for (JsonVariantConst address : ownerEmails) {
            if (!address.is<const char*>() || !isValidOwnerEmail(address.as<const char*>())) {
                addressesValid = false;
                break;
            }
        }
        if (!addressesValid) break;
        String endpoint = request["endpoint"].as<String>();
        String token = request["token"].as<String>();
        if (endpoint.isEmpty() != token.isEmpty()) {
            error = "Enter both the Apps Script URL and relay token";
            break;
        }
        File input = LittleFS.open("/coach.json", "r");
        code = 404;
        error = "Coach configuration not found";
        if (!input) break;
        JsonDocument coach;
        DeserializationError readError = deserializeJson(coach, input);
        input.close();
        code = 500;
        error = "Could not read coach configuration";
        if (readError || !coach.is<JsonObject>()) break;
        if (!endpoint.isEmpty() && !EmailReports::configureRelay(endpoint, token)) {
            code = 400;
            error = "Invalid relay URL or 64-character hexadecimal token";
            break;
        }
        coach["emailReports"] = request["emailReports"].as<bool>();
        coach["diagnosticEmails"] = request["diagnosticEmails"].as<bool>();
        coach["shareWithSupport"] = request["shareWithSupport"].as<bool>();
        JsonArray storedOwners = coach["ownerEmails"].to<JsonArray>();
        for (JsonVariantConst address : ownerEmails) storedOwners.add(address.as<const char*>());
        error = persistConfiguration("/coach.json", coach);
        if (error != nullptr) break;
        code = 200;
        error = nullptr;
        saved = true;
    } while (false);
    JsonDocument response;
    if (!saved) response["error"] = error;
    else {
        response["saved"] = true;
        response["relayConfigured"] = EmailReports::relayConfigured();
    }
    String body;
    serializeJson(response, body);
    m_server.send(code, "application/json", body);
}

void SmartCoachWebServer::handleEmailPortal()
{
    String html = EMAIL_PORTAL_HTML;
    html.replace("<p>Recipients: <span id=\"recipients\">Loading</span></p>", "<label for=\"ownerEmails\">Owner email addresses (comma-separated)</label><input id=\"ownerEmails\" type=\"text\" maxlength=\"1024\" placeholder=\"owner@example.com\">");
    html.replace("byId('recipients').textContent=Array.isArray(coach.ownerEmails)?coach.ownerEmails.join(', '):'No owner addresses configured';", "byId('ownerEmails').value=Array.isArray(coach.ownerEmails)?coach.ownerEmails.join(', '):'';");
    html.replace("shareWithSupport:byId('supportConsent').checked,endpoint", "ownerEmails:byId('ownerEmails').value.split(',').map(function(address){return address.trim()}).filter(function(address){return address.length>0}),shareWithSupport:byId('supportConsent').checked,endpoint");
    if (!LittleFS.exists("/learn_report.json")) html.replace("Stage latest discovery report", "Stage current inventory report");
    m_server.send(200, "text/html", html);
}

void SmartCoachWebServer::handleEmailSend()
{
    ScopedHeapBaseline heapBaseline("email-send");
    EmailReports::Result result = EmailReports::sendPendingReport();
    int code = 500;
    const char* label = "storage_error";
    bool retryable = false;
    switch (result) {
        case EmailReports::Result::Delivered: code = 200; label = "sent"; break;
        case EmailReports::Result::NoPendingReport: code = 409; label = "no_pending_report"; break;
        case EmailReports::Result::NotConfigured: code = 409; label = "relay_not_configured"; break;
        case EmailReports::Result::RetryableFailure: code = 503; label = "retryable_failure"; retryable = true; break;
        case EmailReports::Result::DeliveryUncertain: code = 409; label = "delivery_uncertain"; break;
        case EmailReports::Result::Rejected: code = 422; label = "relay_rejected"; break;
        case EmailReports::Result::InvalidReport: code = 422; label = "invalid_report"; break;
        case EmailReports::Result::StorageError: code = 500; label = "storage_error"; break;
        case EmailReports::Result::Staged: code = 409; label = "not_sent"; break;
        case EmailReports::Result::PendingReportExists: code = 409; label = "not_sent"; break;
    }
    JsonDocument response;
    response["sent"] = result == EmailReports::Result::Delivered;
    response["result"] = label;
    response["retryable"] = retryable;
    String body;
    serializeJson(response, body);
    m_server.send(code, "application/json", body);
}

void SmartCoachWebServer::handleEmailStage()
{
    ScopedHeapBaseline heapBaseline("email-stage");
    int code = 200;
    const char* result = "staged";
    if (LearnMode::isLearning()) {
        code = 409;
        result = "learning_active";
    } else {
        EmailReports::Result staged = LittleFS.exists("/learn_report.json")
            ? EmailReports::stageDiscoveryReport()
            : EmailReports::stageCurrentInventoryReport();
        if (staged != EmailReports::Result::Staged) {
            code = staged == EmailReports::Result::StorageError ? 500 : 409;
            result = staged == EmailReports::Result::PendingReportExists ? "pending_report_exists" : "report_not_staged";
        }
    }
    JsonDocument response;
    response["staged"] = code == 200;
    response["result"] = result;
    String body;
    serializeJson(response, body);
    m_server.send(code, "application/json", body);
}

void SmartCoachWebServer::handleReboot()
{
    m_server.send(200, "application/json", "{\"rebooting\":true}");
    delay(500);
    ESP.restart();
}

// -----------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------
void SmartCoachWebServer::mountFilesystem()
{
    // never format here: that would wipe devices.json and the learn state
    LittleFS.begin(false);
}

void SmartCoachWebServer::installBaseRoutes()
{
    m_server.on("/",       HTTP_GET, handleRootWrapper);
    m_server.on("/status", HTTP_GET, handleStatusWrapper);
    m_server.onNotFound(handleNotFoundWrapper);
}

// -----------------------------------------------------------------
// Static wrappers
// -----------------------------------------------------------------
void SmartCoachWebServer::handleRootWrapper()
{
    if (s_instance) s_instance->handleRoot();
}

void SmartCoachWebServer::handleStatusWrapper()
{
    if (s_instance) s_instance->handleStatus();
}

void SmartCoachWebServer::handleNotFoundWrapper()
{
    if (s_instance) s_instance->handleNotFound();
}

// -----------------------------------------------------------------
// Real handlers
// -----------------------------------------------------------------
void SmartCoachWebServer::handleRoot()
{
    ScopedHeapBaseline heapBaseline("root-page");
    String indexPath = String(PORTAL_DIR) + "/index.html";
    if (LittleFS.exists(indexPath)) {
        File f = LittleFS.open(indexPath, "r");
        constexpr size_t MAX_PORTAL_HTML_BYTES = 65536;
        constexpr size_t MIN_FREE_HEAP_FOR_PORTAL_BUFFER = 16384;
        constexpr const char* DIAGNOSTICS_LINK = "<a class=\"footer-legal-link\" href=\"/diagnostics-page\">Diagnostics</a>";
        constexpr const char* EMAIL_LINK = "<a class=\"footer-legal-link\" href=\"/email\">Email reports</a>";
        size_t fileSize = f.size();
        bool served = false;
        if (fileSize <= MAX_PORTAL_HTML_BYTES &&
            ESP.getFreeHeap() >= fileSize + MIN_FREE_HEAP_FOR_PORTAL_BUFFER) {
            String html;
            bool complete = html.reserve(fileSize + std::strlen(DIAGNOSTICS_LINK) + std::strlen(EMAIL_LINK) + 2);
            char buffer[512];
            while (complete && f.available()) {
                size_t count = f.readBytes(buffer, sizeof(buffer));
                if (count == 0) break;
                complete = html.concat(buffer, count);
            }
            complete = complete && html.length() == fileSize;
            if (complete) {
                String links;
                if (html.indexOf("id=\"diagnostics-open\"") < 0 && html.indexOf("href=\"/diagnostics-page\"") < 0) links += DIAGNOSTICS_LINK;
                if (html.indexOf("href=\"/email\"") < 0) links += EMAIL_LINK;
                if (!links.isEmpty()) {
                    if (html.indexOf("</footer>") >= 0) html.replace("</footer>", links + "</footer>");
                    else if (html.indexOf("</body>") >= 0) html.replace("</body>", links + "</body>");
                }
                m_server.send(200, "text/html", html);
                served = true;
            }
        }
        if (!served) {
            f.seek(0);
            m_server.streamFile(f, "text/html");
        }
        f.close();
    } else {
        m_server.send(200, "text/html",
            "<!DOCTYPE html><html><body>"
            "<h1>SmartCoach</h1>"
            "<p>Device portal files are missing from the filesystem image.</p>"
            "</body></html>");
    }
}

void SmartCoachWebServer::handleStatus()
{
    char json[384];
    String ip = WiFi.localIP().toString();
    bool learningReportAvailable = LittleFS.exists("/learn_report.json");
    bool currentInventoryAvailable = LittleFS.exists("/devices.json");
    bool reportAvailable = !LearnMode::isLearning() && (learningReportAvailable || currentInventoryAvailable);
    bool emailReportPending = LittleFS.exists("/email_outbox.json");
    bool emailRelayConfigured = EmailReports::relayConfigured();
    int length = snprintf(json, sizeof(json),
        "{\"ip\":\"%s\",\"rssi\":%d,\"free_heap\":%lu,\"uptime_ms\":%lu,"
        "\"restart_required\":%s,\"diagnostics_available\":true,"
        "\"email_reports_available\":true,\"email_relay_configured\":%s,"
        "\"email_report_pending\":%s,\"email_report_available\":%s,"
        "\"email_learning_report_available\":%s}",
        ip.c_str(), WiFi.RSSI(), static_cast<unsigned long>(ESP.getFreeHeap()),
        static_cast<unsigned long>(millis()), m_restartRequired ? "true" : "false",
        emailRelayConfigured ? "true" : "false", emailReportPending ? "true" : "false",
        reportAvailable ? "true" : "false",
        (!LearnMode::isLearning() && learningReportAvailable) ? "true" : "false");
    if (length >= 0 && static_cast<size_t>(length) < sizeof(json)) m_server.send(200, "application/json", json);
    else m_server.send(500, "application/json", "{\"error\":\"Status unavailable\"}");
}

void SmartCoachWebServer::handleNotFound()
{
    m_server.send(404, "text/plain", "Not found");
}