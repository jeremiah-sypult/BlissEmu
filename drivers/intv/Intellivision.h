
#ifndef INTELLIVISION_H
#define INTELLIVISION_H

#include "core/Emulator.h"
#include "HandController.h"
#include "ECS.h"
#include "Intellivoice.h"
#include "core/Emulator.h"
#include "core/memory/MemoryBus.h"
#include "core/memory/RAM.h"
#include "core/memory/ROM.h"
#include "core/cpu/CP1610.h"
#include "core/video/AY38900.h"
#include "core/audio/AY38914.h"

class Intellivision : public Emulator
{

    public:
        Intellivision();

    private:
        //core processors
        CP1610            cpu;
        AY38900           stic;
        AY38914           psg;
    
        //core memories
        RAM         RAM8bit;
        RAM         RAM16bit;
        ROM         execROM;
        ROM         grom;
        GRAM        gram;

        //hand controllers
        HandController    player1Controller;
        HandController    player2Controller;

        //the ECS peripheral
        ECS               ecs;
    
        //the Intellivoice peripheral
        Intellivoice      intellivoice;

};

#endif
