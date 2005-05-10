
#ifndef ANTIC_REGISTERS_H
#define ANTIC_REGISTERS_H

#include "core/memory/Memory.h"

class Antic_Registers : public Memory
{

    friend class Antic;

    public:
		void reset() {}
        UINT16 getReadSize();
        UINT16 getReadAddress();
        UINT16 getReadAddressMask();

        UINT16 getWriteSize();
        UINT16 getWriteAddress();
        UINT16 getWriteAddressMask();

        UINT16 peek(UINT16 addr);
        void poke(UINT16 addr, UINT16 value);

    private:
        Antic_Registers();
        void init(Antic* antic);

        Antic* antic;

};

#endif

