
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

#define ECS_RAM_SIZE    0x0800

TYPEDEF_STRUCT_PACK( _ECSState
{
    RAMState     ramState;
    UINT16       ramImage[ECS_RAM_SIZE];
    RAMState     uartState;
    AY38914State psg2State;
} ECSState; )

class Intellivision;

class ECS : public Peripheral
{

    friend class Intellivision;

    public:
        ECS();

        ECSState getState();
        void setState(ECSState state);

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
