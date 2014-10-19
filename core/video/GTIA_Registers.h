
#ifndef GTIA_REGISTERS_H
#define GTIA_REGISTERS_H

#include "core/memory/RAM.h"

class GTIA;

class GTIA_Registers : public RAM
{

    friend class GTIA;

    public:
        UINT16 peek(UINT16 addr);
        void poke(UINT16 addr, UINT16 value);

    private:
        GTIA_Registers();
        void init(GTIA* gtia);

        GTIA* gtia;

};

#endif

