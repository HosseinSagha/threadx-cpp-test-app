#pragma once

#include "fxCommon.hpp"
#include "lxCommon.hpp"
#include "norFlash.hpp"

class NorMedia;

constexpr auto TOTAL_BLOCKS{8};
constexpr auto PHYSICAL_SECTORS_PER_BLOCK{16}; /* Min value of 2, max value of 122 for 1 sector of overhead.  */
constexpr auto USABLE_SECTORS_PER_BLOCK{PHYSICAL_SECTORS_PER_BLOCK - 1};
constexpr auto FREE_BIT_MAP_WORDS{((USABLE_SECTORS_PER_BLOCK - 1) / 32) + 1};
constexpr auto UNUSED_METADATA_WORDS_PER_BLOCK{
    LevelX::norSectorSizeInWord - (3 + FREE_BIT_MAP_WORDS + USABLE_SECTORS_PER_BLOCK)};

struct SectorMetadata
{
    ThreadX::Ulong logicalsector : 29; //Logical sector mapped to this physical sector—when not all ones.
    ThreadX::Ulong writeComplete : 1;  //Mapping entry write is complete when this bit is 0
    ThreadX::Ulong
        obsoleteFlag : 1; //Obsolete flag. When clear, this mapping is either obsolete or is in the process of becoming obsolete.
    ThreadX::Ulong validFlag : 1; //Valid flag. When set and logical sector not all ones indicates mapping is valid
};

struct PhysicalSector
{
    ThreadX::Ulong memory[LevelX::norSectorSizeInWord];
};

struct FlashBlock
{
    ThreadX::Ulong eraseCount;
    ThreadX::Ulong minLogSector;
    ThreadX::Ulong maxLogSector;
    ThreadX::Ulong freeBitMap[FREE_BIT_MAP_WORDS];
    SectorMetadata sectorMetadata[USABLE_SECTORS_PER_BLOCK];
    ThreadX::Ulong unusedWords[UNUSED_METADATA_WORDS_PER_BLOCK];
    PhysicalSector physicalSectors[USABLE_SECTORS_PER_BLOCK];
};

extern FlashBlock norMem[TOTAL_BLOCKS];

void norFlashSimulatorMediaDriver(NorMedia &norMedia);

LevelX::Error norFlashSimulatorRead(ThreadX::Ulong *flash_address, ThreadX::Ulong *destination, ThreadX::Ulong words);
LevelX::Error norFlashSimulatorWrite(ThreadX::Ulong *flash_address, ThreadX::Ulong *source, ThreadX::Ulong words);
LevelX::Error norFlashSimulatorBlockErase(ThreadX::Ulong block, ThreadX::Ulong erase_count);
LevelX::Error norFlashSimulatorEraseAll();
LevelX::Error norFlashSimulatorBlockErasedVerify(ThreadX::Ulong block);