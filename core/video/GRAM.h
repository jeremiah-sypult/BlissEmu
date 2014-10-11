
#ifndef GRAM_H
#define GRAM_H

#include "core/memory/RAM.h"

#define GRAM_SIZE       0x0200

class GRAM : public RAM
{

    friend class AY38900;

    public:
        GRAM();

        void reset();
        UINT16 peek(UINT16 location);
        void poke(UINT16 location, UINT16 value);

        void markClean();
        BOOL isDirty();
        BOOL isCardDirty(UINT16 cardLocation);

    private:
        UINT16    image[GRAM_SIZE];
        BOOL      dirtyCards[GRAM_SIZE>>3];
        BOOL      dirtyRAM;

        static const UINT16 locations[16];

};

#endif

