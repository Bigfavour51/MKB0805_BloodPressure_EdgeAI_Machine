#ifndef SD_READWRITE_H
#define SD_READWRITE_H

#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"

constexpr uint8_t SD_CLK = 39;
constexpr uint8_t SD_CMD = 38;
constexpr uint8_t SD_D0  = 40;

bool NL_init_sd_card();



// unimplemented functions
bool NL_write_file(const char *path, const String &text);
String NL_read_file(const char *path);
bool NL_append_file(const char *path, const String &text);
bool NL_delete_file(const char *path);

#endif // SD_READWRITE_H