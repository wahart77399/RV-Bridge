#pragma once
#include "HTTPClient.h"
class WiFiClientSecure : public NetworkClient {
public:
    void useBuiltinCACertBundle() { fakeHttp.caBundleUsed = true; }
};