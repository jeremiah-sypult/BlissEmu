
#ifndef ANTIC_REGISTERS_H
#define ANTIC_REGISTERS_H

#include "core/memory/RAM.h"

class Antic;

class Antic_Registers : public RAM
{

    friend class Antic;

    public:
        UINT16 peek(UINT16 addr);
        void poke(UINT16 addr, UINT16 value);

    private:
        Antic_Registers();
        void init(Antic* antic);

        Antic* antic;

};

#endif

