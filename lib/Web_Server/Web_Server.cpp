// =========================================================
// IMPLEMENTATION (web_server.cpp)
// =========================================================

#include <Arduino.h>
#include <WebServer.h>

#include "datatypes.h"


/* =========================================================
 * Web Server
 * ========================================================= */

static WebServer server(80);

static DeviceStatus *devicePtr = nullptr;

static bool serverRunning = false;


/* =========================================================
 * Simulation Mode
 * ========================================================= */

static const char *getSimulationModeName(SimulationMode mode)
{
    switch (mode)
    {
        case SimulationMode::NORMAL:
            return "NORMAL";

        case SimulationMode::HYPERTENSION:
            return "HYPERTENSION";

        case SimulationMode::HYPOTENSION:
            return "HYPOTENSION";

        case SimulationMode::EXERCISE:
            return "EXERCISE";

        case SimulationMode::RECOVERY:
            return "RECOVERY";

        case SimulationMode::CRITICAL:
            return "CRITICAL";

        case SimulationMode::RANDOM:
            return "RANDOM";

        default:
            return "UNKNOWN";
    }
}


/* =========================================================
 * Alert Level
 * ========================================================= */

static const char *getAlertLevelName(AlertLevel level)
{
    switch (level)
    {
        case AlertLevel::NONE:
            return "NORMAL";

        case AlertLevel::LOW_:
            return "LOW";

        case AlertLevel::MEDIUM:
            return "MEDIUM";

        case AlertLevel::HIGH_:
            return "HIGH";

        case AlertLevel::CRITICAL:
            return "CRITICAL";

        default:
            return "UNKNOWN";
    }
}


/* =========================================================
 * Device State
 * ========================================================= */

static const char *getDeviceStateName(DeviceState state)
{
    switch (state)
    {
        case DeviceState::BOOT:
            return "BOOT";

        case DeviceState::INIT:
            return "INIT";

        case DeviceState::WAITING:
            return "WAITING";

        case DeviceState::MEASURING:
            return "MEASURING";

        case DeviceState::PROCESSING:
            return "PROCESSING";

        case DeviceState::DISPLAY_RESULT:
            return "DISPLAY RESULT";

        case DeviceState::UPLOADING:
            return "UPLOADING";

        case DeviceState::ALERT:
            return "ALERT";

        case DeviceState::ERROR_STATE:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}


/* =========================================================
 * AI Decision Text
 * ========================================================= */

static const char *getDecisionText(const DeviceStatus &device)
{
    if (!device.bp.valid)
    {
        return "No valid blood pressure reading";
    }

    if (device.anomalyDetected)
    {
        switch (device.alertLevel)
        {
            case AlertLevel::LOW_:
                return "Low blood pressure detected";

            case AlertLevel::MEDIUM:
                return "Moderate blood pressure abnormality";

            case AlertLevel::HIGH_:
                return "High blood pressure detected";

            case AlertLevel::CRITICAL:
                return "Critical blood pressure condition";

            default:
                return "Abnormal BP detected";
        }
    }

    return "No abnormal BP detected";
}


/* =========================================================
 * Uptime
 * ========================================================= */

static String formatUptime(uint32_t milliseconds)
{
    uint32_t totalSeconds =
        milliseconds / 1000;

    uint32_t hours =
        totalSeconds / 3600;

    uint32_t minutes =
        (totalSeconds % 3600) / 60;

    uint32_t seconds =
        totalSeconds % 60;

    char buffer[16];

    snprintf(
        buffer,
        sizeof(buffer),
        "%02lu:%02lu:%02lu",
        (unsigned long)hours,
        (unsigned long)minutes,
        (unsigned long)seconds
    );

    return String(buffer);
}


/* =========================================================
 * JSON API
 *
 * GET /api/status
 * ========================================================= */

static void handleStatus()
{
    if (devicePtr == nullptr)
    {
        server.send(
            500,
            "application/json",
            "{\"error\":\"Device status unavailable\"}"
        );

        return;
    }

    const DeviceStatus &device =
        *devicePtr;

    String json;

    json.reserve(700);

    json += "{";

    json += "\"systolic\":";
    json += String(device.bp.systolic, 1);

    json += ",\"diastolic\":";
    json += String(device.bp.diastolic, 1);

    json += ",\"map\":";
    json += String(device.bp.meanPressure, 1);

    json += ",\"heartRate\":";
    json += String(device.bp.pulseRate, 1);

    json += ",\"valid\":";
    json += device.bp.valid ? "true" : "false";

    json += ",\"decision\":\"";
    json += getAlertLevelName(device.alertLevel);
    json += "\"";

    json += ",\"decisionText\":\"";
    json += getDecisionText(device);
    json += "\"";

    // --- NEW EDGE AI METRIC ---
    json += ",\"ai_score\":";
    json += String(device.anomalyScore, 2);
    // --------------------------

    json += ",\"anomalyDetected\":";
    json += device.anomalyDetected
        ? "true"
        : "false";

    json += ",\"sensorConnected\":";
    json += device.sensorConnected
        ? "true"
        : "false";

    json += ",\"sdMounted\":";
    json += device.sdMounted
        ? "true"
        : "false";

    json += ",\"wifiConnected\":";
    json += device.wifiConnected
        ? "true"
        : "false";

    json += ",\"mqttConnected\":";
    json += device.mqttConnected
        ? "true"
        : "false";

    json += ",\"battery\":";
    json += String(device.batteryLevel);

    json += ",\"simulation\":\"";
    json += getSimulationModeName(
        device.simulationMode
    );
    json += "\"";

    json += ",\"state\":\"";
    json += getDeviceStateName(
        device.state
    );
    json += "\"";

    json += ",\"uptime\":\"";
    json += formatUptime(
        device.uptime
    );
    json += "\"";

    json += "}";

    server.send(
        200,
        "application/json",
        json
    );
}


/* =========================================================
 * DASHBOARD HTML
 *
 * IMPORTANT:
 * This is GLOBAL and stored in FLASH.
 * It is NOT allocated on the loopTask stack.
 * ========================================================= */

static const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="en">

<head>

<meta charset="UTF-8">

<meta name="viewport"
      content="width=device-width,
               initial-scale=1.0">

<title>ADARAMOLA BP Monitor</title>

<style>

* {
    box-sizing: border-box;
}

body {
    margin: 0;
    padding: 20px;

    background: #f2f4f7;

    font-family:
        Arial,
        Helvetica,
        sans-serif;

    color: #1f2937;
}

.container {
    max-width: 800px;

    margin: auto;

    background: white;

    border-radius: 14px;

    overflow: hidden;

    box-shadow:
        0 4px 20px
        rgba(0,0,0,0.08);
}

.header {
    background: #111827;

    color: white;

    padding: 24px;

    text-align: center;
}

.header h1 {
    margin: 0;

    font-size: 24px;
}

.header p {
    margin: 7px 0 0;

    font-size: 14px;

    opacity: 0.75;
}

.section {
    padding: 22px;

    border-bottom:
        1px solid #e5e7eb;
}

.section-title {
    font-size: 14px;

    font-weight: bold;

    letter-spacing: 1px;

    color: #6b7280;

    margin-bottom: 16px;
}

.bp-value {
    text-align: center;

    font-size: 46px;

    font-weight: bold;

    margin: 15px 0;

    color: #111827;
}

.bp-unit {
    text-align: center;

    color: #6b7280;

    font-size: 14px;
}

.vitals {
    display: flex;

    justify-content: center;

    gap: 50px;

    text-align: center;

    margin-top: 20px;
}

.vital-value {
    font-size: 22px;

    font-weight: bold;
}

.vital-label {
    font-size: 12px;

    color: #6b7280;

    margin-top: 4px;
}

.decision {
    text-align: center;

    padding: 20px;

    border-radius: 10px;

    background: #f0fdf4;
}

.decision.normal {
    background: #f0fdf4;
}

.decision.alert {
    background: #fff7ed;
}

.decision.critical {
    background: #fef2f2;
}

.decision.warning {
    background: #fffbeb;
}

.decision-value {
    font-size: 26px;

    font-weight: bold;

    margin-bottom: 8px;
}

.decision-text {
    color: #6b7280;

    font-size: 14px;
}

.status-grid {
    display: grid;

    grid-template-columns:
        repeat(2, 1fr);

    gap: 12px;
}

.status-item {
    display: flex;

    justify-content: space-between;

    padding: 12px;

    background: #f9fafb;

    border-radius: 8px;

    font-size: 14px;
}

.status-good {
    color: #16a34a;

    font-weight: bold;
}

.status-bad {
    color: #dc2626;

    font-weight: bold;
}

.footer-info {
    display: flex;

    justify-content: space-between;

    gap: 20px;

    font-size: 13px;

    color: #6b7280;
}

@media(max-width: 600px)
{
    body {
        padding: 8px;
    }

    .bp-value {
        font-size: 38px;
    }

    .vitals {
        gap: 25px;
    }

    .status-grid {
        grid-template-columns: 1fr;
    }

    .footer-info {
        flex-direction: column;
    }
}

</style>

</head>


<body>

<div class="container">


<div class="header">

<h1>ADARAMOLA BP MONITOR</h1>

<p>
ESP32-S3 Edge AI Prototype
</p>

</div>


<div class="section">

<div class="section-title">
BLOOD PRESSURE
</div>

<div id="bp"
     class="bp-value">

-- / -- mmHg

</div>

<div class="bp-unit">
Blood Pressure
</div>


<div class="vitals">

<div>

<div id="map"
     class="vital-value">

-- mmHg

</div>

<div class="vital-label">
MAP
</div>

</div>


<div>

<div id="heartRate"
     class="vital-value">

-- BPM

</div>

<div class="vital-label">
HEART RATE
</div>

</div>

</div>

</div>


<!-- NEW EDGE AI AUTOENCODER SECTION -->
<div class="section">

<div class="section-title">
EDGE AI AUTOENCODER
</div>

<div id="decisionBox"
     class="decision normal">

<div id="decision"
     class="decision-value">

--

</div>

<!-- NEW AI MSE SCORE DISPLAY -->
<div style="font-size: 15px; margin: 12px 0; padding: 8px; background: rgba(0,0,0,0.05); border-radius: 6px;">
Reconstruction Loss (MSE): <strong id="aiScore" style="font-size: 18px;">--</strong>
</div>

<div id="decisionText"
     class="decision-text">

Waiting for measurement...

</div>

</div>

</div>
<!-- END AI SECTION -->


<div class="section">

<div class="section-title">
DEVICE STATUS
</div>

<div class="status-grid">


<div class="status-item">

<span>Sensor</span>

<span id="sensor">--</span>

</div>


<div class="status-item">

<span>SD Card</span>

<span id="sd">--</span>

</div>


<div class="status-item">

<span>Wi-Fi</span>

<span id="wifi">--</span>

</div>


<div class="status-item">

<span>MQTT</span>

<span id="mqtt">--</span>

</div>


<div class="status-item">

<span>Battery</span>

<span id="battery">--</span>

</div>


<div class="status-item">

<span>State</span>

<span id="state">--</span>

</div>

</div>

</div>


<div class="section">

<div class="footer-info">

<span>

Simulation:

<strong id="simulation">
--
</strong>

</span>

<span>

Uptime:

<strong id="uptime">
--
</strong>

</span>

</div>

</div>


</div>


<script>

function setStatus(id, connected)
{
    const element =
        document.getElementById(id);

    if (connected)
    {
        element.innerHTML =
            "● Connected";

        element.className =
            "status-good";
    }
    else
    {
        element.innerHTML =
            "○ Offline";

        element.className =
            "status-bad";
    }
}


async function updateDashboard()
{
    try
    {
        const response =
            await fetch(
                "/api/status",
                {
                    cache: "no-store"
                }
            );

        if (!response.ok)
        {
            throw new Error(
                "HTTP error"
            );
        }

        const data =
            await response.json();


        if (data.valid)
        {
            document.getElementById(
                "bp"
            ).innerText =
                data.systolic.toFixed(0)
                + " / "
                + data.diastolic.toFixed(0)
                + " mmHg";

            document.getElementById(
                "map"
            ).innerText =
                data.map.toFixed(0)
                + " mmHg";

            document.getElementById(
                "heartRate"
            ).innerText =
                data.heartRate.toFixed(0)
                + " BPM";

            // --- NEW: Update AI Score ---
            document.getElementById(
                "aiScore"
            ).innerText =
                data.ai_score.toFixed(2);
        }
        else
        {
            document.getElementById(
                "bp"
            ).innerText =
                "-- / -- mmHg";

            document.getElementById(
                "map"
            ).innerText =
                "-- mmHg";

            document.getElementById(
                "heartRate"
            ).innerText =
                "-- BPM";

            // --- NEW: Clear AI Score ---
            document.getElementById(
                "aiScore"
            ).innerText =
                "--";
        }


        document.getElementById(
            "decision"
        ).innerText =
            data.decision;

        document.getElementById(
            "decisionText"
        ).innerText =
            data.decisionText;


        const decisionBox =
            document.getElementById(
                "decisionBox"
            );


        if (data.decision === "CRITICAL")
        {
            decisionBox.className =
                "decision critical";
        }
        else if (data.anomalyDetected)
        {
            decisionBox.className =
                "decision alert";
        }
        else
        {
            decisionBox.className =
                "decision normal";
        }


        setStatus(
            "sensor",
            data.sensorConnected
        );

        setStatus(
            "sd",
            data.sdMounted
        );

        setStatus(
            "wifi",
            data.wifiConnected
        );

        setStatus(
            "mqtt",
            data.mqttConnected
        );


        document.getElementById(
            "battery"
        ).innerText =
            data.battery + "%";


        document.getElementById(
            "state"
        ).innerText =
            data.state;


        document.getElementById(
            "simulation"
        ).innerText =
            data.simulation;


        document.getElementById(
            "uptime"
        ).innerText =
            data.uptime;

    }
    catch(error)
    {
        console.error(
            "Dashboard update failed:",
            error
        );
    }
}


updateDashboard();

setInterval(
    updateDashboard,
    1000
);

</script>

</body>

</html>

)rawliteral";


/* =========================================================
 * Root Handler
 * ========================================================= */

static void handleRoot()
{
    server.send_P(
        200,
        "text/html",
        DASHBOARD_HTML
    );
}


/* =========================================================
 * 404 Handler
 * ========================================================= */

static void handleNotFound()
{
    String message;

    message += "404 - Not Found\n\n";

    message += "URI: ";
    message += server.uri();

    message += "\nMethod: ";

    message +=
        (server.method() == HTTP_GET)
        ? "GET"
        : "OTHER";

    server.send(
        404,
        "text/plain",
        message
    );
}


/* =========================================================
 * Begin
 * ========================================================= */

bool WebServer_begin(DeviceStatus &device)
{
    devicePtr = &device;

    server.on(
        "/",
        HTTP_GET,
        handleRoot
    );

    server.on(
        "/api/status",
        HTTP_GET,
        handleStatus
    );

    server.onNotFound(
        handleNotFound
    );

    server.begin();

    serverRunning = true;

    Serial.println();
    Serial.println("======================================");
    Serial.println(" Web Server");
    Serial.println("======================================");

    Serial.println("[Web] Server started");
    Serial.println("[Web] Port: 80");

    return true;
}


/* =========================================================
 * Update
 * ========================================================= */

void WebServer_update()
{
    if (!serverRunning)
        return;

    server.handleClient();
}


/* =========================================================
 * Status
 * ========================================================= */

bool WebServer_isRunning()
{
    return serverRunning;
}