#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "datatypes.h"

bool StateMachine_begin(DeviceStatus &device);

void StateMachine_update(DeviceStatus &device);

void StateMachine_setState(DeviceStatus &device, DeviceState newState);

DeviceState StateMachine_getState(const DeviceStatus &device);

#endif