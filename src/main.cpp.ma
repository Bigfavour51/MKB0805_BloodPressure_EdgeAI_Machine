#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#define OLED_SDA 8
#define OLED_SCL 9
#define OLED_ADDR 0x3C

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

void setup()
{
    Serial.begin(115200);

    while (!Serial)
        delay(10);

    Serial.println();
    Serial.println("OLED Test Starting...");

    Wire.begin(OLED_SDA, OLED_SCL);

    oled.setI2CAddress(OLED_ADDR << 1);

    oled.begin();

    oled.clearBuffer();

    oled.setFont(u8g2_font_ncenB08_tr);
    oled.drawStr(15, 15, "OLED TEST");

    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(5, 35, "ESP32-S3");

    oled.drawFrame(0, 0, 128, 64);

    oled.sendBuffer();

    Serial.println("OLED Initialized");
}

void loop()
{
    static uint32_t last = 0;
    static uint32_t counter = 0;

    if (millis() - last >= 1000)
    {
        last = millis();

        counter++;

        oled.clearBuffer();

        oled.drawFrame(0, 0, 128, 64);

        oled.setFont(u8g2_font_ncenB08_tr);
        oled.drawStr(10, 15, "OLED ALIVE");

        oled.setFont(u8g2_font_6x12_tf);

        char text[30];

        sprintf(text, "Counter: %lu", counter);
        oled.drawStr(10, 35, text);

        sprintf(text, "Millis: %lu", millis() / 1000);
        oled.drawStr(10, 52, text);

        oled.sendBuffer();

        Serial.println(counter);
    }
}