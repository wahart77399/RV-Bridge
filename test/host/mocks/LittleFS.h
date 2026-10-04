#pragma once
#include "Arduino.h"
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <set>

struct FakeFileData {
    std::string bytes;
    size_t writeLimit = std::numeric_limits<size_t>::max();
};

class File {
public:
    File() = default;
    explicit File(std::shared_ptr<FakeFileData> data) : data_(std::move(data)) {}
    File(const File&) = default;
    File& operator=(const File&) = default;
    File(File&&) = default;
    File& operator=(File&&) = default;
    ~File() = default;
    explicit operator bool() const { return data_ != nullptr; }
    size_t size() const { return data_ != nullptr ? data_->bytes.size() : 0; }
    size_t write(const uint8_t* bytes, size_t count) {
        size_t written = 0;
        if (data_ != nullptr && position_ < data_->writeLimit) {
            written = std::min(count, data_->writeLimit - position_);
            data_->bytes.append(reinterpret_cast<const char*>(bytes), written);
            position_ += written;
        }
        return written;
    }
    size_t write(uint8_t byte) { return write(&byte, 1); }
    size_t read(uint8_t* bytes, size_t count) {
        size_t received = 0;
        if (data_ != nullptr && position_ < data_->bytes.size()) {
            received = std::min(count, data_->bytes.size() - position_);
            std::memcpy(bytes, data_->bytes.data() + position_, received);
            position_ += received;
        }
        return received;
    }
    int read() {
        uint8_t byte = 0;
        int result = -1;
        if (read(&byte, 1) == 1) result = byte;
        return result;
    }
    size_t readBytes(char* bytes, size_t count) { return read(reinterpret_cast<uint8_t*>(bytes), count); }
    void flush() {}
    void close() { data_.reset(); }
private:
    std::shared_ptr<FakeFileData> data_;
    size_t position_ = 0;
};

struct FakeFilesystem {
    std::map<std::string, std::shared_ptr<FakeFileData>> files;
    std::set<std::string> failOpen;
    std::set<std::string> failRename;
    std::map<std::string, size_t> writeLimits;
    std::map<std::string, unsigned> writeAttempts;
    bool mounted = true;
    bool begin(bool = false) { return mounted; }
    bool exists(const String& path) const { return files.count(path.c_str()) != 0; }
    bool remove(const String& path) { return files.erase(path.c_str()) != 0; }
    File open(const String& path, const char* mode) {
        File result;
        std::string key(path.c_str());
        if (mounted && failOpen.count(key) == 0) {
            if (mode[0] == 'w') {
                ++writeAttempts[key];
                auto data = std::make_shared<FakeFileData>();
                if (writeLimits.count(key) != 0) data->writeLimit = writeLimits.at(key);
                files[key] = data;
                result = File(data);
            } else if (files.count(key) != 0) {
                result = File(files.at(key));
            }
        }
        return result;
    }
    bool rename(const String& source, const String& destination) {
        bool result = false;
        std::string from(source.c_str());
        if (mounted && failRename.count(from) == 0 && files.count(from) != 0) {
            files[destination.c_str()] = files.at(from);
            files.erase(from);
            result = true;
        }
        return result;
    }
    void put(const char* path, const std::string& bytes) {
        auto data = std::make_shared<FakeFileData>();
        data->bytes = bytes;
        files[path] = data;
    }
    std::string content(const char* path) const {
        std::string result;
        if (files.count(path) != 0) result = files.at(path)->bytes;
        return result;
    }
};
inline FakeFilesystem LittleFS;