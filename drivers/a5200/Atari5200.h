
#ifndef ATARI_5200_H
#define ATARI_5200_H

#include "core/Emulator.h"
#include "core/cpu/SignalLine.h"
#include "core/cpu/6502c.h"
#include "core/memory/RAM.h"
#include "core/memory/ROM.h"
#include "core/video/Antic.h"
#include "core/video/GTIA.h"
#include "core/audio/Pokey.h"
#include "JoyPad.h"

class Atari5200 : public Emulator
{

    public:
        Atari5200();

        BOOL SaveState(const CHAR* filename);
        BOOL LoadState(const CHAR* filename);

    private:
        JoyPad      leftInput;
        JoyPad      rightInput;
        _6502c      cpu;
        Antic       antic;
        GTIA        gtia;
        Pokey       pokey;

        ROM         biosROM;
        RAM         ram;

};

#endif
