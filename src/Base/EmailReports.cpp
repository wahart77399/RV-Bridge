#include "EmailReports.h"

#ifdef FUTURE
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <esp_random.h>

EmailReports::Result EmailReports::stageDiscoveryReport() {
    return stage("discovery", "/learn_report.json", "");
}

EmailReports::Result EmailReports::stageDiagnosticReport(const String& summary) {
    Result result = Result::InvalidReport;
    if (!summary.isEmpty() && summary.length() <= 1024) {
        result = stage("diagnostics", nullptr, summary);
    }
    return result;
}

EmailReports::Result EmailReports::stage(const char* category, const char* reportPath, const String& summary) {
    Result result = Result::InvalidReport;
    do {
        if (!LittleFS.begin(false)) {
            result = Result::StorageError;
            break;
        }
        if (LittleFS.exists(OUTBOX_PATH)) {
            result = Result::PendingReportExists;
            break;
        }
        File coachFile = LittleFS.open("/coach.json", "r");
        if (!coachFile || coachFile.size() > MAX_REPORT_BYTES) break;
        JsonDocument coach;
        DeserializationError error = deserializeJson(coach, coachFile);
        coachFile.close();
        if (error || !coach.is<JsonObject>()) break;
        JsonArrayConst owners = coach["ownerEmails"].as<JsonArrayConst>();
        if (owners.isNull() || owners.size() == 0 || owners.size() > 5 ||
            (reportPath == nullptr && !(coach["diagnosticEmails"] | false))) {
            result = Result::NotConfigured;
            break;
        }

        JsonDocument envelope;
        char requestId[17];
        snprintf(requestId, sizeof(requestId), "%08lX%08lX",
                 static_cast<unsigned long>(esp_random()), static_cast<unsigned long>(esp_random()));
        envelope["schemaVersion"] = 1;
        envelope["requestId"] = requestId;
        envelope["category"] = category;
        envelope["ownerEmails"].set(owners);
        envelope["shareWithSupport"] = coach["shareWithSupport"] | false;
        envelope["summary"] = summary;
        JsonObject identity = envelope["coach"].to<JsonObject>();
        for (const char* field : {"year", "make", "model", "floorplan", "coachId"}) {
            identity[field].set(coach[field]);
        }
        if (reportPath != nullptr) {
            File reportFile = LittleFS.open(reportPath, "r");
            if (!reportFile || reportFile.size() > MAX_REPORT_BYTES) break;
            JsonDocument report;
            error = deserializeJson(report, reportFile);
            reportFile.close();
            if (error || !report.is<JsonObject>()) break;
            envelope["report"].set(report.as<JsonObjectConst>());
            envelope["appleHomeInstructions"] = "On the coach Wi-Fi, open Apple Home, Add Accessory, More Options, select SmartCoach, enter its setup code and assign rooms. Keep a supported Apple TV or HomePod on the coach network as a home hub for remote access and automations.";
        }
        size_t expected = measureJson(envelope);
        if (envelope.overflowed() || expected > MAX_REPORT_BYTES) break;
        result = Result::StorageError;
        File output = LittleFS.open("/email_outbox.tmp", "w");
        if (!output) break;
        size_t written = serializeJson(envelope, output);
        output.flush();
        output.close();
        if (written != expected || !LittleFS.rename("/email_outbox.tmp", OUTBOX_PATH)) {
            LittleFS.remove("/email_outbox.tmp");
            break;
        }
        result = Result::Staged;
    } while (false);
    return result;
}

EmailReports::Result EmailReports::sendPendingReport() {
    return Result::NotConfigured;
}
#endif