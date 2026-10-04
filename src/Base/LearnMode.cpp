#include "LearnMode.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <memory>
#include "ConfigTypes.h"
#include "DeviceFactory.h"
#include "Packet.h"
#include "EmailReports.h"

LearnTable       LearnMode::table_;
LearnMode::State LearnMode::state_        = LearnMode::State::Idle;
uint32_t         LearnMode::elapsedSec_   = 0;
uint32_t         LearnMode::durationSec_  = LearnMode::DEFAULT_DURATION_SEC;
uint32_t         LearnMode::lastSavedSec_ = 0;
uint32_t         LearnMode::lastTickMs_   = 0;
uint32_t         LearnMode::lastFinishAttemptMs_ = 0;
uint32_t         LearnMode::lastDiscoveryScanMs_ = 0;

namespace {
    bool isLightType(const char* type) {
        return (type != nullptr) &&
               ((strcmp(type, "DC_Switch") == 0) || (strcmp(type, "DC_DimmableSwitch") == 0));
    }

    // a dimmer seen as on/off still matches a profile that says dimmable, and vice versa
    bool typesCompatible(const char* profileType, const char* learnedType) {
        bool same = false;
        if ((profileType != nullptr) && (learnedType != nullptr)) {
            same = (strcmp(profileType, learnedType) == 0) ||
                   (isLightType(profileType) && isLightType(learnedType));
        }
        return same;
    }

    int findMatchingDevice(JsonArrayConst rows, const char* type, uint8_t instance) {
        int match = -1;
        int index = 0;
        for (JsonObjectConst row : rows) {
            if ((match < 0) &&
                ((row["rvcIndex"] | -1) == static_cast<int>(instance)) &&
                typesCompatible(row["type"] | "", type)) {
                match = index;
            }
            ++index;
        }
        return match;
    }

    String hexBytes(const uint8_t* data, size_t len) {
        static const char digits[] = "0123456789ABCDEF";
        String out;
        out.reserve(len * 2);
        for (size_t i = 0; i < len; ++i) {
            out += digits[data[i] >> 4];
            out += digits[data[i] & 0x0F];
        }
        return out;
    }
}

bool LearnMode::isLearning() {
    return state_ == State::Learning;
}

String LearnMode::discoveryJson() {
    JsonDocument document;
    document["learning"] = isLearning();
    document["elapsedSec"] = elapsedSec_;
    document["durationSec"] = durationSec_;
    document["recordCapacity"] = LearnTable::MAX_RECORDS;
    document["recordCount"] = table_.count();
    JsonArray records = document["records"].to<JsonArray>();
    for (size_t index = 0; index < table_.count(); ++index) {
        const LearnRecord& record = table_.row(index);
        JsonObject row = records.add<JsonObject>();
        bool decoded = record.kind != LearnedKind::Unknown;
        row["dgn"] = record.dgn;
        row["dgnName"] = LearnTable::dgnName(static_cast<RVC_DGN>(record.dgn));
        row["relatedFamily"] = LearnTable::relatedFamily(static_cast<RVC_DGN>(record.dgn));
        row["discoveryDecoded"] = decoded;
        if (decoded) row["type"] = LearnTable::typeName(record);
        row["rvcIndexVerified"] = decoded;
        row["rvcIndexSource"] = decoded ? "decodedInstance" : "payloadByte0Candidate";
        row["rvcIndex"] = decoded ? record.instance : record.sample[0];
        row["sourceAddress"] = record.sourceAddress;
        row["hits"] = record.hitCount;
        row["sample"] = hexBytes(record.sample, sizeof(record.sample));
    }
    String json;
    if (!document.overflowed()) serializeJson(document, json);
    return json;
}

void LearnMode::begin() {
    if (LittleFS.begin(false)) {
        if (load()) {
            if ((state_ == State::Complete) && !LittleFS.exists(DEVICES_FILE)) {
                start(DEFAULT_DURATION_SEC);
            }
        } else if (!LittleFS.exists(DEVICES_FILE)) {
            start(DEFAULT_DURATION_SEC);
        }
    } else {
        Serial.println("LearnMode: filesystem not mounted, learning disabled");
    }
    lastTickMs_ = millis();
    lastFinishAttemptMs_ = lastTickMs_ - FINISH_RETRY_MS;
    lastDiscoveryScanMs_ = lastTickMs_;
    if (isLearning()) {
        printStatus();
    }
}

void LearnMode::poll() {
    if (isLearning()) {
        uint32_t now   = millis();
        uint32_t delta = now - lastTickMs_;
        if (delta >= 1000) {
            uint32_t secs = delta / 1000;
            elapsedSec_ = elapsedSec_ + secs;
            lastTickMs_ = lastTickMs_ + (secs * 1000);
        }
        if ((elapsedSec_ - lastSavedSec_) >= SAVE_INTERVAL_SEC) {
            save();
        }
        if (elapsedSec_ >= durationSec_ && now - lastFinishAttemptMs_ >= FINISH_RETRY_MS) {
            finish();
        }
    } else if (millis() - lastDiscoveryScanMs_ >= DISCOVERY_SCAN_MS && LittleFS.exists(DEVICES_FILE)) {
        lastDiscoveryScanMs_ = millis();
        if (!stagePendingDevices()) Serial.println("LearnMode: pending discoveries could not be saved; retrying later");
    }
}

void LearnMode::observe(RVC_DGN dgn, uint8_t sourceAddress, uint8_t* data) {
    if ((data != nullptr) && (sourceAddress != SOURCE_ADDRESS)) {
        uint8_t instance = 0xFF;
        bool    haveInstance = DeviceFactory::instanceFromData(dgn, data, instance);
        if (haveInstance || (LearnTable::classify(dgn) == LearnedKind::Unknown)) {
            uint32_t observedSec = isLearning() ? elapsedSec_ : millis() / 1000;
            table_.observe(dgn, instance, sourceAddress, data, observedSec);
        }
    }
}

void LearnMode::start(uint32_t durationSec) {
    table_.reset();
    state_        = State::Learning;
    elapsedSec_   = 0;
    durationSec_  = (durationSec > 0) ? durationSec : DEFAULT_DURATION_SEC;
    lastSavedSec_ = 0;
    lastTickMs_   = millis();
    lastFinishAttemptMs_ = lastTickMs_ - FINISH_RETRY_MS;
    save();
    Serial.printf("LearnMode: learning started for %lu hours\n",
                  static_cast<unsigned long>(durationSec_ / 3600UL));
}

void LearnMode::cancel() {
    if (isLearning()) {
        bool restored = true;
        if (LittleFS.exists(DEVICES_BACKUP)) {
            restored = LittleFS.rename(DEVICES_BACKUP, DEVICES_FILE);
        }
        if (restored) {
            state_ = State::Idle;
            LittleFS.remove(LEARN_FILE);
            Serial.println("LearnMode: learning cancelled; previous configuration restored when available");
        } else {
            Serial.println("LearnMode: cancel failed to restore configuration; learning retained");
        }
    } else {
        Serial.println("LearnMode: not learning; configuration left unchanged");
    }
}

void LearnMode::finish() {
    CoachSpec coach;
    bool      profileFound = false;
    bool complete = false;
    String profile;
    lastFinishAttemptMs_ = millis();
    do {
        if (!DeviceFactory::loadCoachSpec(COACH_FILE, coach)) break;
        profile = profilePath(coach);
        if (!writeDevicesJson(profile, profileFound)) break;
        if (!writeReport(coach, profile, profileFound)) break;
        if (!replaceFile(DEVICES_TMP, DEVICES_FILE)) break;
        state_ = State::Complete;
        if (!save()) {
            state_ = State::Learning;
            break;
        }
        complete = true;
    } while (false);
    if (complete) {
        EmailReports::Result staged = EmailReports::stageDiscoveryReport();
        Serial.printf("EmailReports: discovery staging result=%u\n",
                      static_cast<unsigned int>(staged));
        if (staged == EmailReports::Result::Staged && EmailReports::relayConfigured()) {
            EmailReports::Result delivered = EmailReports::sendPendingReport();
            Serial.printf("EmailReports: discovery delivery result=%u\n",
                          static_cast<unsigned int>(delivered));
        }
    }
    if (complete) {
        Serial.printf("LearnMode: wrote %s (profile %s %s), rebooting\n",
                      DEVICES_FILE, profile.c_str(), profileFound ? "matched" : "not found");
        Serial.flush();
        ESP.restart();
    } else {
        LittleFS.remove(DEVICES_TMP);
        save();
        Serial.println("LearnMode: completion failed; learning retained, automatic retry in one minute");
    }
}

bool LearnMode::load() {
    bool ok = false;

    // a power cut between write and rename leaves only the temp file
    if (!LittleFS.exists(LEARN_FILE) && LittleFS.exists(LEARN_TMP)) {
        LittleFS.rename(LEARN_TMP, LEARN_FILE);
    }

    File f = LittleFS.open(LEARN_FILE, "r");
    if (f) {
        FileHeader h;
        if ((f.read(reinterpret_cast<uint8_t*>(&h), sizeof(h)) == sizeof(h)) &&
            (h.magic == FILE_MAGIC) && (h.version == FILE_VERSION) &&
            (h.rowCount <= LearnTable::MAX_RECORDS)) {
            size_t bytes = static_cast<size_t>(h.rowCount) * sizeof(LearnRecord);
            std::unique_ptr<uint8_t[]> buffer(new uint8_t[bytes > 0 ? bytes : 1]);
            if (f.read(buffer.get(), bytes) == bytes) {
                ok = table_.load(buffer.get(), h.rowCount);
            }
            if (ok) {
                state_        = h.state;
                elapsedSec_   = h.elapsedSec;
                durationSec_  = (h.durationSec > 0) ? h.durationSec : DEFAULT_DURATION_SEC;
                lastSavedSec_ = elapsedSec_;
            }
        }
        f.close();
    }
    return ok;
}

bool LearnMode::save() {
    bool ok = false;
    File f  = LittleFS.open(LEARN_TMP, "w");
    if (f) {
        FileHeader h = {};
        h.magic       = FILE_MAGIC;
        h.version     = FILE_VERSION;
        h.rowCount    = static_cast<uint16_t>(table_.count());
        h.elapsedSec  = elapsedSec_;
        h.durationSec = durationSec_;
        h.state       = state_;
        bool wrote = (f.write(reinterpret_cast<const uint8_t*>(&h), sizeof(h)) == sizeof(h)) &&
                     (f.write(static_cast<const uint8_t*>(table_.bytes()), table_.byteSize()) == table_.byteSize());
        f.flush();
        f.close();
        ok = wrote && replaceFile(LEARN_TMP, LEARN_FILE);
    }
    if (ok) {
        lastSavedSec_ = elapsedSec_;
    } else {
        LittleFS.remove(LEARN_TMP);
        Serial.println("LearnMode: failed to save learn state");
    }
    return ok;
}

bool LearnMode::replaceFile(const char* tmpPath, const char* destPath) {
    bool ok = LittleFS.rename(tmpPath, destPath);
    return ok;
}

String LearnMode::slug(const String& text) {
    String out;
    bool   dash = false;
    for (size_t i = 0; i < text.length(); ++i) {
        char c = static_cast<char>(tolower(static_cast<unsigned char>(text[i])));
        if (isalnum(static_cast<unsigned char>(c))) {
            out += c;
            dash = false;
        } else if (!dash && (out.length() > 0)) {
            out += '-';
            dash = true;
        }
    }
    if (out.endsWith("-")) {
        out.remove(out.length() - 1);
    }
    return out;
}

String LearnMode::profilePath(const CoachSpec& coach) {
    return String("/profiles/") + slug(coach.make) + "-" + slug(coach.model) + "-" + slug(coach.floorplan) + ".json";
}

bool LearnMode::writeDevicesJson(const String& profile, bool& profileFound) {
    bool ok = false;
    profileFound = false;
    do {
        JsonDocument profileDoc;
        File profileFile = LittleFS.open(profile, "r");
        if (profileFile) {
            DeserializationError error = deserializeJson(profileDoc, profileFile);
            profileFile.close();
            if (error || !profileDoc.is<JsonArray>()) break;
            profileFound = true;
        } else if (LittleFS.exists(profile)) {
            break;
        }
        JsonArrayConst profileRows = profileDoc.as<JsonArrayConst>();

        JsonDocument previousDoc;
        File previousFile = LittleFS.open(DEVICES_BACKUP, "r");
        if (previousFile) {
            DeserializationError error = deserializeJson(previousDoc, previousFile);
            previousFile.close();
            if (error || !previousDoc.is<JsonArray>()) break;
        } else if (LittleFS.exists(DEVICES_BACKUP)) {
            break;
        }
        JsonArrayConst previousRows = previousDoc.as<JsonArrayConst>();

        JsonDocument out;
        JsonArray rows = out.to<JsonArray>();
        for (JsonObjectConst previous : previousRows) {
            JsonObject retained = rows.add<JsonObject>();
            retained.set(previous);
            retained["learned"] = false;
        }

        for (size_t index = 0; index < table_.count(); ++index) {
            const LearnRecord& record = table_.row(index);
            if ((record.kind != LearnedKind::Unknown) && (record.hitCount >= MIN_HITS)) {
                const char* type = LearnTable::typeName(record);
                int previous = findMatchingDevice(rows, type, record.instance);
                JsonObject device;
                if (previous >= 0) {
                    device = rows[previous].as<JsonObject>();
                } else {
                    device = rows.add<JsonObject>();
                    int match = findMatchingDevice(profileRows, type, record.instance);
                    if (match >= 0) {
                        device.set(profileRows[match].as<JsonObjectConst>());
                        device.remove("aid");
                        device["enabled"] = true;
                    } else {
                        device["enabled"] = false;
                        device["type"] = type;
                        device["rvcIndex"] = record.instance;
                        device["name"] = String(LearnTable::label(record.kind)) + " " + String(record.instance);
                        device["order"] = 100;
                        device["room"] = "";
                    }
                }
                device["sourceAddress"] = record.sourceAddress;
                device["learned"] = true;
            }
        }

        for (JsonObjectConst defaults : profileRows) {
            const char* type = defaults["type"] | "";
            uint8_t instance = defaults["rvcIndex"] | 0;
            if (findMatchingDevice(rows, type, instance) < 0) {
                JsonObject device = rows.add<JsonObject>();
                device.set(defaults);
                device.remove("aid");
                device["enabled"] = true;
                device["learned"] = false;
            }
        }

        if (out.overflowed()) break;
        File output = LittleFS.open(DEVICES_TMP, "w");
        if (!output) break;
        size_t expected = measureJsonPretty(out);
        size_t written = serializeJsonPretty(out, output);
        output.flush();
        output.close();
        ok = written == expected;
    } while (false);
    if (!ok) LittleFS.remove(DEVICES_TMP);
    return ok;
}

bool LearnMode::writeReport(const CoachSpec& coach, const String& profile, bool profileFound) {
    JsonDocument doc;
    JsonObject c = doc["coach"].to<JsonObject>();
    c["year"]      = coach.year;
    c["make"]      = coach.make;
    c["model"]     = coach.model;
    c["floorplan"] = coach.floorplan;
    c["coachId"]   = coach.coachId;

    doc["learnedSec"]   = elapsedSec_;
    doc["durationSec"]  = durationSec_;
    doc["profile"]      = profile;
    doc["profileFound"] = profileFound;

    JsonArray devices = doc["devices"].to<JsonArray>();
    JsonArray unknown = doc["unknownTraffic"].to<JsonArray>();
    char dgnText[8];

    for (size_t i = 0; i < table_.count(); ++i) {
        const LearnRecord& r = table_.row(i);
        snprintf(dgnText, sizeof(dgnText), "%05lX", static_cast<unsigned long>(r.dgn));
        JsonObject o = (r.kind == LearnedKind::Unknown) ? unknown.add<JsonObject>() : devices.add<JsonObject>();
        if (r.kind != LearnedKind::Unknown) {
            o["type"]     = LearnTable::typeName(r);
            o["rvcIndex"] = r.instance;
            o["rvcIndexVerified"] = true;
            o["rvcIndexSource"] = "decodedInstance";
        } else {
            if (r.flags & LearnRecord::FLAG_HAS_SAMPLE) {
                o["rvcIndex"] = r.sample[0];
            } else {
                o["rvcIndex"] = nullptr;
            }
            o["rvcIndexVerified"] = false;
            o["rvcIndexSource"] = "payloadByte0Candidate";
        }
        o["dgn"]           = dgnText;
        o["dgnName"]       = LearnTable::dgnName(static_cast<RVC_DGN>(r.dgn));
        o["relatedFamily"] = LearnTable::relatedFamily(static_cast<RVC_DGN>(r.dgn));
        o["discoveryDecoded"] = r.kind != LearnedKind::Unknown;
        o["sourceAddress"] = r.sourceAddress;
        o["hits"]          = r.hitCount;
        o["firstSeenSec"]  = r.firstSeenSec;
        o["lastSeenSec"]   = r.lastSeenSec;
        o["sample"]        = hexBytes(r.sample, sizeof(r.sample));
    }

    bool ok = false;
    if (!doc.overflowed()) {
        File output = LittleFS.open("/learn_report.tmp", "w");
        if (output) {
            size_t expected = measureJson(doc);
            size_t written = serializeJson(doc, output);
            output.flush();
            output.close();
            ok = written == expected && replaceFile("/learn_report.tmp", REPORT_FILE);
        }
    }
    if (!ok) LittleFS.remove("/learn_report.tmp");
    return ok;
}

bool LearnMode::stagePendingDevices() {
    bool saved = false;
    do {
        File input = LittleFS.open(DEVICES_FILE, "r");
        if (!input) break;
        JsonDocument document;
        DeserializationError error = deserializeJson(document, input);
        input.close();
        JsonArray devices = document.as<JsonArray>();
        if (error || devices.isNull()) break;
        bool changed = false;
        for (size_t index = 0; index < table_.count(); ++index) {
            const LearnRecord& record = table_.row(index);
            if (record.kind != LearnedKind::Unknown && record.hitCount >= MIN_HITS &&
                findMatchingDevice(devices, LearnTable::typeName(record), record.instance) < 0) {
                JsonObject device = devices.add<JsonObject>();
                device["enabled"] = false;
                device["ignored"] = false;
                device["type"] = LearnTable::typeName(record);
                device["rvcIndex"] = record.instance;
                device["sourceAddress"] = record.sourceAddress;
                device["name"] = String(LearnTable::label(record.kind)) + " " + String(record.instance);
                device["room"] = "";
                device["order"] = 100;
                device["learned"] = true;
                device["discoveredAtUptimeMs"] = millis();
                changed = true;
            }
        }
        if (!changed) {
            saved = true;
            break;
        }
        if (document.overflowed() || !DeviceFactory::validateAndReserveConfiguration(document)) break;
        File output = LittleFS.open("/devices.discovery.tmp", "w");
        if (!output) break;
        size_t expected = measureJson(document);
        size_t written = serializeJson(document, output);
        output.flush();
        output.close();
        saved = written == expected && replaceFile("/devices.discovery.tmp", DEVICES_FILE);
    } while (false);
    if (!saved) LittleFS.remove("/devices.discovery.tmp");
    return saved;
}

void LearnMode::printStatus() {
    static const char* names[] = { "idle", "learning", "complete" };
    size_t known = 0;
    for (size_t i = 0; i < table_.count(); ++i) {
        if (table_.row(i).kind != LearnedKind::Unknown) {
            known = known + 1;
        }
    }
    Serial.printf("LearnMode: %s, %lu of %lu s, %u records (%u devices)\n",
                  names[static_cast<uint8_t>(state_) % 3],
                  static_cast<unsigned long>(elapsedSec_),
                  static_cast<unsigned long>(durationSec_),
                  static_cast<unsigned int>(table_.count()),
                  static_cast<unsigned int>(known));
}

void LearnMode::handleCommand(const String& args) {
    String cmd = args;
    cmd.trim();
    cmd.toLowerCase();

    if (cmd.startsWith("start")) {
        long hours = cmd.substring(5).toInt();
        bool ready = !isLearning();
        if (!ready) {
            Serial.println("LearnMode: already learning; finish or cancel before starting another run");
        } else if (LittleFS.exists(DEVICES_FILE)) {
            ready = LittleFS.rename(DEVICES_FILE, DEVICES_BACKUP);
            if (!ready) Serial.println("LearnMode: backup failed; existing configuration left unchanged");
        }
        if (ready) {
            start((hours > 0) ? static_cast<uint32_t>(hours) * 3600UL : DEFAULT_DURATION_SEC);
        }
    } else if (cmd == "finish") {
        if (isLearning()) {
            finish();
        } else {
            Serial.println("LearnMode: not learning");
        }
    } else if (cmd == "cancel") {
        cancel();
    } else {
        printStatus();
    }
}
