#include "SmartCoachWeb.h"
#include "WifiCredentials.h"
#include <ArduinoJson.h>
#include <cstring>
#ifdef FUTURE
#include "BridgeDiagnostics.h"
#endif

SmartCoachWebServer* SmartCoachWebServer::s_instance = nullptr;

static bool isValidCoachText(JsonVariant value, size_t maxLength)
{
    bool valid = value.is<const char*>();
    if (valid) valid = std::strlen(value.as<const char*>()) <= maxLength;
    return valid;
}

namespace {
    constexpr const char* PORTAL_DIR = "/SmartCoachDevicePortal";

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
    : m_port(port)
    , m_server(port)
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
#ifdef FUTURE
    m_server.on("/diagnostics", HTTP_GET, [this]() {
        String report = BridgeDiagnostics::reportJson();
        if (report.isEmpty()) {
            m_server.send(503, "application/json", "{\"error\":\"Diagnostics unavailable\"}");
        } else {
            m_server.sendHeader("Cache-Control", "no-store");
            m_server.sendHeader("Content-Disposition", "attachment; filename=smartcoach-diagnostics.json");
            m_server.send(200, "application/json", report);
        }
    });
#endif
    serveStaticFile("/style.css", "/SmartCoachDevicePortal/style.css", "text/css");
    serveStaticFile("/smartcoach-mark.svg", "/SmartCoachDevicePortal/smartcoach-mark.svg", "image/svg+xml");
    serveStaticFile("/devices.json", "/devices.json", "application/json");
    serveStaticFile("/coach.json", "/coach.json", "application/json");
    m_server.on("/devices/rename", HTTP_POST, [this]() { handleRenameDevice(); });
    m_server.on("/coach/update", HTTP_POST, [this]() { handleCoachUpdate(); });
    m_server.on("/wifi/reset", HTTP_POST, [this]() { handleWifiReset(); });
    m_server.on("/reboot", HTTP_POST, [this]() { handleReboot(); });
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

void SmartCoachWebServer::handleCoachUpdate()
{
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

void SmartCoachWebServer::handleReboot()
{
    m_server.send(200, "application/json", "{\"rebooting\":true}");
    delay(500);
    ESP.restart();
}

WebServer& SmartCoachWebServer::server()
{
    return m_server;
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
    String indexPath = String(PORTAL_DIR) + "/index.html";
    if (LittleFS.exists(indexPath)) {
        File f = LittleFS.open(indexPath, "r");
        m_server.streamFile(f, "text/html");
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
    String json;
    json.reserve(128);
    json += "{\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\",\"rssi\":";
    json += WiFi.RSSI();
    json += ",\"free_heap\":";
    json += ESP.getFreeHeap();
    json += ",\"uptime_ms\":";
    json += millis();
    json += '}';
    m_server.send(200, "application/json", json);
}

void SmartCoachWebServer::handleNotFound()
{
    m_server.send(404, "text/plain", "Not found");
}