
#include "GROM.h"

GROM::GROM()
: ROM("GROM", "grom.bin", 0, 1, GROM_SIZE, GROM_ADDRESS)
{}

void GROM::reset()
{
    visible = TRUE;
}

UINT16 GROM::peek(UINT16 location)
{
    return (visible ? ROM::peek(location) : location);
}



