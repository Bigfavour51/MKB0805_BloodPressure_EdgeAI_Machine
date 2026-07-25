#include <Arduino.h>

#include "datatypes.h"
#include "bp_sensor.h"
#include "oled_driver.h"
#include "StateMachine.h"
#include "buzzer_driver.h"

DeviceStatus device;

void setup()
{
    Serial.begin(115200);

    while (!Serial)
    {
        delay(10);
    }

    Serial.println();
    Serial.println("======================================");
    Serial.println(" Adaramola Blood Pressure Monitor");
    Serial.println("======================================");

    //--------------------------------------------------
    // Initialize device status
    //--------------------------------------------------
    device.state = DeviceState::BOOT;
    device.uptime = millis();

    //--------------------------------------------------
    // Initialize all drivers
    //--------------------------------------------------
    OLED_begin();
    Buzzer_begin();
    BP_begin();
    StateMachine_begin(device);

    //--------------------------------------------------
    // Startup beep
    //--------------------------------------------------
    Buzzer_play(BuzzerPattern::STARTUP);

     //--------------------------------------------------
    // Display first screen
    //--------------------------------------------------
    OLED_update(device);

    //--------------------------------------------------
    // Simulate hardware status
    //--------------------------------------------------
    device.sensorConnected = true;
    device.sdMounted = true;
    device.wifiConnected = false;
    device.mqttConnected = false;
    device.batteryLevel = 100;

    //--------------------------------------------------
    // Generate first reading
    //--------------------------------------------------
    BP_measure(device);


}

void loop()
{
    //--------------------------------------------------
    // Keep non-blocking drivers running
    //--------------------------------------------------

    Buzzer_update();
    OLED_update(device);

    static uint32_t timer = 0;

    if (millis() - timer >= 3000)
    {
        timer = millis();

        switch (device.state)
        {
            //--------------------------------------------------
            case DeviceState::BOOT:
            //--------------------------------------------------
            {
                Serial.println("BOOT");
                device.state = DeviceState::WAITING;
            }
            break;

            //--------------------------------------------------
            case DeviceState::WAITING:
            //--------------------------------------------------
            {
                Serial.println("WAITING");

                Buzzer_play(BuzzerPattern::BUTTON);

                device.state = DeviceState::MEASURING;
            }
            break;

            //--------------------------------------------------
            case DeviceState::MEASURING:
            //--------------------------------------------------
            {
                Serial.println("MEASURING");

                BP_measure(device);

                Buzzer_play(BuzzerPattern::SUCCESS);

                device.state = DeviceState::PROCESSING;
            }
            break;

            //--------------------------------------------------
            case DeviceState::PROCESSING:
            //--------------------------------------------------
            {
                Serial.println("PROCESSING");

                // 25% chance of abnormal reading
                if (random(100) < 25)
                {
                    device.alertLevel = AlertLevel::CRITICAL;
                    device.state = DeviceState::ALERT;
                }
                else
                {
                    device.alertLevel = AlertLevel::NONE;
                    device.state = DeviceState::DISPLAY_RESULT;
                }
            }
            break;

            //--------------------------------------------------
            case DeviceState::DISPLAY_RESULT:
            //--------------------------------------------------
            {
                Serial.println("DISPLAY RESULT");

                device.state = DeviceState::UPLOADING;
            }
            break;

            //--------------------------------------------------
            case DeviceState::UPLOADING:
            //--------------------------------------------------
            {
                Serial.println("UPLOADING");

                Buzzer_play(BuzzerPattern::SUCCESS);

                device.state = DeviceState::WAITING;
            }
            break;

            //--------------------------------------------------
            case DeviceState::ALERT:
            //--------------------------------------------------
            {
                Serial.println("CRITICAL ALERT");

                Buzzer_play(BuzzerPattern::CRITICAL);

                device.state = DeviceState::DISPLAY_RESULT;
            }
            break;

            //--------------------------------------------------
            case DeviceState::ERROR_STATE:
            //--------------------------------------------------
            {
                Serial.println("ERROR");

                Buzzer_play(BuzzerPattern::ERROR);

                device.state = DeviceState::WAITING;
            }
            break;

            default:
            {
                device.state = DeviceState::WAITING;
            }
            break;
        }
    }
}