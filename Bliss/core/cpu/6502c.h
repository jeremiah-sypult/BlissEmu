
#ifndef _6502C_H_
#define _6502C_H_

#include "Processor.h"
#include "SignalLine.h"
#include "core/memory/MemoryBus.h"


#define _6502C_PIN_IN_NMI   0
#define _6502C_PIN_IN_HALT  1
#define _6502C_PIN_IN_READY 2

class _6502c : public Processor
{

    public:
        _6502c(MemoryBus*);
        INT32 getClockSpeed();
        void resetProcessor();
        INT32 tick(INT32);
        inline BOOL isIdle() {
            return (pinIn[_6502C_PIN_IN_HALT]->isHigh ||
                    !pinIn[_6502C_PIN_IN_READY]->isHigh);
        }

#ifdef DEVELOPER_VERSION
        UINT32 getProgramCounter();
        UINT32 decode(CHAR description[256], UINT32 memoryLocation);
        void   getRegisters(CHAR op[40]);
#endif

    private:
        UINT16 resetVector;
        UINT16 irqVector;
        UINT16 nmiVector;

        //status bits
        BOOL   N, V, B, D, I, Z, C;

        //registers
        UINT8  AC, XR, YR, SP;

        //program counter
        UINT16 PC;

        //reset address
        UINT32 RESET;

        //the 64K 8-bit memory bus
        MemoryBus* memoryBus;

};

void itoa16(char* c, int i, int digits);

#endif
