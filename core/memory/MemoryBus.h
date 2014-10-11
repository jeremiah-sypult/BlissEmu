
#ifndef MEMORYBUS_H
#define MEMORYBUS_H

#include <stdio.h>
#include <string.h>
#include "Memory.h"

#define MAX_MAPPED_MEMORIES 50
#define MAX_OVERLAPPED_MEMORIES 4

/**
 * Emulates a 64K memory bus which may be composed of 8-bit or 16-bit memory units.
 *
 * @author Kyle Davis
 */
class MemoryBus
{

    public:
        MemoryBus();
        virtual ~MemoryBus();

        void reset();

        UINT16 peek(UINT16 location);
        void poke(UINT16 location, UINT16 value);

        void addMemory(Memory* m);
        void removeMemory(Memory* m);
        void removeAll();

    private:
        Memory*     mappedMemories[MAX_MAPPED_MEMORIES];
        UINT16      mappedMemoryCount;
        UINT16*     writeableMemoryCounts;
        Memory***   writeableMemorySpace;
        UINT16*     readableMemoryCounts;
        Memory***   readableMemorySpace;

};

#endif
