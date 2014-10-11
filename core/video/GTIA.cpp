
#include "GTIA.h"

#include <stdio.h>


GTIA::GTIA()
    : Processor("GTIA")
{
    registers.init(this);
}

void GTIA::resetProcessor()
{
    memset(HPOSP, 0, sizeof(HPOSP));
    memset(HPOSM, 0, sizeof(HPOSM));
    memset(SIZEP, 0, sizeof(SIZEP));
    SIZEM = 0;
    memset(GRAFP, 0, sizeof(GRAFP));
    GRAFM = 0;
    memset(COLPM, 0, sizeof(COLPM));
    memset(COLPF, 0, sizeof(COLPF));
    COLBK = 0;
    PRIOR = 0;
    VDELAY = 0;
    GRACTL = 0;
    memset(MPF, 0, sizeof(MPF));
    memset(PPL, 0, sizeof(PPF));
    memset(MPF, 0, sizeof(MPL));
    memset(PPL, 0, sizeof(PPL));
    memset(TRIG, 0, sizeof(TRIG));
    CONSOL = 0;
}

/* GTIA modes are not yet supported for performanc reasons
void GTIA::process(UINT16 location, UINT8 an0, UINT8 color0, UINT8 an1, UINT8 color1)
{
    switch (PRIOR & 0xC0) {
        case 0x00:
        {
            imageBank[location] = color0;
            imageBank[location+1] = color1;
            break;
        }
        case 0x40:
        {
            UINT8 color = COLBK;
            if ((an0 & 0x04) == 0)
                color |=  (an0 & 0x03) << 2;
            if ((an1 & 0x04) == 0)
                color |= (an1 & 0x03);
            imageBank[location] = color;
            imageBank[location+1] = color;
            break;
        }
        case 0x80:
        {
            if ((an0 & 0x04) == 0) {
                imageBank[location] = COLBK;
                imageBank[location+1] = COLBK;
            }
            else {
                UINT8 an_palette[16] = { COLPM[0], COLPM[1], COLPM[2], COLPM[3],
                        COLPF[0], COLPF[1], COLPF[2], COLPF[3],
                        COLBK, COLBK, COLBK, COLBK,
                        COLPF[0], COLPF[1], COLPF[2], COLPF[3] };
                UINT8 color = an_palette[((an0 & 0x03) << 2) | (an1 & 0x03)];
                imageBank[location] = color;
                imageBank[location+1] = color;
            }
            break;
        }
        case 0xC0:
        {
            UINT8 color = COLBK;
            if ((an0 & 0x04) == 0)
                color |= (an0 & 0x03) << 6;
            if ((an1 & 0x04) == 0)
                color |= (an1 & 0x03) << 4;
            imageBank[location] = color;
            imageBank[location+1] = color;
            break;
        }
    }
}
*/
