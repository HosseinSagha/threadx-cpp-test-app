#pragma once

#include "fxCommon.hpp"
#include "lxCommon.hpp"
#include "norFlash.hpp"

class NorMedia;

constexpr auto TOTAL_BLOCKS{8};
constexpr auto PHYSICAL_SECTORS_PER_BLOCK{16}; /* Min value of 2, max value of 120 for 1 sector of overhead.  */
constexpr auto USABLE_SECTORS_PER_BLOCK{PHYSICAL_SECTORS_PER_BLOCK - 1};
constexpr auto FREE_BIT_MAP_WORDS{((USABLE_SECTORS_PER_BLOCK - 1) / 32) + 1};
constexpr auto UNUSED_METADATA_WORDS_PER_BLOCK{
    LevelX::norSectorSizeInWord - (3 + FREE_BIT_MAP_WORDS + USABLE_SECTORS_PER_BLOCK)};

struct PhysicalSector
{
    ThreadX::Ulong memory[LevelX::norSectorSizeInWord];
};

struct FlashBlock
{
    ThreadX::Ulong erase_count;
    ThreadX::Ulong min_log_sector;
    ThreadX::Ulong max_log_sector;
    ThreadX::Ulong free_bit_map[FREE_BIT_MAP_WORDS];
    ThreadX::Ulong sector_metadata[USABLE_SECTORS_PER_BLOCK];
    ThreadX::Ulong unused_words[UNUSED_METADATA_WORDS_PER_BLOCK];
    PhysicalSector physical_sectors[USABLE_SECTORS_PER_BLOCK];
};

extern FlashBlock norMem[TOTAL_BLOCKS];

void norFlashSimulatorMediaDriver(NorMedia &norMedia);

LevelX::Error norFlashSimulatorRead(ThreadX::Ulong *flash_address, ThreadX::Ulong *destination, ThreadX::Ulong words);
LevelX::Error norFlashSimulatorWrite(ThreadX::Ulong *flash_address, ThreadX::Ulong *source, ThreadX::Ulong words);
LevelX::Error norFlashSimulatorBlockErase(ThreadX::Ulong block, ThreadX::Ulong erase_count);
LevelX::Error norFlashSimulatorEraseAll();
LevelX::Error norFlashSimulatorBlockErasedVerify(ThreadX::Ulong block);