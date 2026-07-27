#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__


#include <Arduino.h>
#include <WiFi.h>
#include <IPAddress.h>

enum class WiFiMode
{
    STATION,
    ACCESS_POINT
};

/*
 * Initialize Wi-Fi.
 *
 * Attempts to connect to the configured
 * Wi-Fi network. If unsuccessful, starts
 * Access Point mode.
 */
bool WiFiManager_begin();

/*
 * Keep the Wi-Fi connection alive.
 * Call this repeatedly from loop().
 */
void WiFiManager_update();

/*
 * Connection status.
 */
bool WiFiManager_isConnected();

/*
 * Returns the current IP address.
 */
IPAddress WiFiManager_getIP();

/*
 * Returns true if the ESP32 is currently
 * operating as an Access Point.
 */
bool WiFiManager_isAPMode();

/*
 * Returns the current Wi-Fi mode.
 */
WiFiMode WiFiManager_getMode();



#endif // __WIFI_MANAGER_H__