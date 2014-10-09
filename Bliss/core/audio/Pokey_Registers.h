
#ifndef POKEY_REGISTERS_H
#define POKEY_REGISTERS_H

#include "core/memory/RAM.h"

class Pokey;

class Pokey_Registers : public RAM
{

    public:
        Pokey_Registers();
        void init(Pokey* p);

        void poke(UINT16 addr, UINT16 value);
        UINT16 peek(UINT16 addr);

    private:
        Pokey* pokey;
};

#endif
