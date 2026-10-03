#include "WifiCredentials.h"

bool WifiCredentials::load(String& ssid, String& password) {
    bool success = false;
    Preferences prefs;

    ssid = "";
    password = "";

    if (prefs.begin(kNamespace, true)) {
        ssid     = prefs.getString(kKeySsid, "");
        password = prefs.getString(kKeyPass, "");
        prefs.end();
        success = (ssid.length() > 0);
    }
    return success;
}

bool WifiCredentials::save(const String& ssid, const String& password) {
    bool success = false;
    Preferences prefs;

    if (ssid.length() > 0 && prefs.begin(kNamespace, false)) {
        prefs.putString(kKeySsid, ssid);
        prefs.putString(kKeyPass, password);
        prefs.end();
        success = true;
    }
    return success;
}

bool WifiCredentials::clear() {
    bool success = false;
    Preferences prefs;

    if (prefs.begin(kNamespace, false)) {
        prefs.clear();
        prefs.end();
        success = true;
    }
    return success;
}
