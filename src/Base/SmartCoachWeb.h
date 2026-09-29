#ifdef FUTURE
#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

/**
 * SmartCoachWebServer – singleton foundation for the SmartCoach web UI
 * on the SK Pang ESP32-S3 CAN/LIN board.
 *
 * Principles applied:
 * 1. No public setters/getters.
 * 2. Explicit special-member functions (Rule of Five).
 * 3. Strict private / protected / public separation.
 * 4. Common server behaviour lives here; derived classes only add routes.
 */
class SmartCoachWebServer {
public:
    // -----------------------------------------------------------------
    // Singleton access – the only public way to obtain the object
    // -----------------------------------------------------------------
    static SmartCoachWebServer& instance(const char* ssid     = nullptr,
                                         const char* password = nullptr,
                                         uint16_t    port     = 80);

    // -----------------------------------------------------------------
    // Explicit language behaviours (Rule of Five)
    // -----------------------------------------------------------------
    SmartCoachWebServer(const SmartCoachWebServer&)            = delete;
    SmartCoachWebServer& operator=(const SmartCoachWebServer&) = delete;
    SmartCoachWebServer(SmartCoachWebServer&&)                 = delete;
    SmartCoachWebServer& operator=(SmartCoachWebServer&&)      = delete;

    virtual ~SmartCoachWebServer();

    // -----------------------------------------------------------------
    // Public interface
    // -----------------------------------------------------------------
    void begin();
    void handleClient();
    bool isRunning() const;

protected:
    // -----------------------------------------------------------------
    // Protected – for derived classes only
    // -----------------------------------------------------------------
    virtual void registerAdditionalRoutes();
    void serveStaticFile(const char* uri, const char* path, const char* contentType);
    WebServer& server();

private:
    // -----------------------------------------------------------------
    // Private construction – enforces the singleton
    // -----------------------------------------------------------------
    explicit SmartCoachWebServer(const char* ssid,
                                 const char* password,
                                 uint16_t    port);

    // -----------------------------------------------------------------
    // Private attributes
    // -----------------------------------------------------------------
    const char*   m_ssid;
    const char*   m_password;
    uint16_t      m_port;
    WebServer     m_server;
    bool          m_running;

    // -----------------------------------------------------------------
    // Private behaviours
    // -----------------------------------------------------------------
    void connectWiFi();
    void mountFilesystem();
    void installBaseRoutes();

    static void handleRootWrapper();
    static void handleStatusWrapper();
    static void handleNotFoundWrapper();

    void handleRoot();
    void handleStatus();
    void handleRenameDevice();
    void handleNotFound();

    // The single instance
    static SmartCoachWebServer* s_instance;
};
#endif