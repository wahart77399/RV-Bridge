#pragma once

#include <Arduino.h>

class EmailReports {
public:
    enum class Result : uint8_t {
        Staged, PendingReportExists, InvalidReport, StorageError, NotConfigured,
        Delivered, RetryableFailure, Rejected, DeliveryUncertain, NoPendingReport
    };

    EmailReports() = delete;
    EmailReports(const EmailReports&) = delete;
    EmailReports& operator=(const EmailReports&) = delete;
    EmailReports(EmailReports&&) = delete;
    EmailReports& operator=(EmailReports&&) = delete;
    ~EmailReports() = delete;

    static Result stageDiscoveryReport();
    static Result stageCurrentInventoryReport();
    static Result stageDiagnosticReport(const String& summary);
    static bool configureRelay(const String& endpoint, const String& token);
    static bool relayConfigured();
    static Result sendPendingReport();

private:
    static constexpr size_t MAX_REPORT_BYTES = 32768;
    static constexpr const char* OUTBOX_PATH = "/email_outbox.json";
    static Result stage(const char* category, const char* reportPath, const String& summary);
    static bool loadRelaySettings(String& endpoint, String& token);
    static bool validEndpoint(const String& endpoint);
    static bool validToken(const String& token);
    static Result postRelayRequest(const String& endpoint, const String& payload);
};