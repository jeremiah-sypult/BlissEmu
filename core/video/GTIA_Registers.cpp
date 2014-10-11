
#include "GTIA_Registers.h"
#include "GTIA.h"

GTIA_Registers::GTIA_Registers()
: RAM(0x20, 0xC000, 0xFF1F, 0xFF1F, 8)
{
    gtia = NULL;
}

void GTIA_Registers::init(GTIA* g)
{
    gtia = g;
}   

void GTIA_Registers::poke(UINT16 addr, UINT16 value)
{
    switch (addr & 0x1F) {
        case 0x00:
            gtia->HPOSP[0] = (UINT8)value;
            break;
        case 0x01:
            gtia->HPOSP[1] = (UINT8)value;
            break;
        case 0x02:
            gtia->HPOSP[2] = (UINT8)value;
            break;
        case 0x03:
            gtia->HPOSP[3] = (UINT8)value;
            break;
        case 0x04:
            gtia->HPOSM[0] = (UINT8)value;
            break;
        case 0x05:
            gtia->HPOSM[1] = (UINT8)value;
            break;
        case 0x06:
            gtia->HPOSM[2] = (UINT8)value;
            break;
        case 0x07:
            gtia->HPOSM[3] = (UINT8)value;
            break;
        case 0x08:
            gtia->SIZEP[0] = (UINT8)value;
            break;
        case 0x09:
            gtia->SIZEP[1] = (UINT8)value;
            break;
        case 0x0A:
            gtia->SIZEP[2] = (UINT8)value;
            break;
        case 0x0B:
            gtia->SIZEP[3] = (UINT8)value;
            break;
        case 0x0C:
            gtia->SIZEM = (UINT8)value;
            break;
        case 0x0D:
            gtia->GRAFP[0] = (UINT8)value;
            break;
        case 0x0E:
            gtia->GRAFP[1] = (UINT8)value;
            break;
        case 0x0F:
            gtia->GRAFP[2] = (UINT8)value;
            break;
        case 0x10:
            gtia->GRAFP[3] = (UINT8)value;
            break;
        case 0x11:
            gtia->GRAFM = (UINT8)value;
            break;
        case 0x12:
            gtia->COLPM[0] = (UINT8)value;
            break;
        case 0x13:
            gtia->COLPM[1] = (UINT8)value;
            break;
        case 0x14:
            gtia->COLPM[2] = (UINT8)value;
            break;
        case 0x15:
            gtia->COLPM[3] = (UINT8)value;
            break;
        case 0x16:
            gtia->COLPF[0] = (UINT8)value;
            break;
        case 0x17:
            gtia->COLPF[1] = (UINT8)value;
            break;
        case 0x18:
            gtia->COLPF[2] = (UINT8)value;
            break;
        case 0x19:
            gtia->COLPF[3] = (UINT8)value;
            break;
        case 0x1A:
            gtia->COLBK = (UINT8)value;
            break;
        case 0x1B:
            gtia->PRIOR = (UINT8)value;
            break;
        case 0x1C:
            gtia->VDELAY = (UINT8)value;
            break;
        case 0x1D:
            gtia->GRACTL = (UINT8)value;
            break;
        case 0x1E:
            //TODO: clear the collision registers
            break;
        case 0x1F:
            gtia->CONSOL = (UINT8)value;
            break;
    }
}

UINT16 GTIA_Registers::peek(UINT16 addr)
{
    switch (addr & 0x1F) {
        case 0x00:
            return gtia->MPF[0];
        case 0x01:
            return gtia->MPF[1];
        case 0x02:
            return gtia->MPF[2];
        case 0x03:
            return gtia->MPF[3];
        case 0x04:
            return gtia->PPF[0];
        case 0x05:
            return gtia->PPF[1];
        case 0x06:
            return gtia->PPF[2];
        case 0x07:
            return gtia->PPF[3];
        case 0x08:
            return gtia->MPL[0];
        case 0x09:
            return gtia->MPL[1];
        case 0x0A:
            return gtia->MPL[2];
        case 0x0B:
            return gtia->MPL[3];
        case 0x0C:
            return gtia->PPL[0];
        case 0x0D:
            return gtia->PPL[1];
        case 0x0E:
            return gtia->PPL[2];
        case 0x0F:
            return gtia->PPL[3];
        case 0x10:
            return gtia->TRIG[0];
        case 0x11:
            return gtia->TRIG[1];
        case 0x12:
            return gtia->TRIG[2];
        case 0x13:
            return gtia->TRIG[3];
        case 0x14:
            return 0x0E;
        case 0x15:
            return 0;
        case 0x16:
            return 0;
        case 0x17:
            return 0;
        case 0x18:
            return 0;
        case 0x19:
            return 0;
        case 0x1A:
            return 0;
        case 0x1B:
            return 0;
        case 0x1C:
            return 0;
        case 0x1D:
            return 0;
        case 0x1E:
            return 0;
        case 0x1F:
            return gtia->CONSOL;
        default:
            return 0xFF;
    }
}

