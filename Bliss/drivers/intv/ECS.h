
#ifndef ECS_H
#define ECS_H

#include "core/Peripheral.h"
#include "ECSKeyboard.h"
#include "core/audio/AudioOutputLine.h"
#include "core/memory/RAM.h"
#include "core/memory/ROM.h"
#include "core/memory/ROMBanker.h"
#include "core/types.h"
#include "core/audio/AY38914.h"

class Intellivision;

class ECS : public Peripheral
{

    friend class Intellivision;

    public:
        ECS();

    private:
        ECSKeyboard         keyboard;

    public:
        ROM       bank0;
        ROMBanker bank0Banker;
        ROM       bank1;
        ROMBanker bank1Banker;
        ROM       bank2;
        ROMBanker bank2Banker;
        RAM       ramBank;
        RAM       uart;
        AY38914   psg2;

};

#endif
