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

bool WiFiManager_begin();
void WiFiManager_update();
bool WiFiManager_isConnected();
IPAddress WiFiManager_getIP();
bool WiFiManager_isAPMode();
WiFiMode WiFiManager_getMode();

#endif