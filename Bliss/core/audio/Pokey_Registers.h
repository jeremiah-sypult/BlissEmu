
#ifndef POKEY_REGISTERS_H
#define POKEY_REGISTERS_H

#include "core/memory/Memory.h"

class Pokey;

class Pokey_Registers : Memory
{

    public:
        Pokey_Registers();
        void init(Pokey* p);
        UINT16 getSize();
        UINT16 getNumLocations();
        UINT16 getLocation(UINT16 i);
        BOOL isLocationReadable(UINT16 i);
        BOOL isLocationWriteable(UINT16 i);
        void poke(UINT16 addr, UINT16 value);
        UINT16 peek(UINT16 addr);

    private:
        Pokey* pokey;
};

#endif
