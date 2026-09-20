#ifndef MAX30100_DRIVER_H
#define MAX30100_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "datatypes.h"

// Non-blocking driver functions
bool MAX30100_begin();
void MAX30100_update(DeviceStatus &device);

#endif // MAX30100_DRIVER_H