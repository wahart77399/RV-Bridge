# Gmail Relay

This Google Apps Script web app sends SmartCoach reports through the Gmail account that authorizes the script. Gmail credentials never go on the ESP32. The web app requires a per-coach bearer token in the JSON body, checks recipients against a per-coach allowlist, requires the category-specific consent bit, honors support sharing separately, limits a coach to three send attempts per UTC day, and deduplicates successful request IDs for six hours.

Apps Script web apps return JSON bodies but do not let `doPost` select an HTTP status code. Clients must inspect the response's `ok` and `code` fields. A response with `code: "delivery_uncertain"` must not be retried automatically because Gmail may already have accepted the message.

## Current Beta Deployment

Apps Script web app URL: `https://script.google.com/macros/s/AKfycbybuF3VJumdiLCmmu7gZgZvpZQ8uzfkqjxzM2ixF9drJ8Zk7UGCXgdsC7jP20nxdnC5LQ/exec`

SmartCoach email settings page: `http://<current-bridge-ip>:8080/email`

## Setup

1. Create a standalone project at [script.google.com](https://script.google.com) and add `Code.gs` and `RelayTests.gs` from this folder.
2. Run `runRelayTests` from the Apps Script editor. It should return `10/10 relay tests passed` without sending email.
3. Generate a unique 32-byte token for each coach and encode it as exactly 64 hexadecimal characters. Keep the raw token for the device configuration. Temporarily set the script property `TOKEN_TO_HASH` to that token, run `hashTokenForRegistry`, and copy the logged SHA-256 digest. The helper deletes `TOKEN_TO_HASH` after hashing.
4. Set the script property `DEVICE_REGISTRY_JSON` to an object keyed by the exact `coachId`. Each entry has `tokenSha256`, `ownerEmails` (the recipient allowlist), and `supportAllowed` (normally `false`). Example:

   ```json
   {"coach-123":{"tokenSha256":"64-character-sha256-hex","ownerEmails":["owner@example.com"],"supportAllowed":false}}
   ```

5. Set `SUPPORT_EMAIL` only if support delivery is used. Both the device's `shareWithSupport` consent and the registration's `supportAllowed: true` are required for a support copy.
6. Deploy as a web app that executes as your Google account and is accessible to anyone. The URL is public, but requests without a registered coach token are rejected. Keep the token out of source control, screenshots, and chat.

## Request Contract

The POST body is the staged report envelope plus a top-level `relayToken`. Discovery messages require `ownerEmailConsent: true`; diagnostics require `diagnosticEmailConsent: true`. The relay only sends to registered owner addresses. It never places the relay token in the email body.

Email routes and transport are included in normal `Release`; `EmailBeta` remains as a compatible alias. Sending still requires owner consent, registered owner addresses, and a per-coach relay token. Opted-in learning completion stages and attempts the discovery report once. A failed send stays in the outbox. The portal can edit the owner recipient allowlist, stage a completed learning report or, if none exists, an explicitly labeled snapshot of the current `devices.json`, and manually retry a pending report. Relay URL and token are configured in the portal; the token is stored in NVS, never returned by the device, and is not included in `coach.json` or the email body.

Build the standard firmware with `platformio run --environment Release`. Use `platformio run --environment DiagnosticsBeta` to additionally enable diagnostic collection and the manual diagnostics download. The relay has completed a live owner-only beta send; additional coaches still need distinct registry entries and relay tokens.