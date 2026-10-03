#ifdef FUTURE
#include "SmartCoachWeb.h"
#include "WifiCredentials.h"
#include <ArduinoJson.h>
#include <cstring>

SmartCoachWebServer* SmartCoachWebServer::s_instance = nullptr;

static bool isValidCoachText(JsonVariant value, size_t maxLength)
{
    if (!value.is<const char*>()) return false;
    return std::strlen(value.as<const char*>()) <= maxLength;
}

// -----------------------------------------------------------------
// Singleton accessor
// -----------------------------------------------------------------
SmartCoachWebServer& SmartCoachWebServer::instance(const char* ssid,
                                                   const char* password,
                                                   uint16_t    port)
{
    if (s_instance == nullptr) {
        // First call must supply the credentials
        if (ssid == nullptr || password == nullptr) {
            // In production you would log / assert; for now we simply refuse
            while (true) { delay(1000); }   // hard fail – never create an invalid server
        }
        s_instance = new SmartCoachWebServer(ssid, password, port);
    }
    return *s_instance;
}

// -----------------------------------------------------------------
// Private constructor
// -----------------------------------------------------------------
SmartCoachWebServer::SmartCoachWebServer(const char* ssid,
                                         const char* password,
                                         uint16_t    port)
    : m_ssid(ssid)
    , m_password(password)
    , m_port(port)
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
    if (m_running) return;

    connectWiFi();
    mountFilesystem();
    installBaseRoutes();
    registerAdditionalRoutes();

    m_server.begin();
    m_running = true;
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
    JsonDocument request;
    if (deserializeJson(request, m_server.arg("plain"))) {
        m_server.send(400, "application/json", "{\"error\":\"Invalid request body\"}");
        return;
    }

    const char* type = request["type"] | "";
    const char* name = request["name"] | "";
    const int sourceAddress = request["sourceAddress"] | -1;
    const int rvcIndex = request["rvcIndex"] | -1;
    if (type[0] == '\0' || name[0] == '\0' || std::strlen(name) > 64 || sourceAddress < 0 || sourceAddress > 255 || rvcIndex < 0 || rvcIndex > 255) {
        m_server.send(400, "application/json", "{\"error\":\"Invalid rename details\"}");
        return;
    }

    File input = LittleFS.open("/devices.json", "r");
    if (!input) {
        m_server.send(404, "application/json", "{\"error\":\"Device configuration not found\"}");
        return;
    }

    JsonDocument devicesDocument;
    const DeserializationError readError = deserializeJson(devicesDocument, input);
    input.close();
    JsonArray devices = devicesDocument.as<JsonArray>();
    if (readError || devices.isNull()) {
        m_server.send(500, "application/json", "{\"error\":\"Could not read device configuration\"}");
        return;
    }

    JsonObject target;
    for (JsonObject device : devices) {
        if (device["type"] == type && device["sourceAddress"].as<int>() == sourceAddress && device["rvcIndex"].as<int>() == rvcIndex) {
            target = device;
            break;
        }
    }
    if (target.isNull()) {
        m_server.send(404, "application/json", "{\"error\":\"Device not found\"}");
        return;
    }
    target["name"] = name;

    const char* temporaryPath = "/devices.tmp";
    const char* backupPath = "/devices.bak";
    LittleFS.remove(temporaryPath);
    File output = LittleFS.open(temporaryPath, "w");
    if (!output) {
        m_server.send(500, "application/json", "{\"error\":\"Could not write device configuration\"}");
        return;
    }
    const size_t expectedBytes = measureJson(devicesDocument);
    const size_t writtenBytes = serializeJson(devicesDocument, output);
    output.flush();
    output.close();
    if (writtenBytes != expectedBytes) {
        LittleFS.remove(temporaryPath);
        m_server.send(500, "application/json", "{\"error\":\"Incomplete device configuration write\"}");
        return;
    }

    LittleFS.remove(backupPath);
    if (!LittleFS.rename("/devices.json", backupPath)) {
        LittleFS.remove(temporaryPath);
        m_server.send(500, "application/json", "{\"error\":\"Could not stage device configuration\"}");
        return;
    }
    if (!LittleFS.rename(temporaryPath, "/devices.json")) {
        LittleFS.rename(backupPath, "/devices.json");
        LittleFS.remove(temporaryPath);
        m_server.send(500, "application/json", "{\"error\":\"Could not replace device configuration\"}");
        return;
    }
    LittleFS.remove(backupPath);
    m_server.send(200, "application/json", "{\"saved\":true}");
}

void SmartCoachWebServer::handleWifiReset()
{
    if (!WifiCredentials::clear()) {
        m_server.send(500, "application/json", "{\"error\":\"Could not clear Wi-Fi credentials\"}");
        return;
    }

    m_server.send(200, "application/json", "{\"reset\":true}");
    delay(500);
    ESP.restart();
}

void SmartCoachWebServer::handleCoachUpdate()
{
    JsonDocument coachDocument;
    if (deserializeJson(coachDocument, m_server.arg("plain"))) {
        m_server.send(400, "application/json", "{\"error\":\"Invalid coach configuration\"}");
        return;
    }

    JsonObject coach = coachDocument.as<JsonObject>();
    if (coach.isNull() || !coach["year"].is<int>() || coach["year"].as<int>() < 1900 || coach["year"].as<int>() > 2200 ||
        !isValidCoachText(coach["make"], 64) || !isValidCoachText(coach["model"], 64) ||
        !isValidCoachText(coach["floorplan"], 64) || !isValidCoachText(coach["coachId"], 64) ||
        !coach["tanks"].is<JsonArray>() || !coach["coverTimes"].is<JsonObject>() || !coach["batteries"].is<JsonArray>()) {
        m_server.send(400, "application/json", "{\"error\":\"Invalid coach configuration fields\"}");
        return;
    }

    bool tankInstances[256] = {};
    for (JsonObject tank : coach["tanks"].as<JsonArray>()) {
        if (!tank["instance"].is<int>() || !tank["capacityLiters"].is<int>() || !isValidCoachText(tank["name"], 64)) {
            m_server.send(400, "application/json", "{\"error\":\"Invalid tank details\"}");
            return;
        }
        const int instance = tank["instance"].as<int>();
        const int capacity = tank["capacityLiters"].as<int>();
        const char* name = tank["name"].as<const char*>();
        if (instance < 0 || instance > 255 || tankInstances[instance] || capacity < 0 || capacity > 65535 || name[0] == '\0') {
            m_server.send(400, "application/json", "{\"error\":\"Invalid tank details\"}");
            return;
        }
        tankInstances[instance] = true;
    }

    for (JsonPair coverPair : coach["coverTimes"].as<JsonObject>()) {
        JsonObject timing = coverPair.value().as<JsonObject>();
        if (timing.isNull() || !timing["extendSec"].is<int>() || !timing["retractSec"].is<int>()) {
            m_server.send(400, "application/json", "{\"error\":\"Invalid cover timing details\"}");
            return;
        }
        const int extendSeconds = timing["extendSec"].as<int>();
        const int retractSeconds = timing["retractSec"].as<int>();
        if (extendSeconds < 1 || extendSeconds > 65535 || retractSeconds < 1 || retractSeconds > 65535) {
            m_server.send(400, "application/json", "{\"error\":\"Cover times must be between 1 and 65535 seconds\"}");
            return;
        }
    }

    bool batteryInstances[256] = {};
    for (JsonObject battery : coach["batteries"].as<JsonArray>()) {
        if (!battery["instance"].is<int>() || !battery["nominalVoltage"].is<float>() || !battery["capacityAh"].is<float>() || !isValidCoachText(battery["name"], 64)) {
            m_server.send(400, "application/json", "{\"error\":\"Invalid battery details\"}");
            return;
        }
        const int instance = battery["instance"].as<int>();
        const float nominalVoltage = battery["nominalVoltage"].as<float>();
        const float capacityAh = battery["capacityAh"].as<float>();
        const char* name = battery["name"].as<const char*>();
        if (instance < 0 || instance > 255 || batteryInstances[instance] || nominalVoltage <= 0 || nominalVoltage > 1000 || capacityAh < 0 || capacityAh > 100000 || name[0] == '\0') {
            m_server.send(400, "application/json", "{\"error\":\"Invalid battery details\"}");
            return;
        }
        batteryInstances[instance] = true;
    }

    if (!LittleFS.exists("/coach.json")) {
        m_server.send(404, "application/json", "{\"error\":\"Coach configuration not found\"}");
        return;
    }

    const char* temporaryPath = "/coach.tmp";
    const char* backupPath = "/coach.bak";
    LittleFS.remove(temporaryPath);
    File output = LittleFS.open(temporaryPath, "w");
    if (!output) {
        m_server.send(500, "application/json", "{\"error\":\"Could not write coach configuration\"}");
        return;
    }
    const size_t expectedBytes = measureJson(coachDocument);
    const size_t writtenBytes = serializeJson(coachDocument, output);
    output.flush();
    output.close();
    if (writtenBytes != expectedBytes) {
        LittleFS.remove(temporaryPath);
        m_server.send(500, "application/json", "{\"error\":\"Incomplete coach configuration write\"}");
        return;
    }

    LittleFS.remove(backupPath);
    if (!LittleFS.rename("/coach.json", backupPath)) {
        LittleFS.remove(temporaryPath);
        m_server.send(500, "application/json", "{\"error\":\"Could not stage coach configuration\"}");
        return;
    }
    if (!LittleFS.rename(temporaryPath, "/coach.json")) {
        LittleFS.rename(backupPath, "/coach.json");
        LittleFS.remove(temporaryPath);
        m_server.send(500, "application/json", "{\"error\":\"Could not replace coach configuration\"}");
        return;
    }
    LittleFS.remove(backupPath);
    m_server.send(200, "application/json", "{\"saved\":true}");
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
void SmartCoachWebServer::connectWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(m_ssid, m_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }
}

void SmartCoachWebServer::mountFilesystem()
{
    LittleFS.begin(true);   // format on failure
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
    if (LittleFS.exists("/index.html")) {
        File f = LittleFS.open("/index.html", "r");
        m_server.streamFile(f, "text/html");
        f.close();
    } else {
        m_server.send(200, "text/html",
            "<!DOCTYPE html><html><body>"
            "<h1>SmartCoach</h1>"
            "<p>Place index.html in the data/ folder.</p>"
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
#endif