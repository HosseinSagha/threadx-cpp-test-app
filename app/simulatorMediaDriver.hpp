#pragma once

#include "fxCommon.hpp"
#include "lxCommon.hpp"
#include "test.hpp"

class NorMedia;

extern std::byte ramMem[20 * 512];
extern NorFlash::Block norMem[norBlocks];

void ramDriver(RamMedia &ramMedia);
void norFlashSimulatorMediaDriver(NorMedia &norMedia);

LevelX::Error norFlashSimulatorRead(ThreadX::Ulong *flash_address, ThreadX::Ulong *destination, ThreadX::Ulong words);
LevelX::Error norFlashSimulatorWrite(ThreadX::Ulong *flash_address, ThreadX::Ulong *source, ThreadX::Ulong words);
LevelX::Error norFlashSimulatorBlockErase(ThreadX::Ulong block, ThreadX::Ulong erase_count);
LevelX::Error norFlashSimulatorEraseAll();
LevelX::Error norFlashSimulatorBlockErasedVerify(ThreadX::Ulong block);