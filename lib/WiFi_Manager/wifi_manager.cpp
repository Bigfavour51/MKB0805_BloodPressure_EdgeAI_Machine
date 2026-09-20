#include "wifi_manager.h"

/* =========================================================
 * ESP32 Access Point Configuration
 * ========================================================= */

static const char *AP_SSID = "Adaramola-BP";
static const char *AP_PASSWORD = "BPMonitor123";

/*
 * Fixed IP for the ESP32 itself.
 *
 * Client connects to:
 *
 *     Adaramola-BP
 *
 * Then opens:
 *
 *     http://192.168.4.1
 */

static IPAddress AP_IP(192, 168, 4, 1);
static IPAddress AP_GATEWAY(192, 168, 4, 1);
static IPAddress AP_SUBNET(255, 255, 255, 0);

/* =========================================================
 * State
 * ========================================================= */

static bool wifiConnected = false;
static WiFiMode currentMode = WiFiMode::ACCESS_POINT;


/* =========================================================
 * Start Access Point
 * ========================================================= */

static bool startAccessPoint()
{
    Serial.println();
    Serial.println("======================================");
    Serial.println(" ESP32 ACCESS POINT");
    Serial.println("======================================");

    /*
     * Configure ESP32 as Wi-Fi Access Point
     */
    WiFi.mode(WIFI_AP);

    /*
     * Configure fixed IP
     */
    if (!WiFi.softAPConfig(
            AP_IP,
            AP_GATEWAY,
            AP_SUBNET))
    {
        Serial.println("[WiFi] AP IP configuration FAILED");
        wifiConnected = false;
        return false;
    }

    /*
     * Start Access Point
     */
    bool success = WiFi.softAP(
        AP_SSID,
        AP_PASSWORD);

    if (!success)
    {
        Serial.println("[WiFi] AP start FAILED");
        wifiConnected = false;
        return false;
    }

    currentMode = WiFiMode::ACCESS_POINT;
    wifiConnected = true;

    Serial.println("[WiFi] Access Point started successfully");

    Serial.print("[WiFi] SSID: ");
    Serial.println(AP_SSID);

    Serial.print("[WiFi] Password: ");
    Serial.println(AP_PASSWORD);

    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.softAPIP());

    Serial.println();
    Serial.println("[WiFi] Client connection:");
    Serial.println("1. Connect to Wi-Fi: Adaramola-BP");
    Serial.println("2. Open browser");
    Serial.println("3. Go to: http://192.168.4.1");

    return true;
}


/* =========================================================
 * Begin Wi-Fi
 * ========================================================= */

bool WiFiManager_begin()
{
    /*
     * This device is intentionally AP-only.
     *
     * No router.
     * No client Wi-Fi.
     * No DHCP reservation.
     * No external network required.
     */

    return startAccessPoint();
}


/* =========================================================
 * Update
 * ========================================================= */

void WiFiManager_update()
{
    /*
     * Nothing needs to be done here for AP mode.
     *
     * The ESP32 continuously hosts its own network.
     */
    if (currentMode == WiFiMode::ACCESS_POINT)
    {
        wifiConnected = true;
    }
}


/* =========================================================
 * Connection Status
 * ========================================================= */

bool WiFiManager_isConnected()
{
    return wifiConnected;
}


/* =========================================================
 * Get IP Address
 * ========================================================= */

IPAddress WiFiManager_getIP()
{
    return WiFi.softAPIP();
}


/* =========================================================
 * Check AP Mode
 * ========================================================= */

bool WiFiManager_isAPMode()
{
    return currentMode == WiFiMode::ACCESS_POINT;
}


/* =========================================================
 * Get Wi-Fi Mode
 * ========================================================= */

WiFiMode WiFiManager_getMode()
{
    return currentMode;
}