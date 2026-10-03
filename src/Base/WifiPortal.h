#pragma once

#include <Arduino.h>
#include <AsyncUDP.h>
#include <WebServer.h>
#include <WiFi.h>

class WifiPortal {
public:
    WifiPortal() = delete;
    WifiPortal(const WifiPortal&) = delete;
    WifiPortal& operator=(const WifiPortal&) = delete;
    WifiPortal(WifiPortal&&) = delete;
    WifiPortal& operator=(WifiPortal&&) = delete;
    ~WifiPortal();

    explicit WifiPortal(const char* apSsid);

    void begin();
    void poll();
    void stop();
    bool isActive() const;

private:
    static constexpr byte     DNS_PORT  = 53;
    static constexpr uint16_t HTTP_PORT = 80;
    static constexpr size_t   SSID_MAX  = 32;

    char        apSsid_[SSID_MAX + 1];
    bool        active_;
    AsyncUDP    dns_;
    WebServer*  server_;   // allocated in begin(), not in ctor

    bool active() const { return active_; }
    void active(bool value) { active_ = value; }

    WebServer* server() const { return server_; }
    void server(WebServer* value) { server_ = value; }

    void copySsid(const char* src);
    void handleDnsPacket(AsyncUDPPacket& packet);
    void registerRoutes();
    void handleRoot();
    void handleSave();
    void handleNotFound();
    String pageHtml() const;
};