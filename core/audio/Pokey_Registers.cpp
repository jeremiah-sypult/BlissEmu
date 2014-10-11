
#include "Pokey_Registers.h"
#include "Pokey.h"


Pokey_Registers::Pokey_Registers()
: RAM(0x10, 0xE800, 0xFF0F, 0xFF0F, 8),
  pokey(NULL)
{
    pokey = NULL;
}

void Pokey_Registers::init(Pokey* p)
{
    pokey = p;
}

void Pokey_Registers::poke(UINT16 addr, UINT16 val)
{
    val &= 0xFF;
    switch (addr & 0x0F) 
    {
        case 0x0:  //AUDF1
            pokey->AUDF1 = (UINT8)val;
            break;
        case 0x1:  //AUDC1
            pokey->AUDC1 = (UINT8)val;
            break;
        case 0x2:  //AUDF2
            pokey->AUDF2 = (UINT8)val;
            break;
        case 0x3:  //AUDC2
            pokey->AUDC2 = (UINT8)val;
            break;
        case 0x4:  //AUDF3
            pokey->AUDF3 = (UINT8)val;
            break;
        case 0x5:  //AUDC3
            pokey->AUDC3 = (UINT8)val;
            break;
        case 0x6:  //AUDF4
            pokey->AUDF4 = (UINT8)val;
            break;
        case 0x7:  //AUDC4
            pokey->AUDC4 = (UINT8)val;
            break;
        case 0x8:  //AUDCTL
            pokey->AUDCTL = (UINT8)val;
            break;
        case 0x9:  //STIMER
            pokey->STIMER = (UINT8)val;
            break;
        case 0xA:  //SKRES
            pokey->SKRES = (UINT8)val;
            break;
        case 0xB:  //POTGO
            pokey->POTGO = (UINT8)val;
            break;
        case 0xD:  //SEROUT
            pokey->SEROUT = (UINT8)val;
            break;
        case 0xE:  //IRQEN
            pokey->IRQEN = (UINT8)val;
            pokey->IRQST |= (val ^ 0xFF);
            break;
        case 0xF:  //SKCTLS
            pokey->SKCTLS = (UINT8)val;
            break;
    }
}

UINT16 Pokey_Registers::peek(UINT16 addr)
{
    switch (addr & 0x0F) 
    {
        case 0x0:  //POT0
            return pokey->leftInput->getPotX();
        case 0x1:  //POT1
            return pokey->leftInput->getPotY();
        case 0x2:  //POT2
            return pokey->rightInput->getPotX();
        case 0x3:  //POT3 
            return pokey->rightInput->getPotY();
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
            return pokey->KBCODE_LATCH<<1;
        case 0xA:  //RANDOM
            break;
        case 0xD:  //SERIN
            break;
        case 0xE:  //IRQST
            return pokey->IRQST;
        case 0xF:  //SKSTAT
            break;
    }
    //unimplemented or write-only registers
    return 0xFF;
}

