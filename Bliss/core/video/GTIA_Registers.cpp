
#include "GTIA_Registers.h"
#include "GTIA.h"

UINT16 GTIA_ALIASES[8] = {
    0xC000, 0xC020, 0xC040, 0xC060, 0xC080, 0xC0A0, 0xC0C0, 0xC0E0
};

GTIA_Registers::GTIA_Registers()
{
    gtia = NULL;
}

void GTIA_Registers::init(GTIA* g)
{
    gtia = g;
}

UINT16 GTIA_Registers::getSize()
{
    return 0x20;
}

UINT16 GTIA_Registers::getAddress()
{
    return 0xC000;
}

UINT16 GTIA_Registers::getReadAddressMask()
{
    return 0xFF1F;
}

UINT16 GTIA_Registers::getWriteAddressMask()
{
    return 0xFF1F;
}

void GTIA_Registers::poke(UINT16 addr, UINT16 value)
{
    switch (addr) {
        case 0x00:
            gtia->HPOSP0 = (UINT8)value;
            break;
        case 0x01:
            gtia->HPOSP1 = (UINT8)value;
            break;
        case 0x02:
            gtia->HPOSP2 = (UINT8)value;
            break;
        case 0x03:
            gtia->HPOSP3 = (UINT8)value;
            break;
        case 0x04:
            gtia->HPOSM0 = (UINT8)value;
            break;
        case 0x05:
            gtia->HPOSM1 = (UINT8)value;
            break;
        case 0x06:
            gtia->HPOSM2 = (UINT8)value;
            break;
        case 0x07:
            gtia->HPOSM3 = (UINT8)value;
            break;
        case 0x08:
            gtia->SIZEP0 = (UINT8)value;
            break;
        case 0x09:
            gtia->SIZEP1 = (UINT8)value;
            break;
        case 0x0A:
            gtia->SIZEP2 = (UINT8)value;
            break;
        case 0x0B:
            gtia->SIZEP3 = (UINT8)value;
            break;
        case 0x0C:
            gtia->SIZEM = (UINT8)value;
            break;
        case 0x0D:
            gtia->GRAFP0 = (UINT8)value;
            break;
        case 0x0E:
            gtia->GRAFP1 = (UINT8)value;
            break;
        case 0x0F:
            gtia->GRAFP2 = (UINT8)value;
            break;
        case 0x10:
            gtia->GRAFP3 = (UINT8)value;
            break;
        case 0x11:
            gtia->GRAFM = (UINT8)value;
            break;
        case 0x12:
            gtia->COLPM0 = (UINT8)value;
            break;
        case 0x13:
            gtia->COLPM1 = (UINT8)value;
            break;
        case 0x14:
            gtia->COLPM2 = (UINT8)value;
            break;
        case 0x15:
            gtia->COLPM3 = (UINT8)value;
            break;
        case 0x16:
            gtia->COLPF0 = (UINT8)value;
            break;
        case 0x17:
            gtia->COLPF1 = (UINT8)value;
            break;
        case 0x18:
            gtia->COLPF2 = (UINT8)value;
            break;
        case 0x19:
            gtia->COLPF3 = (UINT8)value;
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
    switch (addr) {
		case 0:				// Suppress 'default only' compiler warning
        default:
            return 0xFF;
    }
}

