
#include "Pokey_Registers.h"
#include "Pokey.h"

UINT16 POKEY_ALIASES[8] = {
    0x0100, 0x0120, 0x0140, 0x0160, 0x0180, 0x01A0, 0x01C0, 0x01E0,
};

Pokey_Registers::Pokey_Registers()
{
    pokey = NULL;
}

void Pokey_Registers::init(Pokey* p)
{
    pokey = p;
}

UINT16 Pokey_Registers::getSize()
{
    return 0x20;
}

UINT16 Pokey_Registers::getNumLocations()
{
    return 8;
}

UINT16 Pokey_Registers::getLocation(UINT16 i)
{
    return POKEY_ALIASES[i];
}

BOOL Pokey_Registers::isLocationReadable(UINT16)
{
    return TRUE;
}

BOOL Pokey_Registers::isLocationWriteable(UINT16)
{
    return TRUE;
}

void Pokey_Registers::poke(UINT16 addr, UINT16)
{
    switch (addr) {
        case 0x0:  //AUDF1
            break;
        case 0x1:  //AUDC1
            break;
        case 0x2:  //AUDF2
            break;
        case 0x3:  //AUDC2
            break;
        case 0x4:  //AUDF3
            break;
        case 0x5:  //AUDC3
            break;
        case 0x6:  //AUDF4
            break;
        case 0x7:  //AUDC4
            break;
        case 0x8:  //AUDCTL
            break;
        case 0x9:  //STIMER
            break;
        case 0xA:  //SKRES
            break;
        case 0xB:  //POTGO
            break;
        case 0xD:  //SEROUT
            break;
        case 0xE:  //IRQEN
            break;
        case 0xF:  //SKCTLS
            break;
    }
}

UINT16 Pokey_Registers::peek(UINT16 addr)
{
    switch (addr) {
        case 0x0:  //POT0
            break;
        case 0x1:  //POT1
            break;
        case 0x2:  //POT2
            break;
        case 0x3:  //POT3
            break;
        case 0x4:  //POT4
            break;
        case 0x5:  //POT5
            break;
        case 0x6:  //POT6
            break;
        case 0x7:  //POT7
            break;
        case 0x8:  //ALLPOT
            break;
        case 0x9:  //KBCODE
            break;
        case 0xA:  //RANDOM
            break;
        case 0xD:  //SERIN
            break;
        case 0xE:  //IRQST
            break;
        case 0xF:  //SKSTAT
            break;
    }
    //unimplemented or write-only registers
    return 0xFF;
}

