#ifndef SD_DRIVER_H
#define SD_DRIVER_H

#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#include "datatypes.h"

constexpr uint8_t SD_CLK = 39;
constexpr uint8_t SD_CMD = 38;
constexpr uint8_t SD_D0  = 40;

bool SD_begin();
bool SD_isMounted();

bool SD_writeFile(const char *path, const String &text);
String SD_readFile(const char *path);
bool SD_appendFile(const char *path, const String &text);
bool SD_deleteFile(const char *path);

// Project-specific helpers
bool SD_createLog();
bool SD_writeReading(const DeviceStatus &device);
bool SD_writeEvent(const String &event);

bool SD_writeReading(const DeviceStatus &device);
static bool SD_createProjectStructure();
bool SD_writeEvent(const String &event);

#endif