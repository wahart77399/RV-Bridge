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
    // HomeSpan's HAP server owns port 80
    static constexpr uint16_t WEB_PORT = 8080;

    // -----------------------------------------------------------------
    // Singleton access – the only public way to obtain the object
    // -----------------------------------------------------------------
    static SmartCoachWebServer& instance(uint16_t port = WEB_PORT);

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

private:
    // -----------------------------------------------------------------
    // Private construction – enforces the singleton
    // -----------------------------------------------------------------
    explicit SmartCoachWebServer(uint16_t port);

    // -----------------------------------------------------------------
    // Private attributes
    // -----------------------------------------------------------------
    WebServer     m_server;
    bool          m_running;
    bool          m_restartRequired = false;

    // -----------------------------------------------------------------
    // Private behaviours
    // -----------------------------------------------------------------
    void mountFilesystem();
    void installBaseRoutes();

    static void handleRootWrapper();
    static void handleStatusWrapper();
    static void handleNotFoundWrapper();

    void handleRoot();
    void handleStatus();
    void handleRenameDevice();
    void handleReviewDevice();
    void handleCoachUpdate();
    void handleWifiReset();
    void handleReboot();
    void handleEmailSettings();
    void handleEmailPortal();
    void handleEmailStage();
    void handleEmailSend();
    void handleNotFound();

    // The single instance
    static SmartCoachWebServer* s_instance;
};