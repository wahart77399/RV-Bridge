#include "WifiPortal.h"
#include "WifiCredentials.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

bool WifiPortal::saveCoachDetails() {
    WebServer* request = server();
    if (request == nullptr || !LittleFS.begin(false)) return false;
    String yearText = request->arg("year");
    String make = request->arg("make");
    String model = request->arg("model");
    String floorplan = request->arg("floorplan");
    make.trim();
    model.trim();
    floorplan.trim();
    int year = yearText.toInt();
    if (yearText.length() != 4 || year < 1900 || year > 2200 ||
        make.isEmpty() || model.isEmpty() || floorplan.isEmpty() ||
        make.length() > 64 || model.length() > 64 || floorplan.length() > 64) return false;
    String emails = request->arg("emails");
    if (emails.length() > 1024) return false;
    JsonDocument document;
    File existing = LittleFS.open("/coach.json", "r");
    if (existing) {
        DeserializationError error = deserializeJson(document, existing);
        existing.close();
        if (error || !document.is<JsonObject>()) return false;
    }
    document["year"] = year;
    document["make"] = make;
    document["model"] = model;
    document["floorplan"] = floorplan;
    if (document["coachId"].isNull()) document["coachId"] = "";
    if (document["tanks"].isNull()) document["tanks"].to<JsonArray>();
    if (document["batteries"].isNull()) document["batteries"].to<JsonArray>();
    if (document["coverTimes"].isNull()) document["coverTimes"].to<JsonObject>();
    JsonArray recipients = document["ownerEmails"].to<JsonArray>();
    size_t start = 0;
    while (start < emails.length()) {
        int comma = emails.indexOf(',', start);
        String email = emails.substring(start, comma < 0 ? emails.length() : comma);
        email.trim();
        int at = email.indexOf('@');
        if (email.length() > 254 || at <= 0 || email.lastIndexOf('@') != at ||
            email.indexOf('.', at + 2) < 0 || email.endsWith(".") ||
            email.indexOf(' ') >= 0 || email.indexOf('\r') >= 0 || email.indexOf('\n') >= 0 ||
            recipients.size() >= 5) return false;
        recipients.add(email);
        if (comma < 0) break;
        start = comma + 1;
    }
    if (recipients.size() == 0) return false;
    document["shareWithSupport"] = request->hasArg("support");
    document["diagnosticEmails"] = request->hasArg("diagnostics");
    File output = LittleFS.open("/coach.setup.tmp", "w");
    if (!output) return false;
    size_t expected = measureJson(document);
    size_t written = serializeJson(document, output);
    output.flush();
    output.close();
    if (written != expected) return false;
    bool hadExisting = LittleFS.exists("/coach.json");
    if (hadExisting && !LittleFS.rename("/coach.json", "/coach.setup.bak")) return false;
    if (!LittleFS.rename("/coach.setup.tmp", "/coach.json")) {
        if (hadExisting) LittleFS.rename("/coach.setup.bak", "/coach.json");
        return false;
    }
    LittleFS.remove("/coach.setup.bak");
    return true;
}

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
        } else {
            pass = "";
        }
        if (haveSsid && ssid.length() <= 32 && pass.length() <= 64 && saveCoachDetails()) {
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
    JsonDocument coach;
    if (LittleFS.begin(false)) {
        File input = LittleFS.open("/coach.json", "r");
        if (input) {
            deserializeJson(coach, input);
            input.close();
        }
    }
    auto escape = [](String value) {
        value.replace("&", "&amp;");
        value.replace("<", "&lt;");
        value.replace(">", "&gt;");
        value.replace("'", "&#39;");
        value.replace("\"", "&quot;");
        return value;
    };
    String emailList;
    for (JsonVariant recipient : coach["ownerEmails"].as<JsonArray>()) {
        if (emailList.length() > 0) emailList += ", ";
        emailList += recipient.as<String>();
    }
    String setupCode = WifiCredentials::preparePairingCode();
    String html;
    html = "<!doctype html><html lang='en'><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>SmartCoach Setup</title>";
    html += "<style>body{margin:0;padding:24px;background:#f3f6f8;color:#17212b;";
    html += "font:16px -apple-system,BlinkMacSystemFont,sans-serif}";
    html += "main{max-width:420px;margin:8vh auto;padding:24px;background:#fff;";
    html += "border:1px solid #d8e0e5;border-radius:8px}h1{font-size:22px;margin:0 0 24px}";
    html += "label{display:block;margin:16px 0 6px}input{box-sizing:border-box;width:100%;";
    html += "padding:12px;border:1px solid #8797a3;border-radius:4px;font:inherit}";
    html += "input[type=checkbox]{width:auto;margin-right:8px}h2{font-size:18px;margin-top:28px}";
    html += "button{margin-top:24px;padding:12px 18px;border:0;border-radius:4px;";
    html += "background:#126b52;color:white;font:inherit}</style></head><body><main>";
    html += "<h1>SmartCoach Setup</h1><form method='POST' action='/save'>";
    html += "<label for='ssid'>Wi-Fi network name</label>";
    html += "<input id='ssid' name='ssid' maxlength='32' autocomplete='off' required>";
    html += "<label for='pass'>Wi-Fi password</label>";
    html += "<input id='pass' name='pass' type='password' maxlength='64' autocomplete='off'>";
    html += "<h2>Coach</h2><label for='year'>Year</label>";
    html += "<input id='year' name='year' type='number' min='1900' max='2200' required value='";
    html += escape(coach["year"].as<String>()) + "'>";
    const char* fields[] = {"make", "model", "floorplan"};
    const char* labels[] = {"Make", "Model", "Floorplan"};
    for (size_t index = 0; index < 3; ++index) {
        html += String("<label for='") + fields[index] + "'>" + labels[index] + "</label>";
        html += String("<input id='") + fields[index] + "' name='" + fields[index];
        html += "' maxlength='64' required value='" + escape(coach[fields[index]] | "") + "'>";
    }
    html += "<h2>Reports</h2><label for='emails'>Owner email addresses</label>";
    html += "<input id='emails' name='emails' type='email' multiple maxlength='1024' required value='";
    html += escape(emailList) + "'>";
    html += "<label><input type='checkbox' name='support'";
    if (coach["shareWithSupport"] | false) html += " checked";
    html += ">Share the discovery report with SmartCoach support</label>";
    html += "<label><input type='checkbox' name='diagnostics'";
    if (coach["diagnosticEmails"] | false) html += " checked";
    html += ">Email status and diagnostic reports</label>";
    html += "<p>Email preferences are saved locally. Email delivery is not connected in this firmware.</p>";
    html += "<h2>Apple Home</h2>";
    if (setupCode.length() == 8) {
        html += "<p>Pairing code: <strong>" + setupCode.substring(0, 3) + "-";
        html += setupCode.substring(3, 5) + "-" + setupCode.substring(5) + "</strong></p>";
    } else {
        html += "<p>Your existing HomeKit pairing code and pairing are unchanged.</p>";
    }
    html += "<p>Connect your iPhone to the coach Wi-Fi. In Home, select Add Accessory, More Options, then SmartCoach. Enter the pairing code and assign rooms.</p>";
    html += "<p>For remote access and automations, keep a supported Apple TV or HomePod/HomePod mini powered and connected to the coach network, configured as a home hub in the same Apple Home.</p>";
    html += "<p>A new unit without a device configuration learns for 72 hours of powered listening time. Devices absent from its coach profile remain pending approval.</p>";
    html += "<button type='submit'>Save and connect</button></form></main></body></html>";
    return html;
}