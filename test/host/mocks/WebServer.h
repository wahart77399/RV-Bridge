#pragma once
#include "LittleFS.h"
#include <functional>

constexpr int HTTP_GET = 1;
constexpr int HTTP_POST = 2;

class WebServer {
public:
    class Client {
    public:
        explicit Client(WebServer& server) : server_(server) {}
        size_t write(const uint8_t* data, size_t length) {
            size_t written = std::min(length, server_.clientWriteLimit);
            server_.responseBody += String(std::string(reinterpret_cast<const char*>(data), written));
            return written;
        }
    private:
        WebServer& server_;
    };

    inline static WebServer* active = nullptr;
    int responseCode = 0;
    String responseBody;
    size_t contentLength = 0;
    size_t clientWriteLimit = static_cast<size_t>(-1);
    std::map<std::string, std::string> responseHeaders;
    bool chunkedResponseEnded = false;
    explicit WebServer(uint16_t) { active = this; }
    WebServer(const WebServer&) = delete;
    WebServer& operator=(const WebServer&) = delete;
    WebServer(WebServer&&) = delete;
    WebServer& operator=(WebServer&&) = delete;
    ~WebServer() = default;
    void begin() {}
    void stop() {}
    void handleClient() {}
    void on(const char* route, int method, std::function<void()> callback) {
        routes_[std::to_string(method) + route] = std::move(callback);
    }
    void onNotFound(std::function<void()> callback) { missing_ = std::move(callback); }
    String arg(const char*) const { return requestBody_; }
    void send(int code, const char*, const String& body) { responseCode = code; responseBody = body; }
    void sendHeader(const char* name, const char* value) { responseHeaders[name] = value; }
    void setContentLength(size_t length) { contentLength = length; }
    Client client() { return Client(*this); }
    void chunkResponseBegin(const char*) { responseCode = 200; responseBody = ""; chunkedResponseEnded = false; }
    void chunkWrite(const char* content, size_t length) { responseBody += String(std::string(content, length)); }
    void chunkResponseEnd() { chunkedResponseEnded = true; }
    void streamFile(File& file, const char* contentType) {
        std::string body;
        int byte = file.read();
        while (byte >= 0) { body += static_cast<char>(byte); byte = file.read(); }
        send(200, contentType, String(body));
    }
    void request(const char* route, int method, const String& body = "") {
        requestBody_ = body;
        responseCode = 0;
        responseBody = "";
        contentLength = 0;
        responseHeaders.clear();
        chunkedResponseEnded = false;
        auto callback = routes_.find(std::to_string(method) + route);
        if (callback != routes_.end()) callback->second();
        else if (missing_) missing_();
    }
private:
    String requestBody_;
    std::map<std::string, std::function<void()>> routes_;
    std::function<void()> missing_;
};