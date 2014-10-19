
#include "Atari5200.h"

Atari5200::Atari5200()
    : Emulator("Atari 5200"),
      ram(0x4000, 0x0000),
      leftInput(3, "Left JoyPad"),
      rightInput(4, "Right JoyPad"),
      pokey(&leftInput, &rightInput),
      biosROM("BIOS ROM", "5200.bin", 0, 1, 0x0800, 0xF800),
      cpu(&memoryBus),
      antic(&memoryBus, &gtia)
{
	videoWidth = 336;
	videoHeight = 240;

    //make the connections between the antic and the cpu
    antic.connectPinOut(ANTIC_PIN_OUT_NMI, &cpu, _6502C_PIN_IN_NMI);
    antic.connectPinOut(ANTIC_PIN_OUT_HALT, &cpu, _6502C_PIN_IN_HALT);
    antic.connectPinOut(ANTIC_PIN_OUT_READY, &cpu, _6502C_PIN_IN_READY);
    pokey.connectPinOut(POKEY_PIN_OUT_IRQ, &cpu, _6502C_PIN_IN_IRQ);

    //add the 16K of 8-bit RAM
    AddRAM(&ram);

    //add the BIOS ROM
	AddROM(&biosROM);
    
    //add the main CPU
    AddProcessor(&cpu);

    //add the Antic and GTIA
    AddProcessor(&antic);
    AddVideoProducer(&antic);
    AddRAM(&antic.registers);
    AddRAM(&gtia.registers);

    //add the Pokey
    AddProcessor(&pokey);
    AddAudioProducer(&pokey);
    pokey.resetProcessor();
    AddRAM(&pokey.registers);

    //add the joypads
    AddInputConsumer(&leftInput);
    AddInputConsumer(&rightInput);
}

