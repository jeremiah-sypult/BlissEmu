
#ifndef GROM_H
#define GROM_H

#include "core/memory/Memory.h"
#include "core/memory/ROM.h"

#define GROM_SIZE 0x0800
#define GROM_ADDRESS 0x3000

class GROM : public ROM
{

    friend class AY38900;

    public:
        GROM();

        void reset();
        UINT16 peek(UINT16 location);

    private:
        BOOL         visible;

};

#endif
