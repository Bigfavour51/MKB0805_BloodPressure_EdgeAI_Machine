#ifndef BP_SENSOR_H
#define BP_SENSOR_H

#include <Arduino.h>
#include "datatypes.h"

/******************************************************
 * Initialization
 ******************************************************/
bool BP_begin(void);

/******************************************************
 * Sensor Status
 ******************************************************/
bool BP_isConnected(void);

/******************************************************
 * Measurement
 ******************************************************/
bool BP_measure(DeviceStatus &device);

/******************************************************
 * Simulation Control
 ******************************************************/
void BP_setSimulationMode(SimulationMode mode);

SimulationMode BP_getSimulationMode(void);

#endif