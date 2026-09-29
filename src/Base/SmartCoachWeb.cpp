#ifdef FUTURE
#include "SmartCoachWeb.h"
#include <ArduinoJson.h>
#include <cstring>

SmartCoachWebServer* SmartCoachWebServer::s_instance = nullptr;

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