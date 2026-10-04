#pragma once
#include "Arduino.h"
constexpr int WL_CONNECTED = 3;
struct FakeAddress {
    String toString() const { return "127.0.0.1"; }
};
struct FakeWifi {
    int statusCode = WL_CONNECTED;
    FakeAddress localIP() const { return {}; }
    int RSSI() const { return -35; }
    int status() const { return statusCode; }
};
inline FakeWifi WiFi;