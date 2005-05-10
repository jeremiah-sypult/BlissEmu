
#ifndef GTIA_REGISTERS_H
#define GTIA_REGISTERS_H

#include "core/memory/Memory.h"

class GTIA_Registers : public Memory
{

    friend class GTIA;

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
        GTIA_Registers();
        void init(GTIA* gtia);

        GTIA* gtia;

};

#endif

