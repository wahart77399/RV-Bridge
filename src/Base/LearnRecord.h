// LearnRecord.h — listen-only inventory, not a full packet log
#pragma once
#include <Arduino.h>
#include "DGN.h"

// Values are persisted in /learn.bin — append only, never renumber.
enum class LearnedKind : uint8_t {
    Unknown = 0,
    Light,
    Awning,
    Shade,
    Tank,
    Thermostat,
    FloorHeat,
    WaterPump,
    DoorLock,
    Generator,
    Inverter,
    Charger,
    Ats,
    Battery,
    Count
};

// Plain persisted record: must stay trivially copyable for the binary file.
struct LearnRecord {
    uint32_t    dgn;            // first DGN seen for this device
    uint32_t    firstSeenSec;   // learn-clock seconds (survives reboots)
    uint32_t    lastSeenSec;
    uint32_t    hitCount;
    uint8_t     instance;
    uint8_t     sourceAddress;
    LearnedKind kind;
    uint8_t     flags;
    uint8_t     sample[8];      // last payload snapshot

    static constexpr uint8_t FLAG_HAS_SAMPLE   = 0x01;
    static constexpr uint8_t FLAG_PARTIAL_DIM  = 0x02;  // light reported a level between off and full
};

class LearnTable {
    friend class LearnMode;
public:
    static constexpr size_t MAX_RECORDS     = 192;
    static constexpr size_t RESERVED_KNOWN  = 64;   // slots unknown DGNs may not consume

    LearnTable();
    LearnTable(const LearnTable&) = delete;
    LearnTable& operator=(const LearnTable&) = delete;
    LearnTable(LearnTable&&) = delete;
    LearnTable& operator=(LearnTable&&) = delete;
    ~LearnTable() = default;

    void   observe(RVC_DGN dgn, uint8_t instance, uint8_t sourceAddress, const uint8_t* data, uint32_t nowSec);
    void   reset();
    bool         load(const void* src, size_t rowCount);

    static LearnedKind classify(RVC_DGN dgn);
    static const char* relatedFamily(RVC_DGN dgn);
    static const char* dgnName(RVC_DGN dgn);
    static const char* typeName(const LearnRecord& r);   // devices.json "type"
    static const char* label(LearnedKind kind);           // default device name

private:
    LearnRecord rows_[MAX_RECORDS];
    size_t      count_;

    size_t count() const { return count_; }
    const LearnRecord& row(size_t index) const { return rows_[index]; }
    size_t byteSize() const { return count_ * sizeof(LearnRecord); }
    const void* bytes() const { return rows_; }

    LearnRecord* findOrAlloc(RVC_DGN dgn, LearnedKind kind, uint8_t instance, uint8_t sourceAddress);
};