#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>

#include "datatypes.h"

bool OLED_begin();

bool OLED_isReady();

void OLED_update(const DeviceStatus &device);

void OLED_clear();

void OLED_sleep();

void OLED_wakeup();

#endif