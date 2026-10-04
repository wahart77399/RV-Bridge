#pragma once

inline constexpr char SMARTCOACH_DIAGNOSTICS_PAGE[] = R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SmartCoach Diagnostics</title>
  <style>
    :root { color-scheme: light; --ink: #18392f; --muted: #61756a; --line: #d1dbcf; --panel: #fff; --canvas: #edf2e9; --accent: #315e4b; --warn: #9c4639; }
    * { box-sizing: border-box; }
    body { margin: 0; background: var(--canvas); color: var(--ink); font: 15px system-ui, sans-serif; }
    main { max-width: 1100px; margin: 28px auto; padding: 0 18px 36px; }
    header { display: flex; flex-wrap: wrap; align-items: end; justify-content: space-between; gap: 14px; margin-bottom: 18px; }
    h1 { margin: 0; font-size: 24px; }
    .subhead { margin: 6px 0 0; color: var(--muted); font-size: 13px; }
    .actions { display: flex; flex-wrap: wrap; gap: 8px; }
    button, .download { display: inline-flex; align-items: center; justify-content: center; min-height: 40px; padding: 8px 12px; border: 1px solid var(--accent); border-radius: 4px; background: var(--accent); color: #fff; font: inherit; text-decoration: none; cursor: pointer; }
    button:disabled { opacity: .6; cursor: progress; }
    .download { background: transparent; color: var(--accent); }
    #state { min-height: 22px; margin: 8px 0 14px; color: var(--muted); }
    #state[data-error="true"] { color: var(--warn); }
    .metrics { display: grid; grid-template-columns: repeat(auto-fit, minmax(145px, 1fr)); gap: 10px; margin: 0 0 24px; }
    .metric { min-width: 0; padding: 14px; border: 1px solid var(--line); border-radius: 4px; background: var(--panel); }
    .metric span { display: block; color: var(--muted); font-size: 11px; text-transform: uppercase; }
    .metric strong { display: block; margin-top: 5px; font-size: 18px; overflow-wrap: anywhere; }
    section { margin-top: 22px; }
    h2 { margin: 0 0 10px; font-size: 17px; }
    .table-wrap { overflow-x: auto; border: 1px solid var(--line); background: var(--panel); }
    table { width: 100%; min-width: 760px; border-collapse: collapse; font-size: 12px; }
    th, td { padding: 10px 9px; border-bottom: 1px solid var(--line); text-align: left; vertical-align: top; overflow-wrap: anywhere; }
    th { color: var(--muted); font-weight: 600; }
    .note { max-width: 800px; color: var(--muted); font-size: 12px; line-height: 1.55; }
    @media (max-width: 600px) { main { margin-top: 18px; } header { align-items: start; flex-direction: column; } }
  </style>
</head>
<body>
  <main>
    <header>
      <div><h1>SmartCoach Diagnostics</h1><p class="subhead">Passive bridge and RV-C status snapshot</p></div>
      <div class="actions"><button id="refresh" type="button">Refresh snapshot</button><a class="download" href="/diagnostics">Download JSON</a><a class="download" href="/">Back to portal</a></div>
    </header>
    <p id="state" role="status" aria-live="polite">Loading diagnostics</p>
    <section aria-label="System and bus summary"><div class="metrics" id="metrics"></div></section>
    <section aria-labelledby="device-heading">
      <h2 id="device-heading">Configured devices</h2>
      <div class="table-wrap"><table><thead><tr><th>Name</th><th>Type</th><th>Index</th><th>Source</th><th>Observed</th><th>Last DGN</th><th>Last frame age</th><th>Handled / received</th></tr></thead><tbody id="devices"></tbody></table></div>
    </section>
    <section aria-labelledby="unmapped-heading">
      <h2 id="unmapped-heading">Unmapped CAN traffic</h2>
      <div class="table-wrap"><table><thead><tr><th>DGN</th><th>Family</th><th>Source</th><th>Index</th><th>Hits</th><th>Last frame age</th><th>Payload</th><th>Review status</th></tr></thead><tbody id="unmapped"></tbody></table></div>
    </section>
    <p id="meaning" class="note"></p>
  </main>
  <script>
    const state = document.querySelector("#state");
    const metrics = document.querySelector("#metrics");
    const devices = document.querySelector("#devices");
    const unmapped = document.querySelector("#unmapped");
    const meaning = document.querySelector("#meaning");
    function metric(label, value) {
      const item = document.createElement("div");
      item.className = "metric";
      const title = document.createElement("span");
      const amount = document.createElement("strong");
      title.textContent = label;
      amount.textContent = value == null ? "Unavailable" : String(value);
      item.append(title, amount);
      metrics.append(item);
    }
    function addCell(row, value) {
      const cell = document.createElement("td");
      cell.textContent = value == null ? "Not observed" : String(value);
      row.append(cell);
    }
    function unmappedDgnLabel(activity) {
      const formatDgn = (value) => "0x" + Number(value).toString(16).toUpperCase().padStart(5, "0");
      const dgnName = activity.dgnName || "UNKNOWN_DGN";
      if (!activity.pgnName) return dgnName + " (DGN " + formatDgn(activity.dgn) + ")";
      const destination = activity.destinationAddress == null ? "" : ", DA " + formatDgn(activity.destinationAddress);
      return activity.pgnName + " (" + activity.protocol + " PGN " + formatDgn(activity.pgn) + destination + "; ID " + formatDgn(activity.dgn) + ")";
    }
    async function refresh() {
      const button = document.querySelector("#refresh");
      button.disabled = true;
      state.dataset.error = "false";
      state.textContent = "Loading diagnostics snapshot";
      try {
        const response = await fetch("/diagnostics", { cache: "no-store" });
        if (!response.ok) throw new Error("Diagnostics unavailable (HTTP " + response.status + ")");
        const report = await response.json();
        const bus = report.bus || {};
        metrics.replaceChildren();
        devices.replaceChildren();
        unmapped.replaceChildren();
        metric("Uptime", report.uptimeMs == null ? null : report.uptimeMs + " ms");
        metric("Wi-Fi", report.wifiConnected ? "Connected" : "Disconnected");
        metric("RSSI", report.wifiRssiDbm == null ? null : report.wifiRssiDbm + " dBm");
        metric("Free heap", report.freeHeapBytes == null ? null : report.freeHeapBytes + " bytes");
        metric("Minimum heap", report.minimumFreeHeapBytes == null ? null : report.minimumFreeHeapBytes + " bytes");
        metric("Received frames", bus.receivedFrames);
        metric("Bus off", bus.busOff == null ? (bus.driverStatusAvailable ? "No" : "Unavailable") : (bus.busOff ? "Yes" : "No"));
        metric("Bus errors", bus.busErrorCount);
        metric("Unmapped overflow", bus.unmappedOverflowHits || 0);
        for (const device of report.configuredDevices || []) {
          const row = document.createElement("tr");
          addCell(row, device.name);
          addCell(row, device.type);
          addCell(row, device.rvcIndex);
          addCell(row, device.sourceAddress);
          addCell(row, device.observed ? "Yes" : "No");
          addCell(row, device.lastDgnName);
          addCell(row, device.lastFrameAgeMs == null ? null : device.lastFrameAgeMs + " ms");
          addCell(row, device.handledFrames + " / " + device.receivedFrames);
          devices.append(row);
        }
        const unmappedRows = report.unmappedTraffic || [];
        for (const activity of unmappedRows) {
          const row = document.createElement("tr");
          addCell(row, unmappedDgnLabel(activity));
          addCell(row, activity.family === "Unknown" ? activity.protocol : activity.family);
          addCell(row, activity.sourceAddress);
          addCell(row, activity.rvcIndex + " (" + (activity.instanceVerified ? "verified" : "candidate") + ")");
          addCell(row, activity.hits);
          addCell(row, activity.lastFrameAgeMs == null ? null : activity.lastFrameAgeMs + " ms");
          addCell(row, activity.payloadHex);
          addCell(row, (activity.inReview ? "In review" : "Not in review") + (activity.payloadAllFf ? " - all 0xFF" : ""));
          unmapped.append(row);
        }
        if (unmappedRows.length === 0) {
          const row = document.createElement("tr");
          const cell = document.createElement("td");
          cell.colSpan = 8;
          cell.textContent = "No unmapped CAN traffic observed";
          row.append(cell);
          unmapped.append(row);
        }
        meaning.textContent = [report.handledFramesMeaning, report.unmappedTrafficMeaning, report.payloadAllFfMeaning].filter(Boolean).join(" ");
        state.textContent = "Snapshot updated " + new Date().toLocaleTimeString();
      } catch (error) {
        state.dataset.error = "true";
        state.textContent = error.message;
      } finally {
        button.disabled = false;
      }
    }
    document.querySelector("#refresh").addEventListener("click", refresh);
    refresh();
  </script>
</body>
</html>)HTML";
