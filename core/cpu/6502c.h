
#ifndef _6502C_H_
#define _6502C_H_

#include "Processor.h"
#include "SignalLine.h"
#include "core/memory/MemoryBus.h"

#define _6502C_PIN_IN_NMI   0
#define _6502C_PIN_IN_HALT  1
#define _6502C_PIN_IN_READY 2
#define _6502C_PIN_IN_IRQ   3

class _6502c : public Processor
{
public:
    _6502c(MemoryBus* memoryBus);
    
    INT32 getClockSpeed() { return 1792080; }
    
    void resetProcessor();
    
    INT32 tick(INT32 minimum);

private:
    const static UINT16 resetVector;
    const static UINT16 irqVector;
    const static UINT16 nmiVector;

    //status bits
    BOOL N, V, B, D, I, Z, C;

    //registers
    UINT8  AC, XR, YR, SP;

    //program counter
    UINT16 PC;

    //the 64K 8-bit memory bus
    MemoryBus* memoryBus;

    UINT8 MERGE_SR();
    void SPLIT_SR(UINT8 x);
    void PUSH(UINT8 x);
    void dPUSH(UINT16 x);
    UINT8 POP();
    UINT16 dPOP();
    void BRANCH(INT8 x);
    inline UINT16 IMMEDIATE();
    inline UINT16 ZERO_PAGE();
    inline UINT16 ZERO_PAGE_X();
    inline UINT16 ZERO_PAGE_Y();
    inline UINT16 Absolute();
    inline UINT16 Absolute_X();
    inline UINT16 Absolute_Y();
    inline UINT16 INDIRECT_X();
    inline UINT16 INDIRECT_Y();
    inline void INC(UINT16 addr);
    inline void DEC(UINT16 addr);
    inline void SLO(UINT16 addr);
    inline void ASL(UINT16 addr);
    inline void AAC(UINT16 addr);
    inline void ASR(UINT16 addr);
    inline void LSL(UINT16 addr);
    inline void RLA(UINT16 addr);
    inline void ROL(UINT16 addr);
    inline void SRE(UINT16 addr);
    inline void LSR(UINT16 addr);
    inline void ROR(UINT16 addr);
    inline void ALR(UINT16 addr);
    inline void RRA(UINT16 addr);
    inline void BIT(UINT16 addr);
    inline void ADC(UINT16 addr);
    inline void SBC(UINT16 addr);
    inline void CMP(UINT16 addr);
    inline void CPY(UINT16 addr);
    inline void CPX(UINT16 addr);
    inline void DCP(UINT16 addr);
    inline void ISC(UINT16 addr);
    inline void bADC(UINT16 x);
    inline void dADC(UINT16 x);
    inline void bSBC(UINT16 x);
    inline void dSBC(UINT16 x);
};

#endif

/*
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

void itoa16(char* c, INT32 i, INT32 digits);
*/
