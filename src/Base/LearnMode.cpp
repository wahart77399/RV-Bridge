#include "LearnMode.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <memory>
#include <vector>
#include "ConfigTypes.h"
#include "DeviceFactory.h"
#include "Packet.h"

LearnTable       LearnMode::table_;
LearnMode::State LearnMode::state_        = LearnMode::State::Idle;
uint32_t         LearnMode::elapsedSec_   = 0;
uint32_t         LearnMode::durationSec_  = LearnMode::DEFAULT_DURATION_SEC;
uint32_t         LearnMode::lastSavedSec_ = 0;
uint32_t         LearnMode::lastTickMs_   = 0;

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

    int findProfileMatch(JsonArrayConst rows, const std::vector<bool>& used, const LearnRecord& r) {
        int match = -1;
        const char* learnedType = LearnTable::typeName(r);
        int i = 0;
        for (JsonObjectConst row : rows) {
            if ((match < 0) && !used[i] &&
                ((row["rvcIndex"] | -1) == static_cast<int>(r.instance)) &&
                typesCompatible(row["type"] | "", learnedType)) {
                match = i;
            }
            i = i + 1;
        }
        return match;
    }

    int findPreviousMatch(JsonArrayConst rows, const std::vector<bool>& used, const LearnRecord& r) {
        int match = -1;
        const char* learnedType = LearnTable::typeName(r);
        int i = 0;
        for (JsonObjectConst row : rows) {
            if ((match < 0) && !used[i] &&
                ((row["rvcIndex"] | -1) == static_cast<int>(r.instance)) &&
                typesCompatible(row["type"] | "", learnedType)) {
                match = i;
            }
            i = i + 1;
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
        if (elapsedSec_ >= durationSec_) {
            finish();
        }
    }
}

void LearnMode::observe(RVC_DGN dgn, uint8_t sourceAddress, uint8_t* data) {
    if (isLearning() && (data != nullptr) && (sourceAddress != SOURCE_ADDRESS)) {
        uint8_t instance = 0xFF;
        bool    haveInstance = DeviceFactory::instanceFromData(dgn, data, instance);
        if (haveInstance || (LearnTable::classify(dgn) == LearnedKind::Unknown)) {
            table_.observe(dgn, instance, sourceAddress, data, elapsedSec_);
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
    save();
    Serial.printf("LearnMode: learning started for %lu hours\n",
                  static_cast<unsigned long>(durationSec_ / 3600UL));
}

void LearnMode::cancel() {
    state_ = State::Idle;
    LittleFS.remove(LEARN_FILE);
    if (!LittleFS.exists(DEVICES_FILE) && LittleFS.exists(DEVICES_BACKUP)) {
        LittleFS.rename(DEVICES_BACKUP, DEVICES_FILE);
    }
    Serial.println("LearnMode: learning cancelled");
}

void LearnMode::finish() {
    CoachSpec coach;
    bool      profileFound = false;

    DeviceFactory::loadCoachSpec(COACH_FILE, coach);
    String profile = profilePath(coach);

    bool written = writeDevicesJson(profile, profileFound);
    writeReport(coach, profile, profileFound);
    state_ = State::Complete;
    save();

    if (written) {
        Serial.printf("LearnMode: wrote %s (profile %s %s), rebooting\n",
                      DEVICES_FILE, profile.c_str(), profileFound ? "matched" : "not found");
        Serial.flush();
        ESP.restart();
    } else {
        Serial.printf("LearnMode: could not write %s; learning will restart on next boot\n", DEVICES_FILE);
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
        f.close();
        ok = wrote && replaceFile(LEARN_TMP, LEARN_FILE);
    }
    if (ok) {
        lastSavedSec_ = elapsedSec_;
    } else {
        Serial.println("LearnMode: failed to save learn state");
    }
    return ok;
}

bool LearnMode::replaceFile(const char* tmpPath, const char* destPath) {
    bool ok = LittleFS.rename(tmpPath, destPath);
    if (!ok) {
        LittleFS.remove(destPath);
        ok = LittleFS.rename(tmpPath, destPath);
    }
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
    JsonDocument profileDoc;
    File pf = LittleFS.open(profile, "r");
    if (pf) {
        profileFound = !deserializeJson(profileDoc, pf) && profileDoc.is<JsonArray>();
        pf.close();
    }
    JsonArrayConst    profileRows = profileDoc.as<JsonArrayConst>();
    std::vector<bool> used(profileRows.size(), false);

    JsonDocument previousDoc;
    File previousFile = LittleFS.open(DEVICES_BACKUP, "r");
    if (previousFile) {
        deserializeJson(previousDoc, previousFile);
        previousFile.close();
    }
    JsonArrayConst previousRows = previousDoc.as<JsonArrayConst>();
    std::vector<bool> previousUsed(previousRows.size(), false);

    JsonDocument out;
    JsonArray    rows = out.to<JsonArray>();

    for (size_t i = 0; i < table_.count(); ++i) {
        const LearnRecord& r = table_.row(i);
        if ((r.kind != LearnedKind::Unknown) && (r.hitCount >= MIN_HITS)) {
            JsonObject o     = rows.add<JsonObject>();
            int        match = profileFound ? findProfileMatch(profileRows, used, r) : -1;
            if (match >= 0) {
                o.set(profileRows[match].as<JsonObjectConst>());
                o["enabled"] = true;
                used[match]  = true;
            } else {
                int previous = findPreviousMatch(previousRows, previousUsed, r);
                if (previous >= 0) {
                    o.set(previousRows[previous].as<JsonObjectConst>());
                    previousUsed[previous] = true;
                    o["type"] = LearnTable::typeName(r);
                    o["rvcIndex"] = r.instance;
                } else {
                    o["enabled"]  = false;
                    o["type"]     = LearnTable::typeName(r);
                    o["rvcIndex"] = r.instance;
                    o["name"]     = String(LearnTable::label(r.kind)) + " " + String(r.instance);
                    o["order"]    = 100;
                    o["room"]     = "";
                }
            }
            o["sourceAddress"] = r.sourceAddress;
            o["learned"]       = true;
        }
    }

    // trust the profile for devices that stayed quiet (e.g. shades never moved)
    for (size_t i = 0; i < used.size(); ++i) {
        if (!used[i]) {
            JsonObject o = rows.add<JsonObject>();
            o.set(profileRows[i].as<JsonObjectConst>());
            o["enabled"] = true;
            o["learned"] = false;
        }
    }

    bool ok = false;
    File f  = LittleFS.open(DEVICES_TMP, "w");
    if (f) {
        size_t n = serializeJsonPretty(out, f);
        f.close();
        ok = (n > 0) && replaceFile(DEVICES_TMP, DEVICES_FILE);
    }
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
        }
        o["dgn"]           = dgnText;
        o["sourceAddress"] = r.sourceAddress;
        o["hits"]          = r.hitCount;
        o["firstSeenSec"]  = r.firstSeenSec;
        o["lastSeenSec"]   = r.lastSeenSec;
        o["sample"]        = hexBytes(r.sample, sizeof(r.sample));
    }

    bool ok = false;
    File f  = LittleFS.open(REPORT_FILE, "w");
    if (f) {
        ok = serializeJson(doc, f) > 0;
        f.close();
    }
    return ok;
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
        if (LittleFS.exists(DEVICES_FILE)) {
            LittleFS.remove(DEVICES_BACKUP);
            LittleFS.rename(DEVICES_FILE, DEVICES_BACKUP);
        }
        start((hours > 0) ? static_cast<uint32_t>(hours) * 3600UL : DEFAULT_DURATION_SEC);
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
