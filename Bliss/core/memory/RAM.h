
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
        UINT8  getBitWidth();

        UINT16 getReadSize();
        UINT16 getReadAddress();
        UINT16 getReadAddressMask();
        virtual UINT16 peek(UINT16 location);

        UINT16 getWriteSize();
        UINT16 getWriteAddress();
        UINT16 getWriteAddressMask();
        virtual void poke(UINT16 location, UINT16 value);

        //enabled attributes
        void SetEnabled(BOOL b);
        BOOL IsEnabled() { return enabled; }

    protected:
        BOOL    enabled;
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
