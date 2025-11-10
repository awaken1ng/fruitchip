#pragma once

#include <stdint.h>

struct UF2_Block {
    // 32 byte header
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;
    uint32_t fileSize; // or familyID;
    uint8_t data[476];
    uint32_t magicEnd;
};

#define UF2_MAGIC_START0 0x0A324655
#define UF2_MAGIC_START1 0x9E5D5157
#define UF2_MAGIC_END 0x0AB16F30

#define UF2_FLAG_NOT_MAIN_FLASH 0x00002000
#define UF2_FLAG_FAMILY_ID_PRESENT 0x00002000
#define UF2_FAMILY_ID_RP2040 0xe48bff56
