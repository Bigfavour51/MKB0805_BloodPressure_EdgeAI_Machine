#include "SD_ReadWrite.h"


bool NL_init_sd_card()
{
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0);

    if (!SD_MMC.begin("/sdcard", true)) {
        return false;
    }
    return true;
}