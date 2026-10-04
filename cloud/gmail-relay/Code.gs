var RELAY_MAX_BODY_BYTES = 32768;
var RELAY_MAX_DAILY_ATTEMPTS = 3;
var RELAY_DEDUPE_SECONDS = 21600;

function doPost(event) {
    var result;
    try {
        result = processRelayRequest_(event, createProductionServices_());
    } catch (error) {
        result = { ok: false, code: "internal_error", retryable: true };
    }
    var output = ContentService.createTextOutput(JSON.stringify(result));
    output.setMimeType(ContentService.MimeType.JSON);
    return output;
}

function hashTokenForRegistry() {
    var properties = PropertiesService.getScriptProperties();
    var token = properties.getProperty("TOKEN_TO_HASH") || "";
    var hash = "";
    if (/^[0-9a-fA-F]{64}$/.test(token)) {
        hash = sha256Hex_(token);
        properties.deleteProperty("TOKEN_TO_HASH");
        Logger.log(hash);
    }
    return hash;
}

function processRelayRequest_(event, services) {
    var result = { ok: false, code: "invalid_request", retryable: false };
    var locked = false;
    do {
        var raw = event && event.postData && event.postData.contents;
        if (typeof raw !== "string" || raw.length === 0) break;
        if (services.bodyByteLength(raw) > RELAY_MAX_BODY_BYTES) {
            result.code = "body_too_large";
            break;
        }
        var request;
        try {
            request = JSON.parse(raw);
        } catch (error) {
            result.code = "invalid_json";
            break;
        }
        if (!isRecord_(request) || request.schemaVersion !== 1 ||
            typeof request.relayToken !== "string" || !/^[0-9a-fA-F]{64}$/.test(request.relayToken) ||
            !isRecord_(request.coach)) break;

        var coachId = request.coach.coachId;
        if (typeof coachId !== "string" || coachId.length === 0 || coachId.length > 64) break;
        var registry;
        try {
            registry = JSON.parse(services.getProperty("DEVICE_REGISTRY_JSON") || "{}");
        } catch (error) {
            result.code = "relay_not_configured";
            break;
        }
        var device = isRecord_(registry) ? registry[coachId] : null;
        if (!isRecord_(device) || typeof device.tokenSha256 !== "string" ||
            !constantTimeEqual_(services.hashToken(request.relayToken), device.tokenSha256)) {
            result.code = "unauthorized_device";
            break;
        }

        var category = request.category;
        var requestId = request.requestId;
        if ((category !== "discovery" && category !== "diagnostics") ||
            typeof requestId !== "string" || !/^[0-9a-fA-F]{16}$/.test(requestId) ||
            !Array.isArray(request.ownerEmails) || request.ownerEmails.length === 0 ||
            request.ownerEmails.length > 5 || typeof request.shareWithSupport !== "boolean") break;
        if (category === "discovery" && request.ownerEmailConsent !== true) {
            result.code = "owner_consent_required";
            break;
        }
        if (category === "diagnostics" && (request.diagnosticEmailConsent !== true ||
            typeof request.summary !== "string" || request.summary.length === 0 || request.summary.length > 1024)) {
            result.code = "diagnostic_consent_required";
            break;
        }

        var allowedOwners = Array.isArray(device.ownerEmails) ? device.ownerEmails : [];
        var recipients = [];
        var recipientValid = true;
        for (var index = 0; index < request.ownerEmails.length; index++) {
            var address = request.ownerEmails[index];
            if (!isValidEmail_(address) || !containsEmail_(allowedOwners, address) ||
                containsEmail_(recipients, address)) {
                recipientValid = false;
                break;
            }
            recipients.push(address.toLowerCase());
        }
        if (!recipientValid) {
            result.code = "recipient_not_allowed";
            break;
        }

        var supportEmail = "";
        if (request.shareWithSupport) {
            supportEmail = services.getProperty("SUPPORT_EMAIL") || "";
            if (device.supportAllowed !== true || !isValidEmail_(supportEmail)) {
                result.code = "support_sharing_not_configured";
                break;
            }
        }
        if (category === "discovery" && !isRecord_(request.report)) break;
        if (request.report !== undefined && !isRecord_(request.report)) break;

        var deviceKey = services.hashToken(coachId).substring(0, 16);
        var cacheKey = "sent_" + deviceKey + "_" + requestId.toLowerCase();
        if (!services.tryLock()) {
            result.code = "relay_busy";
            result.retryable = true;
            break;
        }
        locked = true;
        var previous = services.cacheGet(cacheKey);
        if (previous === "sent") {
            result = { ok: true, code: "already_sent", duplicate: true, retryable: false };
            break;
        }
        if (previous === "pending") {
            result.code = "delivery_uncertain";
            break;
        }

        var day = services.utcDay();
        var dailyKey = "attempts_" + deviceKey;
        var dailyState;
        try {
            dailyState = JSON.parse(services.getProperty(dailyKey) || "{}");
        } catch (error) {
            result.code = "relay_storage_error";
            result.retryable = true;
            break;
        }
        var dailyAttempts = dailyState.day === day ? Number(dailyState.count) : 0;
        if (!Number.isInteger(dailyAttempts) || dailyAttempts >= RELAY_MAX_DAILY_ATTEMPTS) {
            result.code = "daily_limit_reached";
            break;
        }
        var cc = supportEmail ? supportEmail : "";
        var recipientCount = recipients.length + (cc ? 1 : 0);
        if (services.remainingQuota() < recipientCount) {
            result.code = "gmail_quota_exhausted";
            result.retryable = true;
            break;
        }

        var subject = makeSubject_(category, request.coach, request.reportKind);
        var body = makeBody_(request);
        services.cachePut(cacheKey, "pending", RELAY_DEDUPE_SECONDS);
        services.setProperty(dailyKey, JSON.stringify({ day: day, count: dailyAttempts + 1 }));
        try {
            services.sendEmail(recipients.join(","), cc, subject, body);
        } catch (error) {
            result.code = "delivery_uncertain";
            break;
        }
        services.cachePut(cacheKey, "sent", RELAY_DEDUPE_SECONDS);
        result = { ok: true, code: "accepted", requestId: requestId, retryable: false };
    } while (false);
    if (locked) services.releaseLock();
    return result;
}

function createProductionServices_() {
    var properties = PropertiesService.getScriptProperties();
    var cache = CacheService.getScriptCache();
    var lock = LockService.getScriptLock();
    return {
        getProperty: function (key) { return properties.getProperty(key); },
        setProperty: function (key, value) { properties.setProperty(key, value); },
        hashToken: sha256Hex_,
        tryLock: function () { return lock.tryLock(5000); },
        releaseLock: function () { lock.releaseLock(); },
        cacheGet: function (key) { return cache.get(key); },
        cachePut: function (key, value, seconds) { cache.put(key, value, seconds); },
        remainingQuota: function () { return MailApp.getRemainingDailyQuota(); },
        bodyByteLength: function (value) { return Utilities.newBlob(value).getBytes().length; },
        sendEmail: function (to, cc, subject, body) {
            var message = { to: to, subject: subject, body: body };
            if (cc) message.cc = cc;
            MailApp.sendEmail(message);
        },
        utcDay: function () { return Utilities.formatDate(new Date(), "UTC", "yyyyMMdd"); }
    };
}

function sha256Hex_(value) {
    var bytes = Utilities.computeDigest(Utilities.DigestAlgorithm.SHA_256, value, Utilities.Charset.UTF_8);
    var hex = "";
    for (var index = 0; index < bytes.length; index++) {
        var byte = (bytes[index] + 256) % 256;
        hex += (byte < 16 ? "0" : "") + byte.toString(16);
    }
    return hex;
}

function constantTimeEqual_(left, right) {
    var equal = typeof left === "string" && typeof right === "string" && left.length === right.length;
    var difference = 0;
    if (typeof left === "string" && typeof right === "string") {
        var length = Math.max(left.length, right.length);
        for (var index = 0; index < length; index++) {
            difference |= (left.charCodeAt(index) || 0) ^ (right.charCodeAt(index) || 0);
        }
    } else {
        difference = 1;
    }
    return equal && difference === 0;
}

function isValidEmail_(value) {
    var valid = typeof value === "string" && value.length <= 254 &&
        /^[^\s@,;<>]+@[^\s@,;<>]+\.[^\s@,;<>]+$/.test(value);
    return valid;
}

function containsEmail_(addresses, candidate) {
    var found = false;
    var normalized = typeof candidate === "string" ? candidate.toLowerCase() : "";
    for (var index = 0; index < addresses.length; index++) {
        if (typeof addresses[index] === "string" && addresses[index].toLowerCase() === normalized) found = true;
    }
    return found;
}

function isRecord_(value) {
    return value !== null && typeof value === "object" && !Array.isArray(value);
}

function makeSubject_(category, coach, reportKind) {
    var make = typeof coach.make === "string" ? coach.make : "SmartCoach";
    var model = typeof coach.model === "string" ? coach.model : "report";
    var reportLabel = category === "discovery" && reportKind === "currentInventory" ? "current inventory" : category;
    var subject = ("SmartCoach " + reportLabel + " report: " + make + " " + model)
        .replace(/[\r\n\u0000-\u001f\u007f]/g, " ").substring(0, 180);
    return subject;
}

function makeBody_(request) {
    var reportTitle = request.reportKind === "currentInventory" ? "SmartCoach current inventory snapshot" : "SmartCoach " + request.category + " report";
    var sections = [reportTitle, "Request ID: " + request.requestId];
    if (request.reportKind === "currentInventory") sections.push("This snapshot uses the existing device configuration; it is not a new learning run.");
    sections.push("Coach: " + [request.coach.year, request.coach.make, request.coach.model, request.coach.floorplan].join(" "));
    if (request.summary) sections.push("Summary:\n" + request.summary);
    if (request.report) sections.push("Report data:\n" + JSON.stringify(request.report, null, 2));
    if (request.appleHomeInstructions) sections.push(String(request.appleHomeInstructions).substring(0, 2048));
    return sections.join("\n\n").substring(0, RELAY_MAX_BODY_BYTES);
}