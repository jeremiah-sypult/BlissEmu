
#ifndef ATARI_5200_H
#define ATARI_5200_H

#include "core/Emulator.h"
#include "core/cpu/SignalLine.h"
#include "core/cpu/6502c.h"
#include "core/memory/RAM.h"
#include "core/memory/ROM.h"
#include "core/video/Antic.h"
#include "core/video/GTIA.h"

class Atari5200 : public Emulator
{

    public:
        Atari5200();
        BOOL inVerticalBlank() { return antic.inVerticalBlank(); }
        void setBIOSImage(UINT16* biosImage);

    private:
        void init();

        _6502c      cpu;
        Antic       antic;
        GTIA        gtia;

        ROM         biosROM;
        RAM         ram;
        RAM         pokeyRegisters;

};

#endif

