#include "SD_ReadWrite.h"
#include "datatypes.h"

static bool sdMounted = false;

// static const char *LOG_FILE = "/bp_log.csv";
static const char *ROOT_DIR       = "/BPMonitor";   

static const char *LOG_DIR        = "/BPMonitor/logs";
static const char *CONFIG_DIR     = "/BPMonitor/config";
static const char *PATIENT_DIR    = "/BPMonitor/patients";
static const char *AI_DIR         = "/BPMonitor/ai";
static const char *EXPORT_DIR     = "/BPMonitor/export";

static const char *BP_LOG_FILE    = "/BPMonitor/logs/bp_log.csv";
static const char *EVENT_LOG_FILE = "/BPMonitor/logs/events.csv";

bool SD_begin()
{
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0);

    if (!SD_MMC.begin("/sdcard", true))
    {
        sdMounted = false;
        Serial.println("[SD] Mount Failed");
        return false;
    }

    sdMounted = true;

    Serial.println("[SD] Mounted");

    SD_createProjectStructure();

    SD_createLog();

    return true;
}

bool SD_isMounted()
{
    return sdMounted;
}

bool SD_writeFile(const char *path, const String &text)
{
    if (!sdMounted)
        return false;

    File file = SD_MMC.open(path, FILE_WRITE);

    if (!file)
        return false;

    file.print(text);

    file.close();

    return true;
}

bool SD_appendFile(const char *path, const String &text)
{
    if (!sdMounted)
        return false;

    File file = SD_MMC.open(path, FILE_APPEND);

    if (!file)
        return false;

    file.print(text);

    file.close();

    return true;
}

String SD_readFile(const char *path)
{
    if (!sdMounted)
        return "";

    File file = SD_MMC.open(path);

    if (!file)
        return "";

    String text;

    while (file.available())
    {
        text += (char)file.read();
    }

    file.close();

    return text;
}

bool SD_deleteFile(const char *path)
{
    if (!sdMounted)
        return false;

    return SD_MMC.remove(path);
}

bool SD_createLog()
{
    if (!sdMounted)
        return false;

    if (SD_MMC.exists(BP_LOG_FILE))
        return true;

    File file = SD_MMC.open(BP_LOG_FILE, FILE_WRITE);

    if (!file)
        return false;

    file.println("Timestamp,Systolic,Diastolic,MAP,HeartRate,Simulation,Alert");

    file.close();

    Serial.println("[SD] Log Created");

    return true;
}

static bool createDirectory(const char *path)
{
    if (SD_MMC.exists(path))
        return true;

    if (SD_MMC.mkdir(path))
    {
        Serial.printf("[SD] Created: %s\n", path);
        return true;
    }

    Serial.printf("[SD] Failed: %s\n", path);
    return false;
}

static bool SD_createProjectStructure()
{
    bool ok = true;

    ok &= createDirectory(ROOT_DIR);
    ok &= createDirectory(LOG_DIR);
    ok &= createDirectory(CONFIG_DIR);
    ok &= createDirectory(PATIENT_DIR);
    ok &= createDirectory(AI_DIR);
    ok &= createDirectory(EXPORT_DIR);

    return ok;
}

bool SD_writeReading(const DeviceStatus &device)
{
    if (!sdMounted)
        return false;

    File file = SD_MMC.open(BP_LOG_FILE, FILE_APPEND);

    if (!file)
    {
        Serial.println("[SD] Failed to open log file.");
        return false;
    }

    file.printf("%lu,", device.bp.timestamp);

    file.printf("%.1f,", device.bp.systolic);

    file.printf("%.1f,", device.bp.diastolic);

    file.printf("%.1f,", device.bp.meanPressure);

    file.printf("%.1f,", device.bp.pulseRate);

    file.printf("%d,", (int)device.simulationMode);

    file.printf("%d\n", (int)device.alertLevel);

    file.close();

    return true;
}