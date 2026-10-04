function runRelayTests() {
    var tests = [
        ["consented discovery goes only to registered owner", testDiscoveryOwnerOnly_],
        ["current inventory is labeled accurately", testCurrentInventoryLabel_],
        ["invalid token is rejected", testRejectsInvalidToken_],
        ["unregistered recipient is rejected", testRejectsUnregisteredRecipient_],
        ["discovery consent is required", testRequiresDiscoveryConsent_],
        ["diagnostic consent is required", testRequiresDiagnosticConsent_],
        ["support requires separate opt-in", testSupportOptIn_],
        ["request retry is deduplicated", testDeduplicatesRetry_],
        ["UTF-8 request byte limit is enforced", testRejectsOversizeUtf8_],
        ["per-device daily cap is enforced", testDailyLimit_]
    ];
    var failures = [];
    for (var index = 0; index < tests.length; index++) {
        try {
            tests[index][1]();
        } catch (error) {
            failures.push(tests[index][0] + ": " + error.message);
        }
    }
    if (failures.length > 0) throw new Error(failures.join("\n"));
    return tests.length + "/" + tests.length + " relay tests passed";
}

function testDiscoveryOwnerOnly_() {
    var services = testServices_();
    var request = discoveryRequest_();
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.ok && services.sent.length === 1, "consented report was not sent");
    assertTest_(services.sent[0].to === "owner@example.com" && services.sent[0].cc === "", "unexpected recipient");
    assertTest_(services.sent[0].body.indexOf("relayToken") < 0, "relay token leaked into message");
}

function testCurrentInventoryLabel_() {
    var services = testServices_();
    var request = discoveryRequest_();
    request.reportKind = "currentInventory";
    request.summary = "Current device inventory snapshot; no new learning run was performed.";
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.ok && services.sent.length === 1, "current inventory report was not sent");
    assertTest_(services.sent[0].subject.indexOf("current inventory report") >= 0, "inventory subject is mislabeled");
    assertTest_(services.sent[0].body.indexOf("not a new learning run") >= 0, "inventory disclaimer is missing");
}

function testRejectsInvalidToken_() {
    var services = testServices_();
    var request = discoveryRequest_();
    request.relayToken = "b".repeat(64);
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.code === "unauthorized_device" && services.sent.length === 0, "invalid token was accepted");
}

function testRejectsUnregisteredRecipient_() {
    var services = testServices_();
    var request = discoveryRequest_();
    request.ownerEmails = ["attacker@example.com"];
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.code === "recipient_not_allowed" && services.sent.length === 0, "unregistered recipient was accepted");
}

function testRequiresDiscoveryConsent_() {
    var services = testServices_();
    var request = discoveryRequest_();
    request.ownerEmailConsent = false;
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.code === "owner_consent_required" && services.sent.length === 0, "unconsented discovery report was sent");
}

function testRequiresDiagnosticConsent_() {
    var services = testServices_();
    var request = discoveryRequest_();
    request.category = "diagnostics";
    request.ownerEmailConsent = false;
    request.diagnosticEmailConsent = false;
    request.summary = "CAN status";
    delete request.report;
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.code === "diagnostic_consent_required" && services.sent.length === 0, "unconsented diagnostics were sent");
}

function testSupportOptIn_() {
    var services = testServices_();
    var request = discoveryRequest_();
    request.shareWithSupport = true;
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.ok && services.sent[0].cc === "support@example.com", "explicit support opt-in was not honored");

    var deniedServices = testServices_();
    deniedServices.registration.supportAllowed = false;
    deniedServices.properties.DEVICE_REGISTRY_JSON = JSON.stringify({ "test-coach": deniedServices.registration });
    var denied = processRelayRequest_(testEvent_(request), deniedServices);
    assertTest_(denied.code === "support_sharing_not_configured" && deniedServices.sent.length === 0, "support copy bypassed registration policy");
}

function testDeduplicatesRetry_() {
    var services = testServices_();
    var event = testEvent_(discoveryRequest_());
    var first = processRelayRequest_(event, services);
    var second = processRelayRequest_(event, services);
    assertTest_(first.ok && second.ok && second.duplicate === true && services.sent.length === 1, "retry sent a duplicate email");
}

function testDailyLimit_() {
    var services = testServices_();
    services.properties["attempts_" + services.hashToken("test-coach").substring(0, 16)] = JSON.stringify({ day: "20261004", count: 3 });
    var result = processRelayRequest_(testEvent_(discoveryRequest_()), services);
    assertTest_(result.code === "daily_limit_reached" && services.sent.length === 0, "daily send cap was bypassed");
}

function testRejectsOversizeUtf8_() {
    var services = testServices_();
    var request = discoveryRequest_();
    request.summary = "é".repeat(17000);
    var result = processRelayRequest_(testEvent_(request), services);
    assertTest_(result.code === "body_too_large" && services.sent.length === 0, "oversize UTF-8 body was accepted");
}

function testServices_() {
    var services = {
        registration: { tokenSha256: "valid-token-hash", ownerEmails: ["owner@example.com"], supportAllowed: true },
        properties: { SUPPORT_EMAIL: "support@example.com" },
        cache: {},
        sent: [],
        getProperty: function (key) { return this.properties[key] || ""; },
        setProperty: function (key, value) { this.properties[key] = value; },
        hashToken: function (value) { return value === "a".repeat(64) ? "valid-token-hash" : "invalid-token-hash"; },
        tryLock: function () { return true; },
        releaseLock: function () {},
        cacheGet: function (key) { return this.cache[key] || ""; },
        cachePut: function (key, value) { this.cache[key] = value; },
        remainingQuota: function () { return 100; },
        sendEmail: function (to, cc, subject, body) { this.sent.push({ to: to, cc: cc, subject: subject, body: body }); },
        bodyByteLength: function (value) { return unescape(encodeURIComponent(value)).length; },
        utcDay: function () { return "20261004"; }
    };
    services.properties.DEVICE_REGISTRY_JSON = JSON.stringify({ "test-coach": services.registration });
    return services;
}

function discoveryRequest_() {
    return {
        schemaVersion: 1,
        relayToken: "a".repeat(64),
        requestId: "0123456789ABCDEF",
        category: "discovery",
        ownerEmailConsent: true,
        diagnosticEmailConsent: false,
        shareWithSupport: false,
        ownerEmails: ["owner@example.com"],
        coach: { year: 2022, make: "Test", model: "Coach", floorplan: "1", coachId: "test-coach" },
        summary: "",
        report: { devices: [] },
        appleHomeInstructions: ""
    };
}

function testEvent_(request) {
    return { postData: { contents: JSON.stringify(request) } };
}

function assertTest_(condition, message) {
    if (!condition) throw new Error(message);
}