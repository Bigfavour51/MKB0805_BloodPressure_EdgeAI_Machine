#include "bp_sensor.h"

#include <esp_system.h>

/******************************************************
 * Internal Variables
 ******************************************************/

static bool sensorConnected = false;

static SimulationMode currentMode = SimulationMode::NORMAL;

/*
    Internal simulated patient

    These values persist between measurements
    and slowly evolve like a real patient.
*/
static float sys = 120.0f;
static float dia = 80.0f;
static float hr  = 72.0f;

/******************************************************
 * Helper Functions
 ******************************************************/

static float randomFloat(float min, float max)
{
    return min +
           ((float)random(0,10000) / 10000.0f) *
           (max - min);
}

/******************************************************
 * Simulation Models
 ******************************************************/

static void updateNormal()
{
    sys += randomFloat(-0.25f,0.25f);
    dia += randomFloat(-0.20f,0.20f);
    hr  += randomFloat(-0.30f,0.30f);

    sys = constrain(sys,115.0f,125.0f);
    dia = constrain(dia,75.0f,85.0f);
    hr  = constrain(hr,68.0f,75.0f);
}

static void updateHypertension()
{
    if(sys < 155.0f)
        sys += randomFloat(0.05f,0.35f);

    if(dia < 98.0f)
        dia += randomFloat(0.05f,0.20f);

    hr += randomFloat(-0.20f,0.40f);

    hr = constrain(hr,70.0f,90.0f);
}

static void updateHypotension()
{
    if(sys > 90.0f)
        sys -= randomFloat(0.05f,0.30f);

    if(dia > 60.0f)
        dia -= randomFloat(0.05f,0.20f);

    hr += randomFloat(-0.40f,0.40f);

    hr = constrain(hr,60.0f,85.0f);
}

static void updateExercise()
{
    if(sys < 165.0f)
        sys += randomFloat(0.10f,0.40f);

    if(dia < 90.0f)
        dia += randomFloat(0.00f,0.10f);

    if(hr < 145.0f)
        hr += randomFloat(0.60f,1.20f);
}

static void updateRecovery()
{
    if(sys > 120.0f)
        sys -= randomFloat(0.10f,0.30f);

    if(dia > 80.0f)
        dia -= randomFloat(0.05f,0.15f); 

    if(hr > 72.0f)
        hr -= randomFloat(0.60f,1.00f);
}

static void updateCritical()
{
    sys = randomFloat(180.0f,210.0f);

    dia = randomFloat(120.0f,135.0f);

    hr = randomFloat(95.0f,120.0f);
}

static void updateRandom()
{
    sys = randomFloat(85.0f,190.0f);

    dia = randomFloat(55.0f,120.0f);

    hr = randomFloat(55.0f,170.0f);
}

/******************************************************
 * Public Functions
 ******************************************************/

bool BP_begin(void)
{
    randomSeed(esp_random());

    sensorConnected = true;

    return sensorConnected;
}

bool BP_isConnected(void)
{
    return sensorConnected;
}

void BP_setSimulationMode(SimulationMode mode)
{
    currentMode = mode;
}

SimulationMode BP_getSimulationMode(void)
{
    return currentMode;
}

bool BP_measure(DeviceStatus &device)
{
    if(!sensorConnected)
    {
        device.sensorConnected = false;
        device.bp.valid = false;

        return false;
    }

    switch(currentMode)
    {
        case SimulationMode::NORMAL:
            updateNormal();
            break;

        case SimulationMode::HYPERTENSION:
            updateHypertension();
            break;

        case SimulationMode::HYPOTENSION:
            updateHypotension();
            break;

        case SimulationMode::EXERCISE:
            updateExercise();
            break;

        case SimulationMode::RECOVERY:
            updateRecovery();
            break;

        case SimulationMode::CRITICAL:
            updateCritical();
            break;

        case SimulationMode::RANDOM:
            updateRandom();
            break;
    }

    device.bp.timestamp = millis();

    device.bp.systolic = sys;

    device.bp.diastolic = dia;

    device.bp.meanPressure =
        (sys + (2.0f * dia)) / 3.0f;

    device.bp.pulseRate = hr;

    device.bp.valid = true;

    device.sensorConnected = sensorConnected;

    device.simulationMode = currentMode;

    return true;
}