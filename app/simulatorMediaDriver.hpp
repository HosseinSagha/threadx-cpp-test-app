#pragma once

#include "fxCommon.hpp"
#include "lxCommon.hpp"
#include "test.hpp"

class NorMedia;

extern std::byte ramMem[10 * 512];

void ramMediaDriver(FileX::Media<>::InternalDriver &ramMedia);
void norFlashSimulatorMediaDriver(FileX::Media<NorFlash::sectorSize()>::InternalDriver &norMedia, NorFlash &norFlash);
#if 0
void nandFlashSimulatorMediaDriver(NandMedia &norMedia);
#endif
