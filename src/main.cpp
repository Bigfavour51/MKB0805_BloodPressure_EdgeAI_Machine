#include <Arduino.h>
#include "datatypes.h"
#include "bp_sensor.h"
#include "oled_driver.h"
#include "buzzer_driver.h"
#include "StateMachine.h"

#include "SD_ReadWrite.h"

#include "wifi_manager.h"
#include "Web_Server.h"


/* =========================================================
 * Global Device Status
 * ========================================================= */

DeviceStatus device;


/* =========================================================
 * SETUP
 * ========================================================= */

void setup()
{
    Serial.begin(115200);

    while (!Serial)
    {
        delay(10);
    }


    Serial.println();
    Serial.println("======================================");
    Serial.println(" NORAHLINKS BP MONITOR");
    Serial.println(" ESP32-S3 Edge AI Prototype");
    Serial.println("======================================");


    /* =====================================================
     * Initial Device State
     * ===================================================== */

    device.state = DeviceState::BOOT;

    device.uptime = millis();

    device.sensorConnected = false;

    device.sdMounted = false;

    device.wifiConnected = false;

    device.mqttConnected = false;

    device.anomalyDetected = false;

    device.measuring = false;

    device.batteryLevel = 100;

    device.alertLevel = AlertLevel::NONE;

    device.simulationMode = SimulationMode::NORMAL;


    /* =====================================================
     * OLED
     * ===================================================== */

    Serial.println("[INIT] OLED...");

    OLED_begin();


    /* =====================================================
     * Buzzer
     * ===================================================== */

    Serial.println("[INIT] Buzzer...");

    Buzzer_begin();


    /* =====================================================
     * Blood Pressure Sensor
     * ===================================================== */

    Serial.println("[INIT] BP Sensor...");

    BP_begin();

    device.sensorConnected =
        BP_isConnected();


    /* =====================================================
     * SD Card
     * ===================================================== */

    Serial.println("[INIT] SD Card...");

    device.sdMounted =
        SD_begin();


    /* =====================================================
     * State Machine
     * ===================================================== */

    Serial.println("[INIT] State Machine...");

    StateMachine_begin(device);


    /* =====================================================
     * Wi-Fi
     * ===================================================== */

    Serial.println("[INIT] Wi-Fi...");

    bool wifiOK =
        WiFiManager_begin();

    device.wifiConnected =
        wifiOK;


    /* =====================================================
     * Web Server
     * ===================================================== */

    Serial.println("[INIT] Web Server...");

    WebServer_begin(device);


    /* =====================================================
     * Simulated BP Reading
     * ===================================================== */

    Serial.println("[TEST] Generating simulated BP reading...");

    BP_setSimulationMode(
        SimulationMode::NORMAL
    );

    // BP_measure(device);


    /* =====================================================
     * OLED Status
     * ===================================================== */

    OLED_update(device);


    /* =====================================================
     * Final State
     * ===================================================== */

    device.state =
        DeviceState::WAITING;


    OLED_update(device);


    /* =====================================================
     * Startup Information
     * ===================================================== */

    Serial.println();
    Serial.println("======================================");
    Serial.println(" SYSTEM READY");
    Serial.println("======================================");

    Serial.print("[WiFi] Mode: ");

    if (WiFiManager_isAPMode())
    {
        Serial.println("ACCESS POINT");
    }
    else
    {
        Serial.println("STATION");
    }


    Serial.print("[WiFi] IP Address: ");

    Serial.println(
        WiFiManager_getIP()
    );


    Serial.println();

    Serial.println(
        "[Web] Open the IP address above"
    );

    Serial.println(
        "[Web] Dashboard: http://<IP>/"
    );

    Serial.println(
        "[Web] API:       http://<IP>/api/status"
    );

    Serial.println();
}


/* =========================================================
 * LOOP
 * ========================================================= */

void loop()
{
    /* =====================================================
     * Wi-Fi
     * ===================================================== */

    WiFiManager_update();


    /*
     * Keep DeviceStatus synchronized with Wi-Fi manager.
     */

    device.wifiConnected =
        WiFiManager_isConnected();


    /* =====================================================
     * Web Server
     * ===================================================== */

    WebServer_update();


    /* =====================================================
     * Device Uptime
     * ===================================================== */

    device.uptime =
        millis();


    /* =====================================================
     * Simulated Measurement Cycle
     * ===================================================== */

    static uint32_t measurementTimer = 0;


    if (millis() - measurementTimer >= 5000)
    {
        measurementTimer =
            millis();


        switch (device.state)
        {

            case DeviceState::WAITING:

                Serial.println();
                Serial.println(
                    "[BP] Starting measurement..."
                );

                device.state =
                    DeviceState::MEASURING;

                break;


            case DeviceState::MEASURING:

                device.measuring = true;

                BP_measure(device);

                device.measuring = false;

                device.state =
                    DeviceState::PROCESSING;

                break;


            case DeviceState::PROCESSING:

                /*
                 * In the future this is where the
                 * Edge AI classification can run.
                 */

                device.state =
                    DeviceState::DISPLAY_RESULT;

                break;


            case DeviceState::DISPLAY_RESULT:

                Serial.println();
                Serial.println(
                    "[BP] Measurement complete"
                );

                Serial.print(
                    "SYS: "
                );

                Serial.println(
                    device.bp.systolic
                );

                Serial.print(
                    "DIA: "
                );

                Serial.println(
                    device.bp.diastolic
                );

                Serial.print(
                    "MAP: "
                );

                Serial.println(
                    device.bp.meanPressure
                );

                Serial.print(
                    "HR: "
                );

                Serial.println(
                    device.bp.pulseRate
                );


                /*
                 * For now, return to waiting.
                 *
                 * Later this can become:
                 *
                 * DISPLAY_RESULT
                 *       ↓
                 * UPLOADING
                 *       ↓
                 * WAITING
                 */

                device.state =
                    DeviceState::WAITING;

                break;


            default:

                device.state =
                    DeviceState::WAITING;

                break;
        }
    }


    /* =====================================================
     * OLED
     * ===================================================== */

    OLED_update(device);


    /*
     * Small delay prevents the loop from running
     * unnecessarily aggressively.
     */

    delay(5);
}
