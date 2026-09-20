#include <Arduino.h>

#include "datatypes.h"
#include "bp_sensor.h"
#include "ai_anomaly.h"

#include "oled_driver.h"
#include "buzzer_driver.h"
#include "StateMachine.h"
#include "SD_ReadWrite.h"
#include "wifi_manager.h"
#include "Web_Server.h"


/* =========================================================
 * MKB0805 UART Configuration
 * ========================================================= */

#define RXD2 18
#define TXD2 17

#define PACKET_SIZE 19
#define HEADER_BYTE 0x05

HardwareSerial MKBSerial(2);

const uint8_t CMD_F7[] =
{
    0xF7,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

uint8_t rawBuffer[PACKET_SIZE];
size_t bufferIndex = 0;

unsigned long lastCommandTime = 0;


/* =========================================================
 * Finger / Calibration State
 * ========================================================= */

unsigned long fingerContactStartTime = 0;

const unsigned long STABLE_CONTACT_DELAY_MS = 1000;

unsigned long calibStartTime = 0;

float calibMin = 65535.0f;
float calibMax = 0.0f;


/* =========================================================
 * Device State
 * ========================================================= */

DeviceStatus device;


/* =========================================================
 * BP Simulation Timing
 * ========================================================= */

unsigned long lastBPMeasurement = 0;

const unsigned long BP_MEASUREMENT_INTERVAL = 1000;


/* =========================================================
 * Edge AI Configuration
 * ========================================================= */

/*
 * Current autoencoder returns:
 *
 *     MSE x 100
 *
 * Approximate prototype values:
 *
 * NORMAL      ~5
 * HYPERTENSION >10
 * CRITICAL    >15
 *
 * These are prototype thresholds and can be
 * tuned later using real validation data.
 */

static constexpr float AI_WARNING_THRESHOLD = 10.0f;
static constexpr float AI_HIGH_THRESHOLD    = 13.0f;
static constexpr float AI_CRITICAL_THRESHOLD = 16.0f;


/*
 * Temporary SpO2 value.
 *
 * MAX30100 is currently not being used because
 * the sensor is not responding on I2C.
 *
 * Replace this with the actual SpO2 value once
 * MAX30100 integration is restored.
 */

static constexpr float SIMULATED_SPO2 = 98.0f;


/* =========================================================
 * Process Edge AI
 * ========================================================= */

static void processEdgeAI()
{
    if (!device.bp.valid)
    {
        device.anomalyScore = 0.0f;
        device.anomalyDetected = false;
        device.alertLevel = AlertLevel::NONE;

        return;
    }


    /*
     * Keep DeviceStatus heartRate synchronized
     * with the BP reading.
     */
    device.heartRate =
        device.bp.pulseRate;


    /*
     * Temporary SpO2 source.
     *
     * Later this should come from MAX30100.
     */
    device.spo2 =
        SIMULATED_SPO2;


    /*
     * Run the autoencoder.
     */
    device.anomalyScore =
        EdgeAI_GetAnomalyScore(
            device.bp.systolic,
            device.bp.diastolic,
            device.bp.pulseRate,
            device.spo2
        );


    /*
     * Determine anomaly level.
     */

    if (device.anomalyScore >= AI_CRITICAL_THRESHOLD)
    {
        device.anomalyDetected = true;
        device.alertLevel = AlertLevel::CRITICAL;
    }
    else if (device.anomalyScore >= AI_HIGH_THRESHOLD)
    {
        device.anomalyDetected = true;
        device.alertLevel = AlertLevel::HIGH_;
    }
    else if (device.anomalyScore >= AI_WARNING_THRESHOLD)
    {
        device.anomalyDetected = true;
        device.alertLevel = AlertLevel::MEDIUM;
    }
    else
    {
        device.anomalyDetected = false;
        device.alertLevel = AlertLevel::NONE;
    }


    /*
     * Print AI information to Serial.
     */

    Serial.print("[AI] SYS: ");
    Serial.print(device.bp.systolic, 1);

    Serial.print(" | DIA: ");
    Serial.print(device.bp.diastolic, 1);

    Serial.print(" | HR: ");
    Serial.print(device.bp.pulseRate, 1);

    Serial.print(" | SpO2: ");
    Serial.print(device.spo2, 1);

    Serial.print(" | MSE: ");
    Serial.print(device.anomalyScore, 2);

    Serial.print(" | Anomaly: ");
    Serial.print(
        device.anomalyDetected
        ? "YES"
        : "NO"
    );

    Serial.print(" | Alert: ");

    switch (device.alertLevel)
    {
        case AlertLevel::NONE:
            Serial.println("NONE");
            break;

        case AlertLevel::LOW_:
            Serial.println("LOW");
            break;

        case AlertLevel::MEDIUM:
            Serial.println("MEDIUM");
            break;

        case AlertLevel::HIGH_:
            Serial.println("HIGH");
            break;

        case AlertLevel::CRITICAL:
            Serial.println("CRITICAL");
            break;
    }
}


/* =========================================================
 * Process MKB0805 Frame
 *
 * This now handles ONLY the raw sensor signals.
 *
 * BP values are supplied by BP_measure().
 * ========================================================= */

void processFrame(
    uint16_t rawPPG,
    uint16_t rawECG)
{
    device.rawPPG = rawPPG;
    device.rawECG = rawECG;


    /*
     * Basic contact validation.
     */
    bool validContact =
        (rawPPG >= 300 &&
         rawPPG <= 65000);


    if (!validContact)
    {
        device.sensorConnected = false;

        fingerContactStartTime = 0;

        if (device.state != DeviceState::WAITING)
        {
            device.state =
                DeviceState::WAITING;

            Serial.println(
                "[SENSOR] Signal lost or optical saturation."
            );
        }

        return;
    }


    device.sensorConnected = true;


    /* =====================================================
     * WAITING
     * ===================================================== */

    if (device.state == DeviceState::WAITING)
    {
        if (fingerContactStartTime == 0)
        {
            fingerContactStartTime =
                millis();
        }
        else if (
            millis() -
            fingerContactStartTime >=
            STABLE_CONTACT_DELAY_MS)
        {
            device.state =
                DeviceState::INIT;

            calibStartTime =
                millis();

            calibMin = 65535.0f;
            calibMax = 0.0f;

            Serial.println(
                "[SENSOR] Finger detected."
            );

            Serial.println(
                "[SENSOR] Calibrating baseline for 5s..."
            );
        }

        return;
    }


    /* =====================================================
     * CALIBRATION
     * ===================================================== */

    if (device.state == DeviceState::INIT)
    {
        if (rawPPG < calibMin)
        {
            calibMin = rawPPG;
        }

        if (rawPPG > calibMax)
        {
            calibMax = rawPPG;
        }


        if (
            millis() -
            calibStartTime >=
            5000)
        {
            device.ppgBaseline =
                (calibMin + calibMax) /
                2.0f;


            float ppgAmplitude =
                calibMax - calibMin;


            device.peakThreshold =
                ppgAmplitude * 0.35f;


            if (device.peakThreshold < 100.0f)
            {
                device.peakThreshold =
                    100.0f;
            }


            device.state =
                DeviceState::MEASURING;


            Serial.println(
                "[SENSOR] Calibration complete!"
            );

            Serial.println(
                "[SENSOR] Live measurement active."
            );
        }

        return;
    }


    /* =====================================================
     * RAW SIGNAL PROCESSING
     *
     * We deliberately do NOT calculate BP here.
     *
     * BP_measure() is now the owner of the BP values.
     * ===================================================== */

    if (
        device.state ==
            DeviceState::MEASURING ||
        device.state ==
            DeviceState::DISPLAY_RESULT)
    {
        device.ppgBaseline =
            (device.ppgBaseline * 0.95f) +
            ((float)rawPPG * 0.05f);


        float ppgAC =
            (float)rawPPG -
            device.ppgBaseline;


        /*
         * Currently retained for raw signal processing.
         *
         * Actual BP values come from BP_measure().
         */
        (void)ppgAC;
    }
}


/* =========================================================
 * Setup
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
    Serial.println(" ADARAMOLA BP MONITOR");
    Serial.println(" ESP32-S3 Edge AI Prototype");
    Serial.println("======================================");


    /* =====================================================
     * MKB0805 UART
     * ===================================================== */

    MKBSerial.begin(
        115200,
        SERIAL_8N1,
        RXD2,
        TXD2
    );


    MKBSerial.write(
        CMD_F7,
        sizeof(CMD_F7)
    );


    lastCommandTime =
        millis();


    /* =====================================================
     * Device Defaults
     * ===================================================== */

    device.state =
        DeviceState::BOOT;

    device.uptime =
        millis();

    device.batteryLevel =
        100;

    device.alertLevel =
        AlertLevel::NONE;

    device.anomalyScore =
        0.0f;

    device.anomalyDetected =
        false;


    /* =====================================================
     * OLED
     * ===================================================== */

    OLED_begin();


    /* =====================================================
     * Buzzer
     * ===================================================== */

    Buzzer_begin();


    /* =====================================================
     * SD
     * ===================================================== */

    // SD_begin();


    /* =====================================================
     * State Machine
     * ===================================================== */

    StateMachine_begin(device);


    /* =====================================================
     * BP Driver
     * ===================================================== */

    BP_begin();


    /*
     * Start prototype in NORMAL mode.
     */

    BP_setSimulationMode(
        SimulationMode::NORMAL
    );


    /* =====================================================
     * Edge AI
     * ===================================================== */

    EdgeAI_Init();


    /* =====================================================
     * Wi-Fi
     *
     * ESP32 operates as its own Access Point.
     *
     * SSID:
     *     Norahlinks-BP
     *
     * IP:
     *     192.168.4.1
     * ===================================================== */

    bool wifiOK =
        WiFiManager_begin();


    device.wifiConnected =
        wifiOK;


    /* =====================================================
     * Web Server
     * ===================================================== */

    WebServer_begin(device);


    /* =====================================================
     * Initial State
     * ===================================================== */

    device.state =
        DeviceState::WAITING;


    OLED_update(device);


    Serial.println();
    Serial.println("======================================");
    Serial.println(" SYSTEM READY");
    Serial.println("======================================");

    Serial.println(
        "[Web] Connect to: Norahlinks-BP"
    );

    Serial.println(
        "[Web] Dashboard: http://192.168.4.1"
    );

    Serial.println();
}


/* =========================================================
 * Main Loop
 * ========================================================= */

void loop()
{
    unsigned long now =
        millis();


    device.uptime =
        now;


    /* =====================================================
     * MKB0805 Command Polling
     * ===================================================== */

    if (
        now -
        lastCommandTime >
        2500)
    {
        MKBSerial.write(
            CMD_F7,
            sizeof(CMD_F7)
        );

        lastCommandTime =
            now;
    }


    /* =====================================================
     * Read MKB0805 UART Data
     * ===================================================== */

    while (MKBSerial.available() > 0)
    {
        uint8_t b =
            MKBSerial.read();


        /*
         * Wait for packet header.
         */
        if (bufferIndex == 0)
        {
            if (b == HEADER_BYTE)
            {
                rawBuffer[
                    bufferIndex++
                ] = b;
            }

            continue;
        }


        /*
         * Store packet byte.
         */
        rawBuffer[
            bufferIndex++
        ] = b;


        /*
         * Complete packet.
         */
        if (bufferIndex == PACKET_SIZE)
        {
            uint16_t rawPPG =
                ((uint16_t)rawBuffer[13] << 8) |
                rawBuffer[14];


            uint16_t rawECG =
                ((uint16_t)rawBuffer[15] << 8) |
                rawBuffer[16];


            processFrame(
                rawPPG,
                rawECG
            );


            bufferIndex = 0;
        }
    }


    /* =====================================================
     * BP Simulation
     *
     * Existing BP driver is the ONLY source of simulated
     * blood pressure values.
     * ===================================================== */

    if (
        now -
        lastBPMeasurement >=
        BP_MEASUREMENT_INTERVAL)
    {
        lastBPMeasurement =
            now;


        /*
         * Generate simulated SYS/DIA/HR.
         */
        BP_measure(device);


        /*
         * Make the prototype display the generated
         * measurement immediately.
         */
        if (device.bp.valid)
        {
            device.state =
                DeviceState::DISPLAY_RESULT;
        }


        /*
         * Run Edge AI using the newly generated BP.
         */
        processEdgeAI();
    }


    /* =====================================================
     * Wi-Fi
     * ===================================================== */

    WiFiManager_update();


    device.wifiConnected =
        WiFiManager_isConnected();


    /* =====================================================
     * Web Server
     * ===================================================== */

    WebServer_update();


    /* =====================================================
     * OLED
     * ===================================================== */

    OLED_update(device);


    /* =====================================================
     * Loop Delay
     * ===================================================== */

    delay(2);
}