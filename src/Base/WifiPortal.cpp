#include "WifiPortal.h"
#include "WifiCredentials.h"

WifiPortal::WifiPortal(const char* apSsid)
    : active_(false)
    , server_(nullptr)
{
    copySsid(apSsid);
}

WifiPortal::~WifiPortal() {
    stop();
}

void WifiPortal::copySsid(const char* src) {
    const char* safe = "SmartCoach-Setup";
    if (src != nullptr && src[0] != '\0') {
        safe = src;
    }
    size_t i = 0;
    while (i < SSID_MAX && safe[i] != '\0') {
        apSsid_[i] = safe[i];
        i = i + 1;
    }
    apSsid_[i] = '\0';
}

bool WifiPortal::isActive() const {
    return active();
}

void WifiPortal::begin() {
    if (!active()) {
        WiFi.mode(WIFI_AP);
        IPAddress apIp(192, 168, 4, 1);
        IPAddress subnet(255, 255, 255, 0);
        if (!WiFi.softAPConfig(apIp, apIp, subnet)) {
            Serial.println("WifiPortal: failed to configure AP address");
            return;
        }
        if (!WiFi.softAP(apSsid_)) {
            Serial.println("WifiPortal: failed to start access point");
            return;
        }

        dns_.onPacket([this](AsyncUDPPacket& packet) { handleDnsPacket(packet); });
        bool dnsStarted = dns_.listen(DNS_PORT);

        if (server() == nullptr) {
            server(new WebServer(HTTP_PORT));
        }

        registerRoutes();
        server()->begin();
        active(true);
        Serial.printf("WifiPortal: SSID %s at http://%s/; DNS %s\n",
                      apSsid_,
                      WiFi.softAPIP().toString().c_str(),
                      dnsStarted ? "started" : "failed");
    }
}

void WifiPortal::stop() {
    if (active()) {
        if (server() != nullptr) {
            server()->stop();
            delete server();
            server(nullptr);
        }
        dns_.close();
        WiFi.softAPdisconnect(true);
        active(false);
    }
}

void WifiPortal::poll() {
    if (active() && server() != nullptr) {
        static uint32_t lastReport = 0;
        if (millis() - lastReport >= 10000) {
            lastReport = millis();
            Serial.printf("WifiPortal: clients=%u freeHeap=%u\n",
                          WiFi.softAPgetStationNum(), ESP.getFreeHeap());
        }
        server()->handleClient();
    }
}

void WifiPortal::handleDnsPacket(AsyncUDPPacket& packet) {
    const uint8_t* query = packet.data();
    size_t queryLength = packet.length();
    if (query == nullptr || queryLength < 17 || (query[2] & 0x80) != 0) {
        return;
    }

    uint16_t questionCount = (static_cast<uint16_t>(query[4]) << 8) | query[5];
    if (questionCount != 1) {
        return;
    }

    size_t offset = 12;
    char queryName[254];
    size_t queryNameLength = 0;
    while (offset < queryLength) {
        uint8_t labelLength = query[offset++];
        if (labelLength == 0) {
            break;
        }
        if ((labelLength & 0xC0) != 0 || labelLength > 63 ||
            offset + labelLength > queryLength) {
            return;
        }
        if (queryNameLength > 0) {
            if (queryNameLength + 1 >= sizeof(queryName)) {
                return;
            }
            queryName[queryNameLength++] = '.';
        }
        if (queryNameLength + labelLength >= sizeof(queryName)) {
            return;
        }
        memcpy(queryName + queryNameLength, query + offset, labelLength);
        queryNameLength += labelLength;
        offset += labelLength;
    }
    queryName[queryNameLength] = '\0';
    if (offset + 4 > queryLength) {
        return;
    }

    size_t questionEnd = offset + 4;
    uint16_t questionType = (static_cast<uint16_t>(query[offset]) << 8) | query[offset + 1];
    uint16_t questionClass = (static_cast<uint16_t>(query[offset + 2]) << 8) | query[offset + 3];
    bool answerWithAddress = (questionClass == 1 && questionType == 1);
    size_t responseLength = questionEnd + (answerWithAddress ? 16 : 0);
    uint8_t response[512];
    if (responseLength > sizeof(response)) {
        return;
    }

    memcpy(response, query, 12);
    memcpy(response + 12, query + 12, questionEnd - 12);
    response[2] = (query[2] & 0x79) | 0x84;
    response[3] = 0;
    response[4] = 0;
    response[5] = 1;
    response[6] = 0;
    response[7] = answerWithAddress ? 1 : 0;
    response[8] = 0;
    response[9] = 0;
    response[10] = 0;
    response[11] = 0;

    if (answerWithAddress) {
        size_t answerOffset = questionEnd;
        IPAddress apAddress = WiFi.softAPIP();
        response[answerOffset++] = 0xC0;
        response[answerOffset++] = 0x0C;
        response[answerOffset++] = 0;
        response[answerOffset++] = 1;
        response[answerOffset++] = 0;
        response[answerOffset++] = 1;
        response[answerOffset++] = 0;
        response[answerOffset++] = 0;
        response[answerOffset++] = 0;
        response[answerOffset++] = 30;
        response[answerOffset++] = 0;
        response[answerOffset++] = 4;
        response[answerOffset++] = apAddress[0];
        response[answerOffset++] = apAddress[1];
        response[answerOffset++] = apAddress[2];
        response[answerOffset] = apAddress[3];
    }

    size_t bytesSent = packet.write(response, responseLength);
    Serial.printf("WifiPortal: DNS %s from %s type=%u reply=%u/%u\n",
                  queryName,
                  packet.remoteIP().toString().c_str(),
                  questionType,
                  static_cast<unsigned int>(bytesSent),
                  static_cast<unsigned int>(responseLength));
}

void WifiPortal::registerRoutes() {
    WebServer* s = server();
    if (s != nullptr) {
        s->on("/", HTTP_GET, [this]() { handleRoot(); });
        const char* captiveCheckPaths[] = {
            "/canonical.html",
            "/generate_204",
            "/gen_204",
            "/hotspot-detect.html",
            "/library/test/success.html",
            "/success.txt",
            "/connecttest.txt",
            "/ncsi.txt",
            "/cpcheck.txt"
        };
        for (const char* path : captiveCheckPaths) {
            s->on(path, HTTP_ANY, [this]() { handleRoot(); });
        }
        s->on("/save", HTTP_POST, [this]() { handleSave(); });
        s->onNotFound([this]() { handleNotFound(); });
    }
}

void WifiPortal::handleRoot() {
    WebServer* s = server();
    if (s != nullptr) {
        String clientIp = s->client().remoteIP().toString();
        String requestPath = s->uri();
        unsigned int requestMethod = static_cast<unsigned int>(s->method());
        String body = pageHtml();
        s->sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
        s->sendHeader("Pragma", "no-cache");
        s->send(200, "text/html; charset=utf-8", body);
        Serial.printf("WifiPortal: HTTP method=%u path=%s response to %s bytes=%u\n",
                      requestMethod,
                      requestPath.c_str(),
                      clientIp.c_str(),
                      static_cast<unsigned int>(body.length()));
    }
}

void WifiPortal::handleNotFound() {
    WebServer* s = server();
    if (s != nullptr) {
        String clientIp = s->client().remoteIP().toString();
        unsigned int requestMethod = static_cast<unsigned int>(s->method());
        Serial.printf("WifiPortal: HTTP method=%u unknown path %s from %s\n",
                      requestMethod, s->uri().c_str(), clientIp.c_str());
        String location = String("http://") + WiFi.softAPIP().toString() + "/";
        s->sendHeader("Location", location, true);
        s->send(302, "text/plain", "");
    }
}

void WifiPortal::handleSave() {
    WebServer* s = server();
    String ssid;
    String pass;
    bool   haveSsid = false;
    bool   saved    = false;
    int    code     = 400;
    String body;

    if (s != nullptr) {
        if (s->hasArg("ssid")) {
            ssid = s->arg("ssid");
            ssid.trim();
            haveSsid = (ssid.length() > 0);
        }
        if (s->hasArg("pass")) {
            pass = s->arg("pass");
            pass.trim();
        } else {
            pass = "";
        }
        if (haveSsid) {
            saved = WifiCredentials::save(ssid, pass);
        }
        if (saved) {
            code = 200;
            body = "<html><body><h3>Saved. Rebooting...</h3></body></html>";
        } else {
            code = 400;
            body = "<html><body><h3>Save failed</h3><a href='/'>Back</a></body></html>";
        }
        s->send(code, "text/html", body);
        if (saved) {
            delay(500);
            ESP.restart();
        }
    }
}

String WifiPortal::pageHtml() const {
    String html;
    html = "<!doctype html><html lang='en'><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>RV-Bridge Wi-Fi Setup</title>";
    html += "<style>body{margin:0;padding:24px;background:#f3f6f8;color:#17212b;";
    html += "font:16px -apple-system,BlinkMacSystemFont,sans-serif}";
    html += "main{max-width:420px;margin:8vh auto;padding:24px;background:#fff;";
    html += "border:1px solid #d8e0e5;border-radius:8px}h1{font-size:22px;margin:0 0 24px}";
    html += "label{display:block;margin:16px 0 6px}input{box-sizing:border-box;width:100%;";
    html += "padding:12px;border:1px solid #8797a3;border-radius:4px;font:inherit}";
    html += "button{margin-top:24px;padding:12px 18px;border:0;border-radius:4px;";
    html += "background:#126b52;color:white;font:inherit}</style></head><body><main>";
    html += "<h1>RV-Bridge Wi-Fi Setup</h1><form method='POST' action='/save'>";
    html += "<label for='ssid'>Wi-Fi network name</label>";
    html += "<input id='ssid' name='ssid' maxlength='32' autocomplete='off' required>";
    html += "<label for='pass'>Wi-Fi password</label>";
    html += "<input id='pass' name='pass' type='password' maxlength='64' autocomplete='off'>";
    html += "<button type='submit'>Save and connect</button></form></main></body></html>";
    return html;
}