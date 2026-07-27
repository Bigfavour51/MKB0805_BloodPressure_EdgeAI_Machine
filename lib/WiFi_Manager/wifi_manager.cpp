#include "wifi_manager.h"

/* =========================================================
 * Configuration
 * ========================================================= */

// Your normal Wi-Fi network
static const char *WIFI_SSID = "Norahlinks";
static const char *WIFI_PASSWORD = "cherish090";

// ESP32 fallback Access Point
static const char *AP_SSID = "Norahlinks-BP";
static const char *AP_PASSWORD = "BPMonitor123";

// How long we try Station mode before falling back to AP
static constexpr uint32_t WIFI_CONNECT_TIMEOUT = 15000;

// How often we retry a lost connection
static constexpr uint32_t WIFI_RECONNECT_INTERVAL = 10000;


/* =========================================================
 * Internal state
 * ========================================================= */

static bool wifiConnected = false;

static WiFiMode currentMode = WiFiMode::STATION;

static uint32_t connectionStartTime = 0;
static uint32_t lastReconnectAttempt = 0;


/* =========================================================
 * Start Access Point
 * ========================================================= */

static bool startAccessPoint()
{
    Serial.println();
    Serial.println("[WiFi] Starting Access Point...");

    WiFi.mode(WIFI_AP);

    bool success = WiFi.softAP(AP_SSID, AP_PASSWORD);

    if (!success)
    {
        Serial.println("[WiFi] AP start FAILED");
        wifiConnected = false;
        return false;
    }

    currentMode = WiFiMode::ACCESS_POINT;
    wifiConnected = true;

    Serial.println("[WiFi] Access Point started");
    Serial.print("[WiFi] SSID: ");
    Serial.println(AP_SSID);

    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.softAPIP());

    return true;
}


/* =========================================================
 * Begin Wi-Fi
 * ========================================================= */

bool WiFiManager_begin()
{
    Serial.println();
    Serial.println("======================================");
    Serial.println(" Wi-Fi Manager");
    Serial.println("======================================");

    wifiConnected = false;

    WiFi.mode(WIFI_STA);

    Serial.print("[WiFi] Connecting to: ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    connectionStartTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - connectionStartTime < WIFI_CONNECT_TIMEOUT)
    {
        delay(500);

        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        currentMode = WiFiMode::STATION;
        wifiConnected = true;

        Serial.println("[WiFi] Connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());

        Serial.print("[WiFi] RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");

        return true;
    }

    Serial.println("[WiFi] Station connection failed.");

    return startAccessPoint();
}


/* =========================================================
 * Wi-Fi Update
 * ========================================================= */

void WiFiManager_update()
{
    /*
     * AP mode doesn't require reconnection.
     */
    if (currentMode == WiFiMode::ACCESS_POINT)
    {
        wifiConnected = true;
        return;
    }

    /*
     * Station mode
     */

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        return;
    }

    /*
     * Connection has been lost.
     */
    if (wifiConnected)
    {
        Serial.println("[WiFi] Connection lost.");

        wifiConnected = false;
    }

    /*
     * Don't continuously hammer Wi-Fi.begin().
     */
    if (millis() - lastReconnectAttempt >= WIFI_RECONNECT_INTERVAL)
    {
        lastReconnectAttempt = millis();

        Serial.println("[WiFi] Attempting reconnection...");

        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
}


/* =========================================================
 * Status
 * ========================================================= */

bool WiFiManager_isConnected()
{
    return wifiConnected;
}


/* =========================================================
 * IP Address
 * ========================================================= */

IPAddress WiFiManager_getIP()
{
    if (currentMode == WiFiMode::ACCESS_POINT)
    {
        return WiFi.softAPIP();
    }

    return WiFi.localIP();
}


/* =========================================================
 * AP Mode
 * ========================================================= */

bool WiFiManager_isAPMode()
{
    return currentMode == WiFiMode::ACCESS_POINT;
}


/* =========================================================
 * Current Mode
 * ========================================================= */

WiFiMode WiFiManager_getMode()
{
    return currentMode;
}