#include "WifiCredentials.h"
#include <esp_random.h>
#include <nvs.h>

String WifiCredentials::preparePairingCode() {
    Preferences settings;
    if (!settings.begin("smartcoach", false)) return "";
    String code = settings.getString("setupCode", "");
    if (code.length() == 8) {
        settings.end();
        return code;
    }
    nvs_handle_t handle;
    size_t length = 0;
    esp_err_t result = nvs_open("SRP", NVS_READONLY, &handle);
    if (result == ESP_OK) {
        result = nvs_get_blob(handle, "VERIFYDATA", nullptr, &length);
        nvs_close(handle);
    }
    if (result != ESP_ERR_NVS_NOT_FOUND) {
        settings.end();
        return "";
    }
    char digits[9];
    bool valid = false;
    while (!valid) {
        for (size_t index = 0; index < 8; ++index) {
            digits[index] = '0' + esp_random() % 10;
        }
        digits[8] = '\0';
        code = digits;
        valid = code != "12345678" && code != "87654321";
        bool repeated = true;
        for (size_t index = 1; index < 8; ++index) {
            repeated = repeated && digits[index] == digits[0];
        }
        valid = valid && !repeated;
    }
    if (settings.putString("setupCode", code) == 0) code = "";
    settings.end();
    return code;
}

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
