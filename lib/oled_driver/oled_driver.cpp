#include "oled_driver.h"

#include <Wire.h>

#include "config.h"

/************************************************
 * Display Object
 ***********************************************/

static U8G2_SSD1306_128X64_NONAME_F_HW_I2C
oled(U8G2_R0);

/************************************************
 * Internal Variables
 ***********************************************/

static bool oledReady = false;

static DeviceState lastState =
    DeviceState::BOOT;

/************************************************
 * Private Function Prototypes
 ***********************************************/

static void drawHeader(
    const DeviceStatus &device);

static void drawFooter();

static void drawSplash();

static void drawWaiting(
    const DeviceStatus &device);

static void drawMeasurement(
    const DeviceStatus &device);

static void drawProcessing();

static void drawResult(
    const DeviceStatus &device);

static void drawUploading(
    const DeviceStatus &device);

static void drawAlert(
    const DeviceStatus &device);

static void drawError();

/************************************************
 * Public Functions
 ***********************************************/

bool OLED_begin()
{
    Wire.begin(
        Config::OLED_SDA,
        Config::OLED_SCL);

    oled.setI2CAddress(
        Config::OLED_ADDR << 1);

    oled.begin();

    oled.clearBuffer();

    oled.sendBuffer();

    oledReady = true;

    return true;
}

bool OLED_isReady()
{
    return oledReady;
}

void OLED_clear()
{
    oled.clearBuffer();

    oled.sendBuffer();
}

static void drawHeader(const DeviceStatus &device)
{
    oled.setFont(u8g2_font_5x7_tf);

    // WiFi
    oled.drawStr(0, 7, device.wifiConnected ? "W" : "-");

    // SD Card
    oled.drawStr(12, 7, device.sdMounted ? "S" : "-");

    // MQTT
    oled.drawStr(24, 7, device.mqttConnected ? "M" : "-");

    // Battery
    char battery[12];
    sprintf(battery, "%d%%", device.batteryLevel);
    oled.drawStr(96, 7, battery);

    oled.drawHLine(0, 10, 128);
}

static void drawFooter()
{
    oled.drawHLine(0, 54, 128);

    oled.setFont(u8g2_font_5x7_tf);

    oled.drawStr(18, 63, "Adaramola BP Monitor");
}

static void drawSplash()
{
    oled.setFont(u8g2_font_ncenB10_tr);
    oled.drawStr(16, 25, "Adaramola");

    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(12, 45, "Blood Pressure");
    oled.drawStr(38, 58, "Monitor");
}

static void drawWaiting(const DeviceStatus &device)
{
    (void)device;

    oled.setFont(u8g2_font_6x12_tf);

    oled.drawStr(35, 25, "READY");

    oled.setFont(u8g2_font_5x7_tf);

    oled.drawStr(20, 42, "Waiting for Measurement");
}

static void drawMeasurement(const DeviceStatus &device)
{
    char text[32];

    oled.setFont(u8g2_font_6x12_tf);

    sprintf(text, "SYS : %.0f", device.bp.systolic);
    oled.drawStr(0, 22, text);

    sprintf(text, "DIA : %.0f", device.bp.diastolic);
    oled.drawStr(0, 36, text);

    sprintf(text, "HR  : %.0f", device.bp.pulseRate);
    oled.drawStr(0, 50, text);
}

static void drawProcessing()
{
    oled.setFont(u8g2_font_6x12_tf);

    oled.drawStr(22, 28, "Processing");

    oled.setFont(u8g2_font_5x7_tf);

    oled.drawStr(35, 44, "Running AI...");
}

static void drawResult(const DeviceStatus &device)
{
    char text[32];

    oled.setFont(u8g2_font_6x12_tf);

    sprintf(text, "SYS %.0f", device.bp.systolic);
    oled.drawStr(0, 18, text);

    sprintf(text, "DIA %.0f", device.bp.diastolic);
    oled.drawStr(0, 31, text);

    sprintf(text, "MAP %.0f", device.bp.meanPressure);
    oled.drawStr(0, 44, text);

    sprintf(text, "HR  %.0f", device.bp.pulseRate);
    oled.drawStr(70, 18, text);
}

static void drawUploading(const DeviceStatus &device)
{
    (void)device;

    oled.setFont(u8g2_font_6x12_tf);

    oled.drawStr(22, 28, "Uploading");

    oled.setFont(u8g2_font_5x7_tf);

    oled.drawStr(30, 45, "Please wait...");
}

static void drawAlert(const DeviceStatus &device)
{
    char text[32];

    oled.setFont(u8g2_font_6x12_tf);

    oled.drawStr(30, 18, "WARNING");

    sprintf(text, "SYS %.0f", device.bp.systolic);
    oled.drawStr(0, 36, text);

    sprintf(text, "DIA %.0f", device.bp.diastolic);
    oled.drawStr(0, 50, text);
}

static void drawError()
{
    oled.setFont(u8g2_font_6x12_tf);

    oled.drawStr(35, 25, "ERROR");

    oled.setFont(u8g2_font_5x7_tf);

    oled.drawStr(18, 42, "Sensor not available");
}

void OLED_update( const DeviceStatus &device)
{
    if(!oledReady)
        return;

    oled.clearBuffer();

    drawHeader(device);

    switch(device.state)
    {
        case DeviceState::BOOT:

            drawSplash();

            break;

        case DeviceState::WAITING:

            drawWaiting(device);

            break;

        case DeviceState::MEASURING:

            drawMeasurement(device);

            break;

        case DeviceState::PROCESSING:

            drawProcessing();

            break;

        case DeviceState::DISPLAY_RESULT:

            drawResult(device);

            break;

        case DeviceState::UPLOADING:

            drawUploading(device);

            break;

        case DeviceState::ALERT:

            drawAlert(device);

            break;

        case DeviceState::ERROR_STATE:

            drawError();

            break;

        default:

            drawError();

            break;
    }

    drawFooter();

    oled.sendBuffer();

    lastState = device.state;
}