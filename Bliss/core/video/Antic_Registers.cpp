
#include "Antic_Registers.h"
#include "Antic.h"

Antic_Registers::Antic_Registers()
    : antic(NULL)
{}

void Antic_Registers::init(Antic* a)
{
    antic = a;
}

UINT16 Antic_Registers::getReadSize()
{
    return 0x10;
}

UINT16 Antic_Registers::getReadAddress()
{
    return 0xD400;
}

UINT16 Antic_Registers::getReadAddressMask()
{
    return 0xFE0F;
}

UINT16 Antic_Registers::getWriteSize()
{
    return 0x10;
}

UINT16 Antic_Registers::getWriteAddress()
{
    return 0xD400;
}

UINT16 Antic_Registers::getWriteAddressMask()
{
    return 0xFE0F;
}

void Antic_Registers::poke(UINT16 addr, UINT16 value)
{
    switch (addr) {
        case 0x00:  //DMACTL
//cout << "poked DMACTL: " << (int)value << "\n";
            antic->DMACTL = (UINT8)value;
            break;
        case 0x01:  //CHACTL
            antic->CHACTL = (UINT8)value;
            break;
        case 0x02:  //DLISTL
            antic->DLIST = (antic->DLIST & 0xFF00) | value;
            break;
        case 0x03:  //DLISTH
            antic->DLIST = (value << 8) | (antic->DLIST & 0x00FF);
            break;
        case 0x04:  //HSCROL
            antic->HSCROL = (UINT8)value;
            break;
        case 0x05:  //VSCROL
            antic->VSCROL = (UINT8)value;
            break;
        case 0x07:  //PMBASE
            antic->PMBASE = (UINT8)value;
            break;
        case 0x09:  //CHBASE
            antic->CHBASE = (UINT8)value;
            break;
        case 0x0A:  //WSYNC
            antic->pinOut[ANTIC_PIN_OUT_READY]->isHigh = FALSE;
            break;
        case 0x0E:  //NMIEN
            antic->NMIEN = (UINT8)value;
            break;
        case 0x0F:  //NMIRES
            antic->NMIST = 0x1F;
            break;
    }
}

UINT16 Antic_Registers::peek(UINT16 addr)
{
    switch (addr) {
        case 0x00:  //DMACTL
            return antic->DMACTL;
        case 0x01:  //CHACTL
            return antic->CHACTL;
        case 0x02:  //DLISTL
            return (antic->DLIST & 0x00FF);
        case 0x03:  //DLISTH
            return (antic->DLIST & 0xFF00) >> 8;
        case 0x04:  //HSCROL
            return antic->HSCROL;
        case 0x05:  //VSCROL
            return antic->VSCROL;
        case 0x07:  //PMBASE
            return antic->PMBASE;
        case 0x09:  //CHBASE
            return antic->CHBASE;
        case 0x0B:  //VCOUNT/2
            return (antic->VCOUNT) >> 1;
        case 0x0C:  //PENH (unimplemented)
            return 0xFF;
            break;
        case 0x0D:  //PENV (unimplemented)
            return 0xFF;
            break;
        case 0x0E:  //NMIEN
            return antic->NMIEN;
            break;
        case 0x0F:  //NMIST
            return antic->NMIST;
        default:
            //unimplemented or write-only registers
            return 0xFF;
            break;
    }
}

