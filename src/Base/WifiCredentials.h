#pragma once

#include <Arduino.h>
#include <Preferences.h>

class WifiCredentials {
public:
    WifiCredentials() = delete;
    WifiCredentials(const WifiCredentials&) = delete;
    WifiCredentials& operator=(const WifiCredentials&) = delete;
    WifiCredentials(WifiCredentials&&) = delete;
    WifiCredentials& operator=(WifiCredentials&&) = delete;
    ~WifiCredentials() = default;

    // domain behaviour
    static bool load(String& ssid, String& password);
    static bool save(const String& ssid, const String& password);
    static bool clear();
    static String preparePairingCode();

private:
    static constexpr const char* kNamespace = "wifi";
    static constexpr const char* kKeySsid   = "ssid";
    static constexpr const char* kKeyPass   = "pass";
};
