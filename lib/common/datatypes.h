#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <Arduino.h>


enum class SimulationMode
{
    NORMAL,
    HYPERTENSION,
    HYPOTENSION,
    EXERCISE,
    RECOVERY,
    CRITICAL,
    RANDOM
};

enum class AlertLevel
{
    NONE,
    LOW_,
    MEDIUM,
    HIGH_,
    CRITICAL
};

enum class DeviceState
{
    BOOT,
    INIT,
    WAITING,
    MEASURING,
    PROCESSING,
    DISPLAY_RESULT,
    UPLOADING,
    ALERT,
    ERROR_STATE
};

struct BPReading
{
    uint32_t timestamp = 0;

    float systolic = 0.0f;
    float diastolic = 0.0f;
    float meanPressure = 0.0f;
    float pulseRate = 0.0f;

    bool valid = false;
};

enum class BuzzerPattern
{
    NONE,

    STARTUP,

    BUTTON,

    SUCCESS,

    WARNING,

    CRITICAL,

    ERROR,

    SHUTDOWN
};

struct DeviceStatus
{
    BPReading bp;

    DeviceState state = DeviceState::BOOT;

    bool sensorConnected = false;

    bool sdMounted = false;

    bool wifiConnected = false;

    bool mqttConnected = false;

    bool anomalyDetected = false;

    bool measuring = false;

    uint8_t batteryLevel = 100;

    uint32_t uptime = 0;

    AlertLevel alertLevel = AlertLevel::NONE;

    SimulationMode simulationMode = SimulationMode::NORMAL;
};


#endif