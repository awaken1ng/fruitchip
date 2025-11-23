#include "stdint.h"
#include "stdio.h"

#include "libcdvd.h"
#include "osd_config.h"
#include "kernel.h"

#include "utils.h"

// borrowed from https://github.com/pcm720/OSDMenu/blob/006863147f584b0194fda992c4d1c38495061221/mbr/src/disc.c

// PS1DRV flags
#define CDROM_PS1_FAST 0x1
#define CDROM_PS1_SMOOTH 0x10

// Describes the first two OSD NVRAM blocks
typedef struct {
  // PS1DRV settings block
  // Byte 0
  uint8_t ps1drvDiscSpeed : 4;
  uint8_t ps1drvTextureMapping : 4;
  // Bytes 1-14, unused
  uint8_t unused1[14];

  // PlayStation 2 settings block
  // Byte 0
  uint8_t ps2SpdifDisabled : 1;
  uint8_t ps2ScreenType : 2;
  uint8_t ps2VideoOutput : 1;
  uint8_t ps2OldLanguage : 1;
  uint8_t ps2ConfigVersion : 1;
  uint8_t ps2Reserved : 2;
  // Byte 1
  uint8_t ps2NewLanguage : 5;
  uint8_t ps2MaxVersion : 3;
  // Byte 2
  uint8_t ps2TimezoneHigh : 3;
  uint8_t ps2DaylightSavings : 1;
  uint8_t ps2TimeNotation : 1;
  uint8_t ps2DateNotation : 2;
  uint8_t ps2OOBEFlag : 1;
  // Byte 3
  uint8_t ps2TimezoneLow;
  // Bytes 4-14, the rest of config block
  uint8_t unused2[11];
} OSDNVRAMConfig;

// Updates MechaCon NVRAM config with selected PS1DRV options, screen type and language.
// Applies kernel patches and sets the OSD configuration
void init_osd_config()
{
    sceCdInit(SCECdINoD);
    int res;
    uint32_t status;

    do
    {
        res = sceCdOpenConfig(1, 0, 2, (u32 *)&status);
        if (!res)
            return;
    } while (status & 0x81);

    uint8_t buffer[30] = {0};

    do
    {
        res = sceCdReadConfig(buffer, (u32 *)&status);
    } while ((status & 0x81) || (res == 0));
    do
    {
        res = sceCdCloseConfig((u32 *)&status);
    } while ((status & 0x81) || (res == 0));

    OSDNVRAMConfig *cnf = (OSDNVRAMConfig *)buffer;
    printf(
        "NVRAM Settings:\n\t"
            "PS1DRV:\n\t\t"
                "Disc speed: %d\n\t\t"
                "Mapping: %d\n\t"
            "OSD:\n\t\t"
                "SPDIF: %d\n\t\t"
                "Screen: %d\n\t\t"
                "Video Out: %d\n\t\t"
                "Old language: %d\n\t\t"
                "Config version: %d\n\t\t"
                "Language: %d\n\t\t"
                "Max version: %d\n\t\t"
                "TimezoneH: %d\n\t\t"
                "TimezoneL: %d\n",
        cnf->ps1drvDiscSpeed,
        cnf->ps1drvTextureMapping,
        cnf->ps2SpdifDisabled,
        cnf->ps2ScreenType,
        cnf->ps2VideoOutput,
        cnf->ps2OldLanguage,
        cnf->ps2ConfigVersion,
        cnf->ps2NewLanguage,
        cnf->ps2MaxVersion,
        cnf->ps2TimezoneHigh,
        cnf->ps2TimezoneLow
    );

    sceCdInit(SCECdEXIT);

    // Apply kernel patches for early kernels
    InitOsd();

    // Set kernel config
    ConfigParam osdConfig = {0};
    // Initialize ConfigParam values
    osdConfig.version = 2;
    osdConfig.spdifMode = cnf->ps2SpdifDisabled;
    osdConfig.screenType = cnf->ps2ScreenType;
    osdConfig.videoOutput = cnf->ps2VideoOutput;
    osdConfig.timezoneOffset = ((uint32_t)cnf->ps2TimezoneHigh << 8 | (uint32_t)cnf->ps2TimezoneLow) & 0x7FF;

    if (cnf->ps1drvDiscSpeed)
        osdConfig.ps1drvConfig |= CDROM_PS1_FAST;
    if (cnf->ps1drvTextureMapping)
        osdConfig.ps1drvConfig |= CDROM_PS1_SMOOTH;

    // Force ConfigParam language to English if one of extended languages is used
    osdConfig.language = (cnf->ps2NewLanguage > LANGUAGE_PORTUGUESE) ? LANGUAGE_ENGLISH : cnf->ps2NewLanguage;
    osdConfig.japLanguage = (cnf->ps2OldLanguage == LANGUAGE_JAPANESE) ? cnf->ps2OldLanguage : LANGUAGE_ENGLISH;

    // Set ConfigParam and check whether the version was not set
    // Early kernels can't retain the version value
    SetOsdConfigParam(&osdConfig);
    GetOsdConfigParam(&osdConfig);

    printf(
        "Kernel Settings:\n\t"
            "OSD1:\n\t\t"
                "Version: %d\n\t\t"
                "SPDIF: %d\n\t\t"
                "Screen Type: %d\n\t\t"
                "Video Output: %d\n\t\t"
                "Language 1: %d\n\t\t"
                "Language 2: %d\n\t\t"
                "TZ Offset: %d\n\t\t"
                "PS1DRV: %d\n",
        osdConfig.version,
        osdConfig.spdifMode,
        osdConfig.screenType,
        osdConfig.videoOutput,
        osdConfig.japLanguage,
        osdConfig.language,
        osdConfig.timezoneOffset,
        osdConfig.ps1drvConfig
    );

    if (osdConfig.version != 0)
    {
        // This kernel supports ConfigParam2.
        // Initialize ConfigParam2 values
        Config2Param osdConfig2 = {0};
        GetOsdConfigParam2(&osdConfig2, sizeof(osdConfig2), 0);

        osdConfig2.format = 2;
        osdConfig2.version = 2;
        osdConfig2.language = cnf->ps2NewLanguage;
        osdConfig2.timeFormat = cnf->ps2TimeNotation;
        osdConfig2.dateFormat = cnf->ps2DateNotation;
        osdConfig2.daylightSaving = cnf->ps2DaylightSavings;

        SetOsdConfigParam2(&osdConfig2, sizeof(osdConfig2), 0);

        printf(
            "\tOSD2:\n\t\t"
                "Version: %d\n\t\t"
                "Format: %d\n\t\t"
                "Daylight Savings: %d\n\t\t"
                "Time Format: %d\n\t\t"
                "Date Format: %d\n\t\t"
                "Language: %d\n",
            osdConfig2.format,
            osdConfig2.daylightSaving,
            osdConfig2.timeFormat,
            osdConfig2.dateFormat,
            osdConfig2.version,
            osdConfig2.language
        );
    }
}
