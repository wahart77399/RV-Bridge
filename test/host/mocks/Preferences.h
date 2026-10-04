#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>

struct FakePreferencesStore {
    std::map<std::string, uint32_t> values;
    std::map<std::string, std::string> stringValues;
    std::set<std::string> failedWrites;
    bool available = true;
};
inline FakePreferencesStore preferencesStore;

class Preferences {
public:
    bool begin(const char* name, bool) {
        opened_ = preferencesStore.available;
        namespace_ = name;
        return opened_;
    }
    uint32_t getUInt(const char* key, uint32_t fallback) const {
        uint32_t result = fallback;
        auto found = preferencesStore.values.find(namespace_ + "/" + key);
        if (opened_ && found != preferencesStore.values.end()) result = found->second;
        return result;
    }
    size_t putUInt(const char* key, uint32_t value) {
        size_t written = 0;
        std::string fullKey = namespace_ + "/" + key;
        if (opened_ && preferencesStore.failedWrites.count(fullKey) == 0) {
            preferencesStore.values[fullKey] = value;
            written = sizeof(value);
        }
        return written;
    }
    String getString(const char* key, const char* fallback = "") const {
        String result(fallback);
        auto found = preferencesStore.stringValues.find(namespace_ + "/" + key);
        if (opened_ && found != preferencesStore.stringValues.end()) result = String(found->second);
        return result;
    }
    size_t putString(const char* key, const String& value) {
        size_t written = 0;
        std::string fullKey = namespace_ + "/" + key;
        if (opened_ && preferencesStore.failedWrites.count(fullKey) == 0) {
            preferencesStore.stringValues[fullKey] = value.c_str();
            written = value.length();
        }
        return written;
    }
    void end() { opened_ = false; }
private:
    bool opened_ = false;
    std::string namespace_;
};