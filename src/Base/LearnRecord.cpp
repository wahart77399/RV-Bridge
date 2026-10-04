#include "LearnRecord.h"
#include <string.h>

namespace {
    // DC_DIMMER_STATUS_3 byte 2 is brightness in 0.5% steps (200 = 100%)
    constexpr uint8_t DIMMER_LEVEL_INDEX   = 2;
    constexpr uint8_t DIMMER_PARTIAL_LOW   = 10;
    constexpr uint8_t DIMMER_PARTIAL_HIGH  = 190;
    constexpr uint8_t UNKNOWN_INSTANCE     = 0xFF;
}

LearnTable::LearnTable() : count_(0) {
    reset();
}

void LearnTable::reset() {
    memset(rows_, 0, sizeof(rows_));
    count_ = 0;
}

LearnedKind LearnTable::classify(RVC_DGN dgn) {
    LearnedKind kind = LearnedKind::Unknown;
    switch (dgn) {
        case DC_DIMMER_STATUS_3:          kind = LearnedKind::Light;      break;
        case AWNING_STATUS:               kind = LearnedKind::Awning;     break;
        case WINDOW_SHADE_CONTROL_STATUS: kind = LearnedKind::Shade;      break;
        case TANK_STATUS:                 kind = LearnedKind::Tank;       break;
        case THERMOSTAT_STATUS_1:         kind = LearnedKind::Thermostat; break;
        case FLOOR_HEAT_STATUS:           kind = LearnedKind::FloorHeat;  break;
        case WATER_PUMP_STATUS:           kind = LearnedKind::WaterPump;  break;
        case LOCK_STATUS:                 kind = LearnedKind::DoorLock;   break;
        case GENERATOR_AC_STATUS_1:       kind = LearnedKind::Generator;  break;
        case INVERTER_AC_STATUS_1:
        case INVERTER_STATUS:             kind = LearnedKind::Inverter;   break;
        case CHARGER_STATUS:              kind = LearnedKind::Charger;    break;
        case ATS_AC_STATUS_1:             kind = LearnedKind::Ats;        break;
        case DC_SOURCE_STATUS_1:          kind = LearnedKind::Battery;    break;
        default:                                                          break;
    }
    return kind;
}

namespace {
    struct RelatedDgn {
        RVC_DGN dgn;
        const char* name;
        const char* family;
    };

    constexpr RelatedDgn relatedDgns[] = {
        {ATS_STATUS, "ATS_STATUS", "ATS"},
        {ATS_COMMAND, "ATS_COMMAND", "ATS"},
        {ATS_AC_STATUS_1, "ATS_AC_STATUS_1", "ATS"},
        {ATS_AC_STATUS_2, "ATS_AC_STATUS_2", "ATS"},
        {ATS_AC_STATUS_3, "ATS_AC_STATUS_3", "ATS"},
        {ATS_AC_STATUS_4, "ATS_AC_STATUS_4", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_STATUS_1, "ATS_ACFAULT_CONFIGURATION_STATUS_1", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_STATUS_2, "ATS_ACFAULT_CONFIGURATION_STATUS_2", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_COMMAND_1, "ATS_ACFAULT_CONFIGURATION_COMMAND_1", "ATS"},
        {ATS_ACFAULT_CONFIGURATION_COMMAND_2, "ATS_ACFAULT_CONFIGURATION_COMMAND_2", "ATS"},
        {THERMOSTAT_AMBIENT_STATUS, "THERMOSTAT_AMBIENT_STATUS", "Thermostat"},
        {THERMOSTAT_STATUS_1, "THERMOSTAT_STATUS_1", "Thermostat"},
        {THERMOSTAT_STATUS_2, "THERMOSTAT_STATUS_2", "Thermostat"},
        {THERMOSTAT_COMMAND_1, "THERMOSTAT_COMMAND_1", "Thermostat"},
        {THERMOSTAT_COMMAND_2, "THERMOSTAT_COMMAND_2", "Thermostat"},
        {THERMOSTAT_SCHEDULE_STATUS_1, "THERMOSTAT_SCHEDULE_STATUS_1 / THERMOSTAT_SCHEDULE_STATUS_2", "Thermostat"},
        {THERMOSTAT_SCHEDULE_COMMAND_1, "THERMOSTAT_SCHEDULE_COMMAND_1", "Thermostat"},
        {THERMOSTAT_SCHEDULE_COMMAND_2, "THERMOSTAT_SCHEDULE_COMMAND_2", "Thermostat"}
    };
}

const char* LearnTable::relatedFamily(RVC_DGN dgn) {
    LearnedKind kind = classify(dgn);
    const char* family = kind == LearnedKind::Unknown ? "Unknown" : label(kind);
    for (const RelatedDgn& entry : relatedDgns) {
        if (entry.dgn == dgn) {
            family = entry.family;
            break;
        }
    }
    return family;
}

const char* LearnTable::dgnName(RVC_DGN dgn) {
    const char* name = "";
    for (const RelatedDgn& entry : relatedDgns) {
        if (entry.dgn == dgn) {
            name = entry.name;
            break;
        }
    }
    return name;
}

const char* LearnTable::typeName(const LearnRecord& r) {
    const char* name = "";
    switch (r.kind) {
        case LearnedKind::Light:
            name = (r.flags & LearnRecord::FLAG_PARTIAL_DIM) ? "DC_DimmableSwitch" : "DC_Switch";
            break;
        case LearnedKind::Awning:     name = "Awning";     break;
        case LearnedKind::Shade:      name = "Shades";     break;
        case LearnedKind::Tank:       name = "Tank";       break;
        case LearnedKind::Thermostat: name = "Thermostat"; break;
        case LearnedKind::FloorHeat:  name = "FloorHeat";  break;
        case LearnedKind::WaterPump:  name = "WaterPump";  break;
        case LearnedKind::DoorLock:   name = "DoorLock";   break;
        case LearnedKind::Generator:  name = "Generator";  break;
        case LearnedKind::Inverter:   name = "Inverter";   break;
        case LearnedKind::Charger:    name = "Charger";    break;
        case LearnedKind::Ats:        name = "ATS";        break;
        case LearnedKind::Battery:    name = "Battery";    break;
        default:                                           break;
    }
    return name;
}

const char* LearnTable::label(LearnedKind kind) {
    const char* text = "Device";
    switch (kind) {
        case LearnedKind::Light:      text = "Light";      break;
        case LearnedKind::Awning:     text = "Awning";     break;
        case LearnedKind::Shade:      text = "Shade";      break;
        case LearnedKind::Tank:       text = "Tank";       break;
        case LearnedKind::Thermostat: text = "Thermostat"; break;
        case LearnedKind::FloorHeat:  text = "Floor Heat"; break;
        case LearnedKind::WaterPump:  text = "Water Pump"; break;
        case LearnedKind::DoorLock:   text = "Lock";       break;
        case LearnedKind::Generator:  text = "Generator";  break;
        case LearnedKind::Inverter:   text = "Inverter";   break;
        case LearnedKind::Charger:    text = "Charger";    break;
        case LearnedKind::Ats:        text = "ATS";        break;
        case LearnedKind::Battery:    text = "Battery";    break;
        default:                                           break;
    }
    return text;
}

LearnRecord* LearnTable::findOrAlloc(RVC_DGN dgn, LearnedKind kind, uint8_t instance, uint8_t sourceAddress) {
    LearnRecord* result = nullptr;
    bool known = (kind != LearnedKind::Unknown);

    // known devices are keyed by kind+instance; unknown traffic by DGN+source
    for (size_t i = 0; (i < count_) && (result == nullptr); ++i) {
        LearnRecord& r = rows_[i];
        if (known) {
            if ((r.kind == kind) && (r.instance == instance)) {
                result = &r;
            }
        } else if ((r.kind == LearnedKind::Unknown) && (r.dgn == static_cast<uint32_t>(dgn)) &&
                   (r.sourceAddress == sourceAddress)) {
            result = &r;
        }
    }

    size_t limit = known ? MAX_RECORDS : (MAX_RECORDS - RESERVED_KNOWN);
    if ((result == nullptr) && (count_ < limit)) {
        result = &rows_[count_];
        memset(result, 0, sizeof(LearnRecord));
        result->dgn           = static_cast<uint32_t>(dgn);
        result->kind          = kind;
        result->instance      = known ? instance : UNKNOWN_INSTANCE;
        result->sourceAddress = sourceAddress;
        count_ = count_ + 1;
    }
    return result;
}

void LearnTable::observe(RVC_DGN dgn, uint8_t instance, uint8_t sourceAddress, const uint8_t* data, uint32_t nowSec) {
    if (data != nullptr) {
        LearnedKind kind = classify(dgn);
        LearnRecord* r = findOrAlloc(dgn, kind, instance, sourceAddress);
        if (r != nullptr) {
            if (r->hitCount == 0) {
                r->firstSeenSec = nowSec;
            }
            r->lastSeenSec   = nowSec;
            r->sourceAddress = sourceAddress;
            if (r->hitCount < UINT32_MAX) {
                r->hitCount = r->hitCount + 1;
            }
            memcpy(r->sample, data, sizeof(r->sample));
            r->flags |= LearnRecord::FLAG_HAS_SAMPLE;

            if (kind == LearnedKind::Light) {
                uint8_t level = data[DIMMER_LEVEL_INDEX];
                if ((level >= DIMMER_PARTIAL_LOW) && (level <= DIMMER_PARTIAL_HIGH)) {
                    r->flags |= LearnRecord::FLAG_PARTIAL_DIM;
                }
            }
        }
    }
}

bool LearnTable::load(const void* src, size_t rowCount) {
    bool ok = false;
    reset();
    if ((src != nullptr) && (rowCount <= MAX_RECORDS)) {
        memcpy(rows_, src, rowCount * sizeof(LearnRecord));
        count_ = rowCount;
        ok = true;
    }
    return ok;
}