#pragma once

#include "fxCommon.hpp"
#include "lxCommon.hpp"
#include "test.hpp"

class NorMedia;

extern std::byte ramMem[10 * 512];

void ramMediaDriver(RamMedia &ramMedia);
void norFlashSimulatorMediaDriver(NorMedia &norMedia);
#if 0
void nandFlashSimulatorMediaDriver(NandMedia &norMedia);
#endif
