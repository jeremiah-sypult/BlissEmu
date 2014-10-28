
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

        inline size_t getImageByteSize() {
            return size * sizeof(UINT16);
        }
        void getImage(void* dst, UINT16 offset, UINT16 size) {
            memcpy(dst, image + offset, size);
        }
        void setImage(void* src, UINT16 offset, UINT16 size) {
            memcpy(image + offset, src, size);
        }

        RAMState getState(UINT16* image);
        void setState(RAMState state, UINT16* image);

    private:
        UINT16    image[GRAM_SIZE];
        BOOL      dirtyCards[GRAM_SIZE>>3];
        BOOL      dirtyRAM;

        static const UINT16 locations[16];
};

#endif
