
#include "Antic_Registers.h"
#include "Antic.h"

Antic_Registers::Antic_Registers()
    : RAM(0x10, 0xD400, 0xFE0F, 0xFE0F, 8)
{}

void Antic_Registers::init(Antic* a)
{
    antic = a;
}

void Antic_Registers::poke(UINT16 addr, UINT16 value)
{
    switch (addr & 0xF) {
        case 0x00:  //DMACTL
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
    switch (addr & 0xF) {
        case 0x0B:  //VCOUNT/2
            return (antic->VCOUNT) >> 1;
        case 0x0C:  //PENH (unimplemented)
            return 0xFF;
            break;
        case 0x0D:  //PENV (unimplemented)
            return 0xFF;
            break;
        case 0x0F:  //NMIST
            return antic->NMIST;
        default:
            //unimplemented or write-only registers
            return 0xFF;
            break;
    }
}

