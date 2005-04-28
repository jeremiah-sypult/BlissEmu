
#ifndef RAM_H
#define RAM_H

#include "Memory.h"

class RAM : public Memory
{

    public:
        RAM(UINT16 size, UINT16 location);
        RAM(UINT16 size, UINT16 location, UINT8 bitWidth);
        RAM(UINT16 size, UINT16 location, UINT16 readAddressMask, UINT16 writeAddressMask);
        RAM(UINT16 size, UINT16 location, UINT16 readAddressMask, UINT16 writeAddressMask, UINT8 bitWidth);
        virtual ~RAM();

        virtual void reset();
        UINT16 getSize();
        UINT16 getAddress();
        UINT16 getReadAddressMask();
        UINT16 getWriteAddressMask();
        UINT8  getBitWidth();

        virtual UINT16 peek(UINT16 location);
        virtual void poke(UINT16 location, UINT16 value);

    protected:
        UINT16  size;
        UINT16  location;
        UINT16  readAddressMask;
        UINT16  writeAddressMask;

    private:
        UINT8   bitWidth;
        UINT16  trimmer;
        UINT16* image;

};

#endif
