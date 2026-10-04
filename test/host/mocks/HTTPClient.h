#pragma once
#include "Arduino.h"
#include "LittleFS.h"
#include <map>
#include <string>
#include <vector>

class NetworkClient {
public:
    NetworkClient() = default;
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;
    NetworkClient(NetworkClient&&) = delete;
    NetworkClient& operator=(NetworkClient&&) = delete;
    ~NetworkClient() = default;
};

enum followRedirects_t { HTTPC_DISABLE_FOLLOW_REDIRECTS, HTTPC_STRICT_FOLLOW_REDIRECTS, HTTPC_FORCE_FOLLOW_REDIRECTS };

struct FakeHttpResponse {
    int code = -1;
    String body;
    String location;
};

struct FakeHttpRequest {
    String method;
    String url;
    String body;
};

struct FakeHttpEnvironment {
    std::vector<FakeHttpResponse> responses;
    std::vector<FakeHttpRequest> requests;
    size_t nextResponse = 0;
    size_t activeResponse = static_cast<size_t>(-1);
    bool beginAllowed = true;
    bool caBundleUsed = false;
};

inline FakeHttpEnvironment fakeHttp;

class HTTPClient {
public:
    bool begin(NetworkClient&, const String& url) {
        url_ = url;
        return fakeHttp.beginAllowed;
    }
    void setTimeout(uint16_t) {}
    void setConnectTimeout(int32_t) {}
    void setFollowRedirects(followRedirects_t) {}
    void collectHeaders(const char*[], size_t) {}
    void addHeader(const String&, const String&) {}
    int POST(const String& body) {
        fakeHttp.requests.push_back({String("POST"), url_, body});
        activeResponse_ = nextResponse();
        return response().code;
    }
    int sendRequest(const char* method, File* stream, size_t) {
        std::string body;
        char buffer[256];
        size_t count = stream->readBytes(buffer, sizeof(buffer));
        while (count > 0) {
            body.append(buffer, count);
            count = stream->readBytes(buffer, sizeof(buffer));
        }
        fakeHttp.requests.push_back({String(method), url_, String(body)});
        activeResponse_ = nextResponse();
        return response().code;
    }
    int GET() {
        fakeHttp.requests.push_back({String("GET"), url_, String()});
        activeResponse_ = nextResponse();
        return response().code;
    }
    String header(const char* name) const {
        String result;
        if (std::string(name) == "Location") result = response().location;
        return result;
    }
    String getString() const { return response().body; }
    void end() {}
private:
    String url_;
    size_t activeResponse_ = static_cast<size_t>(-1);
    size_t nextResponse() {
        size_t result = static_cast<size_t>(-1);
        if (fakeHttp.nextResponse < fakeHttp.responses.size()) result = fakeHttp.nextResponse++;
        return result;
    }
    const FakeHttpResponse& response() const {
        static const FakeHttpResponse missing;
        const FakeHttpResponse* result = &missing;
        if (activeResponse_ < fakeHttp.responses.size()) result = &fakeHttp.responses[activeResponse_];
        return *result;
    }
};