#pragma once
#include <Arduino.h>
#include "DGN.h"
#include "LearnRecord.h"

struct CoachSpec;

/**
 * Listen-only discovery of the coach's RV-C devices.
 * Runs while no /devices.json exists (or after "LEARN start"), keeps its clock
 * and inventory in /learn.bin across reboots, then writes /devices.json and
 * /learn_report.json and reboots into the learned configuration.
 */
class LearnMode {
public:
    LearnMode() = delete;
    ~LearnMode() = delete;
    LearnMode(const LearnMode&) = delete;
    LearnMode& operator=(const LearnMode&) = delete;
    LearnMode(LearnMode&&) = delete;
    LearnMode& operator=(LearnMode&&) = delete;

    static void begin();
    static void poll();
    static void observe(RVC_DGN dgn, uint8_t sourceAddress, uint8_t* data);
    static bool isLearning();
    static void handleCommand(const String& args);   // "status" | "start [hours]" | "finish" | "cancel"
    static String discoveryJson();

private:
    enum class State : uint8_t { Idle = 0, Learning = 1, Complete = 2 };

    struct FileHeader {
        uint32_t magic;
        uint16_t version;
        uint16_t rowCount;
        uint32_t elapsedSec;
        uint32_t durationSec;
        State    state;
        uint8_t  reserved[3];
    };

    static constexpr uint32_t    FILE_MAGIC           = 0x4E4C4353;   // "SCLN"
    static constexpr uint16_t    FILE_VERSION         = 1;
    static constexpr uint32_t    DEFAULT_DURATION_SEC = 72UL * 3600UL;
    static constexpr uint32_t    SAVE_INTERVAL_SEC    = 300;
    static constexpr uint32_t    FINISH_RETRY_MS      = 60000;
    static constexpr uint32_t    DISCOVERY_SCAN_MS    = 60000;
    static constexpr uint32_t    MIN_HITS             = 3;            // ignore one-off frames
    static constexpr const char* LEARN_FILE           = "/learn.bin";
    static constexpr const char* LEARN_TMP            = "/learn.tmp";
    static constexpr const char* DEVICES_FILE         = "/devices.json";
    static constexpr const char* DEVICES_TMP          = "/devices.tmp";
    static constexpr const char* DEVICES_BACKUP       = "/devices.prev.json";
    static constexpr const char* REPORT_FILE          = "/learn_report.json";
    static constexpr const char* COACH_FILE           = "/coach.json";

    static LearnTable table_;
    static State      state_;
    static uint32_t   elapsedSec_;
    static uint32_t   durationSec_;
    static uint32_t   lastSavedSec_;
    static uint32_t   lastTickMs_;
    static uint32_t   lastFinishAttemptMs_;
    static uint32_t   lastDiscoveryScanMs_;

    static void   start(uint32_t durationSec);
    static void   finish();
    static void   cancel();
    static bool   load();
    static bool   save();
    static bool   writeDevicesJson(const String& profile, bool& profileFound);
    static bool   stagePendingDevices();
    static bool   writeReport(const CoachSpec& coach, const String& profile, bool profileFound);
    static bool   replaceFile(const char* tmpPath, const char* destPath);
    static String profilePath(const CoachSpec& coach);
    static String slug(const String& text);
    static void   printStatus();
};
