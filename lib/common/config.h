#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace Config
{
    /**************************************************
     * OLED Display
     **************************************************/
    constexpr uint8_t OLED_SDA  = 8;
    constexpr uint8_t OLED_SCL  = 9;
    constexpr uint8_t OLED_ADDR = 0x3C;

    /**************************************************
     * SD Card (ESP32-S3 Freenove)
     **************************************************/
    constexpr uint8_t SD_CLK = 39;
    constexpr uint8_t SD_CMD = 38;
    constexpr uint8_t SD_D0  = 40;

    /**************************************************
     * Buzzer
     **************************************************/
    constexpr uint8_t BUZZER_PIN = 5;      // Change later

    /**************************************************
     * Blood Pressure Sensor
     **************************************************/
    constexpr uint8_t BP_SENSOR_PIN = 0;   // Placeholder

    /**************************************************
     * Timing
     **************************************************/
    constexpr uint32_t SAMPLE_PERIOD_MS = 1000;
    constexpr uint32_t OLED_REFRESH_MS  = 250;

    /**************************************************
     * Device
     **************************************************/
    constexpr char DEVICE_NAME[] = "Norahlinks BP Monitor";

    /**************************************************
     * Wi-Fi credentials 
     **************************************************/

     constexpr char WIFI_SSID[] = "Norahlinks"; // Replace with your Wi-Fi SSID
     constexpr char WIFI_PASSWORD[] = "cherish090"; // Replace with your Wi-Fi password

}

#endif