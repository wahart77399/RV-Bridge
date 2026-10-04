#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 0
#define ARDUINOJSON_ENABLE_ARDUINO_PRINT 0
#define ARDUINOJSON_ENABLE_PROGMEM 0

class String {
public:
    String() = default;
    String(const char* value) : value_(value != nullptr ? value : "") {}
    String(const std::string& value) : value_(value) {}
    template <typename Number, typename = std::enable_if_t<std::is_integral<Number>::value>>
    String(Number value) : value_(std::to_string(value)) {}
    String(const String&) = default;
    String& operator=(const String&) = default;
    String(String&&) = default;
    String& operator=(String&&) = default;
    ~String() = default;
    const char* c_str() const { return value_.c_str(); }
    size_t length() const { return value_.length(); }
    bool isEmpty() const { return value_.empty(); }
    bool reserve(size_t capacity) { value_.reserve(capacity); return true; }
    bool concat(const char* value) { value_ += value; return true; }
    char operator[](size_t index) const { return value_[index]; }
    String& operator+=(char value) { value_ += value; return *this; }
    template <typename Number, typename = std::enable_if_t<std::is_integral<Number>::value>>
    String& operator+=(Number value) { value_ += std::to_string(value); return *this; }
    String& operator+=(const String& value) { value_ += value.value_; return *this; }
    String operator+(const String& value) const { return String(value_ + value.value_); }
    bool operator==(const char* value) const { return value_ == value; }
    bool operator!=(const char* value) const { return value_ != value; }
    bool operator==(const String& value) const { return value_ == value.value_; }
    bool operator<(const String& value) const { return value_ < value.value_; }
    bool startsWith(const char* prefix) const { return value_.rfind(prefix, 0) == 0; }
    bool endsWith(const char* suffix) const {
        std::string ending(suffix);
        return value_.size() >= ending.size() && value_.compare(value_.size() - ending.size(), ending.size(), ending) == 0;
    }
    void remove(size_t index) { value_.erase(index); }
    String substring(size_t index) const { return String(value_.substr(std::min(index, value_.size()))); }
    long toInt() const { return std::strtol(value_.c_str(), nullptr, 10); }
    void toLowerCase() {
        for (char& value : value_) value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
    }
    void trim() {
        const auto first = value_.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) value_.clear();
        else value_ = value_.substr(first, value_.find_last_not_of(" \t\r\n") - first + 1);
    }
private:
    std::string value_;
};

inline uint32_t fakeMillis = 1000;
inline uint32_t millis() { return fakeMillis; }
inline void delay(uint32_t duration) { fakeMillis += duration; }

struct RestartRequested {};
struct FakeEsp {
    unsigned restarts = 0;
    void restart() { ++restarts; throw RestartRequested{}; }
    uint32_t getFreeHeap() const { return 120000; }
};
inline FakeEsp ESP;

struct FakeSerial {
    std::string log;
    void println(const char* message) { log += message; log += '\n'; }
    void flush() {}
    template <typename... Arguments>
    void printf(const char* format, Arguments... arguments) {
        char buffer[512];
        std::snprintf(buffer, sizeof(buffer), format, arguments...);
        log += buffer;
    }
};
inline FakeSerial Serial;