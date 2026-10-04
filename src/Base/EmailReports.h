#pragma once

#ifdef FUTURE
#include <Arduino.h>

class EmailReports {
public:
    enum class Result : uint8_t { Staged, PendingReportExists, InvalidReport, StorageError, NotConfigured };

    EmailReports() = delete;
    EmailReports(const EmailReports&) = delete;
    EmailReports& operator=(const EmailReports&) = delete;
    EmailReports(EmailReports&&) = delete;
    EmailReports& operator=(EmailReports&&) = delete;
    ~EmailReports() = delete;

    static Result stageDiscoveryReport();
    static Result stageDiagnosticReport(const String& summary);
    static Result sendPendingReport();

private:
    static constexpr size_t MAX_REPORT_BYTES = 32768;
    static constexpr const char* OUTBOX_PATH = "/email_outbox.json";
    static Result stage(const char* category, const char* reportPath, const String& summary);
};
#endif