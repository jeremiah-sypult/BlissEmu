
#include "Atari5200.h"

Atari5200::Atari5200()
    : Emulator("Atari 5200"),
      ram(0x4000, 0x0000),
      pokeyRegisters(0x0100, 0xE800),
      biosROM("BIOS ROM", "5200.bin", 0, 1, 0x0800, 0xF800),
      cpu(&memoryBus),
      antic(&memoryBus, &gtia)
{
    //make the connections between the antic and the cpu
    antic.connectPinOut(ANTIC_PIN_OUT_NMI, &cpu, _6502C_PIN_IN_NMI);
    antic.connectPinOut(ANTIC_PIN_OUT_HALT, &cpu, _6502C_PIN_IN_HALT);
    antic.connectPinOut(ANTIC_PIN_OUT_READY, &cpu, _6502C_PIN_IN_READY);

    //add the 16K of 8-bit RAM
    AddRAM(&ram);

    //add the *fake* (for now) Pokey registers
    AddRAM(&pokeyRegisters);

    //add the BIOS ROM
	AddROM(&biosROM);
    
    //add the main CPU
    AddProcessor(&cpu);

    //add the Antic
    AddProcessor(&antic);
    AddVideoProducer(&antic);
}

