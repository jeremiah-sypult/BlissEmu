
#include "GTIA.h"

#include <stdio.h>


GTIA::GTIA()
    : Processor("GTIA")
{
    registers.init(this);
}

GTIA_Registers* GTIA::getRegisters()
{
    return &registers;
}

INT32 GTIA::getClockSpeed()
{
    return 1;
}

INT32 GTIA::tick(INT32 minimum)
{
#ifdef _TRACE_TICKS
    fprintf(stderr, "GTIA::tick()\n");
#endif

    return minimum;
}

void GTIA::process(UINT16 location, UINT8 an0, UINT8 an1, UINT8 an2, UINT8 an3)
{
    switch (PRIOR & 0xC0) {
        case 0x00:
        {
            UINT8 an_palette[8] = { COLBK, 0, 0, 0, COLPF0, COLPF1, COLPF2,
                    COLPF3 };
            imageBank[location] = an_palette[an0];
            imageBank[location+1] = an_palette[an1];
            imageBank[location+2] = an_palette[an2];
            imageBank[location+3] = an_palette[an3];
            break;
        }
        case 0x40:
        {
            if (!an0) {
                imageBank[location] = COLBK;
                imageBank[location+1] = COLBK;
                imageBank[location+2] = COLBK;
                imageBank[location+3] = COLBK;
            }
            else {
                UINT8 color = (COLBK | ((an0 & 0x03) << 2) |
                        (an2 & 0x03));
                imageBank[location] = color;
                imageBank[location+1] = color;
                imageBank[location+2] = color;
                imageBank[location+3] = color;
            }
            break;
        }
        case 0x80:
        {
            if (!an0) {
                imageBank[location] = COLBK;
                imageBank[location+1] = COLBK;
                imageBank[location+2] = COLBK;
                imageBank[location+3] = COLBK;
            }
            else {
                UINT8 an_palette[16] = { COLPM0, COLPM1, COLPM2, COLPM3,
                        COLPF0, COLPF1, COLPF2, COLPF3,
                        COLPF0, COLPF1, COLPF2, COLPF3,
                        COLBK, COLBK, COLBK, COLBK };
                UINT8 data = ((an0 & 0x03) << 2) | (an2 & 0x03);
                UINT8 color = an_palette[data];
                imageBank[location] = color;
                imageBank[location+1] = color;
                imageBank[location+2] = color;
                imageBank[location+3] = color;
            }
            break;
        }
        case 0xC0:
        {
            if (!an0) {
                imageBank[location] = COLBK;
                imageBank[location+1] = COLBK;
                imageBank[location+2] = COLBK;
                imageBank[location+3] = COLBK;
            }
            else {
                UINT8 color = (COLBK | ((an0 & 0x03) << 6) |
                        ((an2 & 0x03) << 4));
                imageBank[location] = color;
                imageBank[location+1] = color;
                imageBank[location+2] = color;
                imageBank[location+3] = color;
            }
            break;
        }
    }
}
