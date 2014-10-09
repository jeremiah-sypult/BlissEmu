
#include <string.h>
#include "6502c.h"
#include "core/types.h"
#include "core/memory/MemoryBus.h"

const UINT16 _6502c::resetVector = 0xFFFC;
const UINT16 _6502c::irqVector = 0xFFFE;
const UINT16 _6502c::nmiVector = 0xFFFA;

#define PEEK(x)          (UINT8)memoryBus->peek(x)
#define POKE(x, y)       memoryBus->poke(x, (UINT8)y)
#define NZ(val)          N = (val & 0x80) != 0; Z = (val == 0)


_6502c::_6502c(MemoryBus* mb)
: Processor("6502c"),
  memoryBus(mb)
{}

void _6502c::resetProcessor()
{
    N = V = B = D = I = Z = C = FALSE;
    AC = XR = YR = SP = 0;
    PC = (UINT16)(PEEK(resetVector) | PEEK((UINT16)(resetVector+1)) << 8);
}

INT32 _6502c::tick(INT32 minimum)
{
    INT32 usedCycles = 0;
    do {

    if (!pinIn[_6502C_PIN_IN_HALT]->isHigh || !pinIn[_6502C_PIN_IN_READY]->isHigh)
        return minimum;

    if (!pinIn[_6502C_PIN_IN_NMI]->isHigh) 
    {
        pinIn[_6502C_PIN_IN_NMI]->isHigh = TRUE;
        dPUSH(PC);
        UINT8 SR = MERGE_SR();
        PUSH(SR);
        PC = (UINT16)(PEEK(nmiVector) | PEEK((UINT16)(nmiVector+1)) << 8);
    }

    if (!I && !pinIn[_6502C_PIN_IN_IRQ]->isHigh) {
        I = TRUE;
        pinIn[_6502C_PIN_IN_IRQ]->isHigh = TRUE;
        dPUSH(PC);
        UINT8 SR = MERGE_SR();
        PUSH(SR);
        PC = (UINT16)(PEEK(irqVector) | PEEK((UINT16)(irqVector+1)) << 8);
    }
    
    UINT8 op = PEEK(PC);
    PC++;
    switch (op) 
    {
        default:
        case 0x00:  //---- 7  BRK
        {
            PC += 2;
            if (!I) {
                B = TRUE;
                dPUSH(PC);
                UINT8 SR = MERGE_SR();
                PUSH(SR);
                PC = (UINT16)(PEEK(irqVector) | PEEK((UINT16)(irqVector+1)) << 8);
            }
            usedCycles += 7;
        } break;
        case 0x01:  //--NZ 6  ORA ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 6;
        } break;
        case 0x02:  //---- -  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x03:  //C-NZ 8  SLO ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            SLO(addr);
            usedCycles += 8;
        } break;
        case 0x04:  //---- 3  DOP $44
        {
            PC++;
            usedCycles += 3;
        } break;
        case 0x05:  //--NZ 2  ORA $44
        {
            UINT16 addr = ZERO_PAGE();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x06:  //C-NZ 5  ASL $44
        {
            UINT16 addr = ZERO_PAGE();
            ASL(addr);
            usedCycles += 5;
        } break;
        case 0x07:  //C-NZ 5  SLO $44
        {
            UINT16 addr = ZERO_PAGE();
            SLO(addr);
            usedCycles += 5;
        } break;
        case 0x08:  //---- 3  PHP
        {
            UINT8 sr = MERGE_SR();
            PUSH(sr);
            usedCycles += 3;
        } break;
        case 0x09:  //--NZ 2  ORA #$44
        {
            UINT16 addr = IMMEDIATE();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x0A:  //C-NZ 2  ASL A
        {
            C = !!(AC & 0x80);
            AC <<= 1;
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x0B:  //C-NZ 2  AAC #$44
        {
            UINT16 addr = IMMEDIATE();
            AAC(addr);
            usedCycles += 2;
        } break;
        case 0x0C:  //---- 4  TOP $4400
        {
            PC += 2;
            usedCycles += 4;
        } break;
        case 0x0D:  //--NZ 4  ORA $4400
        {
            UINT16 addr = Absolute();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x0E:  //C-NZ 6  ASL $4400
        {
            UINT16 addr = Absolute();
            ASL(addr);
            usedCycles += 6;
        } break;
        case 0x0F:  //C-NZ 6  SLO $4400
        {
            UINT16 addr = Absolute();
            SLO(addr);
            usedCycles += 6;
        } break;
        case 0x10:  //---- 2|3+ BPL
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (!N) {
                usedCycles++;
                //TODO: add an extra cycle if we branch across a page boundary
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0x11:  //--NZ 5+ ORA ($44),Y;
        {
            UINT16 addr = INDIRECT_Y();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 5;
        } break;
        case 0x12:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x13:  //C-NZ 8  SLO ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            SLO(addr);
            usedCycles += 8;
        } break;
        case 0x14:  //---- 4  DOP
        {
            PC++;
            usedCycles += 4;
        } break;
        case 0x15:  //ORA $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 3;
        } break;
        case 0x16:  //C-NZ 6  ASL $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            ASL(addr);
            usedCycles += 6;
        } break;
        case 0x17:  //C-NZ 6  SLO $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            SLO(addr);
            usedCycles += 6;
        } break;
        case 0x18:  //---- 2  CLC
        {
            C = FALSE;
            usedCycles += 2;
        } break;
        case 0x19:  //--NZ 4+ ORA $4400,Y
        {
            UINT16 addr = Absolute_Y();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x1A:  //NOP
        {
            usedCycles += 2;
        } break;
        case 0x1B:  //C-NZ 7  SLO $4400,Y
        {
            UINT16 addr = Absolute_Y();
            SLO(addr);
            usedCycles += 7;
        } break;
        case 0x1C:  //TOP
        {
            PC += 2;
            usedCycles += 4;
        } break;
        case 0x1D:  //--NZ 4+ ORA $4400,X
        {
            UINT16 addr = Absolute_X();
            AC |= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x1E:  //C-NZ 7  ASL $4400,X
        {
            UINT16 addr = Absolute_X();
            ASL(addr);
            usedCycles += 7;
        } break;
        case 0x1F:  //C-NZ 7 SLO $4400,X
        {
            UINT16 addr = Absolute_X();
            SLO(addr);
            usedCycles += 7;
        } break;
        case 0x20:  //---- 6  JSR $4400
        {
            UINT16 addr = (UINT16)(PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8));
            PC += 2;
            dPUSH(PC);
            PC = addr;
            usedCycles += 6;
        } break;
        case 0x21:  //AND ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 6;
        } break;
        case 0x22:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x23:  //C-NZ 8  RLA ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            RLA(addr);
            usedCycles += 8;
        } break;
        case 0x24:  //-VNZ 3  BIT $44
        {
            UINT16 addr = ZERO_PAGE();
            BIT(addr);
            usedCycles += 3;
        } break;
        case 0x25:  //--NZ 2  AND $44
        {
            UINT16 addr = ZERO_PAGE();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x26:  //C-NZ 5  ROL $44
        {
            UINT16 addr = ZERO_PAGE();
            ROL(addr);
            usedCycles += 5;
        } break;
        case 0x27:  //C-NZ 5  RLA $44
        {
            UINT16 addr = ZERO_PAGE();
            RLA(addr);
            usedCycles += 5;
        } break;
        case 0x28:  //CVNZ 4  PLP
        {
            UINT8 sr = POP();
            SPLIT_SR(sr);
            usedCycles += 4;
        } break;
        case 0x29:  //--NZ 2  AND #$44
        {
            UINT16 addr = IMMEDIATE();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x2A:  //C-NZ 2  ROL A
        {
            BOOL c = C;
            C = !!(AC & 0x80);
            AC = (UINT8)((AC<<1) | (c ? 1 : 0));
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x2B:  //AAC #$44
        {
            UINT16 addr = IMMEDIATE();
            AAC(addr);
            usedCycles += 2;
        } break;
        case 0x2C:  //-VNZ 4  BIT $4400
        {
            UINT16 addr = Absolute();
            BIT(addr);
            usedCycles += 4;
        } break;
        case 0x2D:  //--NZ 4  AND $4400
        {
            UINT16 addr = Absolute();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x2E:  //C-NZ 6  ROL $4400
        {
            UINT16 addr = Absolute();
            ROL(addr);
            usedCycles += 6;
        } break;
        case 0x2F:  //C-NZ 6  RLA $4400
        {
            UINT16 addr = Absolute();
            RLA(addr);
            usedCycles += 6;
        } break;
        case 0x30:  //BMI
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (N) {
                usedCycles++;
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0x31:  //--NZ 5+ AND ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 5;
        } break;
        case 0x32:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x33:  //C-NZ 8  RLA ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            RLA(addr);
            usedCycles += 8;
        } break;
        case 0x34:  //---- 4  DOP
        {
            PC++;
            usedCycles += 4;
        } break;
        case 0x35:  //--NZ 3  AND $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 3;
        } break;
        case 0x36:  //C-NZ 6  ROL $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            ROL(addr);
            usedCycles += 6;
        } break;
        case 0x37:  //C-NZ 6  RLA $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            RLA(addr);
            usedCycles += 6;
        } break;
        case 0x38:  //C--- 2  SEC
        {
            C = TRUE;
            usedCycles += 2;
        } break;
        case 0x39:  //--NZ 4+ AND $4400,Y
        {
            UINT16 addr = Absolute_Y();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x3A:  //NOP
        {
            usedCycles += 2;
        } break;
        case 0x3B:  //C-NZ 7  RLA $4400,Y
        {
            UINT16 addr = Absolute_Y();
            RLA(addr);
            usedCycles += 7;
        } break;
        case 0x3C:  //---- 4+ TOP
        {
            PC += 2;
            usedCycles += 4;
        } break;
        case 0x3D:  //--NZ 4+ AND $4400,X
        {
            UINT16 addr = Absolute_X();
            AC &= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x3E:  //C-NZ 7  ROL $4400,X
        {
            UINT16 addr = Absolute_X();
            ROL(addr);
            usedCycles += 7;
        } break;
        case 0x3F:  //C-NZ 7  RLA $4400,X
        {
            UINT16 addr = Absolute_X();
            RLA(addr);
            usedCycles += 7;
        } break;
        case 0x40:  //CVNZ 6  RTI
        {
            UINT8 sr = POP();
            SPLIT_SR(sr);
            PC = dPOP();
            usedCycles += 6;
        } break;
        case 0x41:  //--NZ 6  EOR ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 6;
        } break;
        case 0x42:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x43:  //C-NZ 8  SRE ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            SRE(addr);
            usedCycles += 8;
        } break;
        case 0x44:  //---- 3  DOP
        {
            PC++;
            usedCycles += 3;
        } break;
        case 0x45:  //--NZ 3  EOR $44
        {
            UINT16 addr = ZERO_PAGE();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 3;
        } break;
        case 0x46:  //C-NZ 5  LSR $44
        {
            UINT16 addr = ZERO_PAGE();
            LSR(addr);
            usedCycles += 5;
        } break;
        case 0x47:  //C-NZ 5  SRE $44
        {
            UINT16 addr = ZERO_PAGE();
            SRE(addr);
            usedCycles += 5;
        } break;
        case 0x48:  //---- 3  PHA
        {
            PUSH(AC);
            usedCycles += 3;
        } break;
        case 0x49:  //--NZ 2  EOR #$44
        {
            UINT16 addr = IMMEDIATE();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x4A:  //C-NZ 2  LSR A
        {
            C = (AC & 0x01);
            AC >>= 1;
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x4B:  //C-NZ 2  ALR #$44
        {
            UINT16 addr = IMMEDIATE();
            ALR(addr);
            usedCycles += 2;
        } break;
        case 0x4C:  //---- 3  JMP $4400
        {
            PC = (UINT16)(PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8));
            usedCycles += 3;
        } break;
        case 0x4D:  //--NZ 4  EOR $4400
        {
            UINT16 addr = Absolute();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x4E:  //C-NZ 6  LSR $4400
        {
            UINT16 addr = Absolute();
            LSR(addr);
            usedCycles += 6;
        } break;
        case 0x4F:  //C-NZ 6  SRE $4400
        {
            UINT16 addr = Absolute();
            SRE(addr);
            usedCycles += 6;
        } break;
        case 0x50:  //---- 2+ BVC
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (!V) {
                usedCycles++;
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0x51:  //--NZ 5+ EOR ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 5;
        } break;
        case 0x52:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x53:  //C-NZ 8  SRE ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            SRE(addr);
            usedCycles += 8;
        } break;
        case 0x54:  //---- 4  DOP
        {
            PC++;
            usedCycles += 4;
        } break;
        case 0x55:  //--NZ 4  EOR $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x56:  //C-NZ 6  LSR $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            LSR(addr);
            usedCycles += 6;
        } break;
        case 0x57:  //C-NZ 6  SRE $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            SRE(addr);
            usedCycles += 6;
        } break;
        case 0x58:  //CLI
        {
            I = FALSE;
            usedCycles += 2;
        } break;
        case 0x59:  //--NZ 4+ EOR $4400,Y
        {
            UINT16 addr = Absolute_Y();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x5A:  //---- 2  NOP
        {
            usedCycles += 2;
        } break;
        case 0x5B:  //C-NZ 7  SRE $4400,Y
        {
            UINT16 addr = Absolute_Y();
            SRE(addr);
            usedCycles += 7;
        } break;
        case 0x5C:  //---- 4+ TOP
        {
            PC += 2;
            usedCycles += 4;
        } break;
        case 0x5D:  //--NZ 4+ EOR $4400,X
        {
            UINT16 addr = Absolute_X();
            AC ^= PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x5E:  //C-NZ 7  LSR $4400,X
        {
            UINT16 addr = Absolute_X();
            LSR(addr);
            usedCycles += 7;
        } break;
        case 0x5F:  //C-NZ 7  SRE $4400,X
        {
            UINT16 addr = Absolute_X();
            SRE(addr);
            usedCycles += 7;
        } break;
        case 0x60:  //---- 6  RTS
        {
            PC = dPOP();
            usedCycles += 6;
        } break;
        case 0x61:  //CVNZ 6  ADC ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            ADC(addr);
            usedCycles += 6;
        } break;
        case 0x62:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x63:  //RRA ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            RRA(addr);
            usedCycles += 8;
        } break;
        case 0x64:  //---- 3  DOP
        {
            PC++;
            usedCycles += 3;
        } break;
        case 0x65:  //CVNZ 3  ADC $44
        {
            UINT16 addr = ZERO_PAGE();
            ADC(addr);
            usedCycles += 3;
        } break;
        case 0x66:  //C-NZ 5  ROR $44
        {
            UINT16 addr = ZERO_PAGE();
            ROR(addr);
            usedCycles += 5;
        } break;
        case 0x67:  //RRA $44
        {
            UINT16 addr = ZERO_PAGE();
            RRA(addr);
            usedCycles += 5;
        } break;
        case 0x68:  //--NZ 4  PLA
        {
            AC = POP();
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0x69:  //CVNZ 2  ADC #$44
        {
            UINT16 addr = IMMEDIATE();
            ADC(addr);
            usedCycles += 2;
        } break;
        case 0x6A:  //C-NZ 2  ROR A
        {
            UINT8 c = (C ? 0x80 : 0);
            C = (AC & 0x01);
            AC = (UINT8)((AC>>1) | c);
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x6B:  //ARR #$44
        {
            UINT16 addr = IMMEDIATE();
            UINT8 data = AC & PEEK(addr);
            if (D) {
                UINT16 tmp = (data >> 1) | (C ? 0x80 : 0);
                NZ(tmp);
                V = !!((tmp ^ data) & 0x40);
		        if (((data & 0x0F) + (data & 0x01)) > 5)
			        tmp = (tmp & 0xF0) | ((tmp + 0x6) & 0x0F);

		        if (((data & 0xF0) + (data & 0x10)) > 0x50) {
			        tmp = (tmp & 0x0F) | ((tmp + 0x60) & 0xF0);
			        C = TRUE;
		        }
		        else
			        C = FALSE;
		        AC = (UINT8)tmp;
            }
            else {
                AC = (data >> 1) | (C ? 0x80 : 0);
                NZ(AC);
			    C = !!(AC & 0x40);
			    V = ((AC >> 6) ^ (AC >> 5)) & 1;
            }
            usedCycles += 2;
        } break;
        case 0x6C:  //---- 5  JMP ($4400)
        {
            UINT16 addr = (UINT16)(PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8));
            PC = (UINT16)(PEEK(addr) | (PEEK((UINT16)(addr+1)) << 8));
            usedCycles += 5;
        } break;
        case 0x6D:  //CVNZ 4  ADC $4400
        {
            UINT16 addr = Absolute();
            ADC(addr);
            usedCycles += 4;
        } break;
        case 0x6E:  //C-NZ 6  ROR $4400
        {
            UINT16 addr = Absolute();
            ROR(addr);
            usedCycles += 6;
        } break;
        case 0x6F:  //RRA $4400
        {
            UINT16 addr = Absolute();
            RRA(addr);
            usedCycles += 6;
        } break;
        case 0x70:  //BVS
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (V) {
                usedCycles++;
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0x71:  //CVNZ 5+ ADC ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            ADC(addr);
            usedCycles += 5;
        } break;
        case 0x72:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x73:  //CVNZ 8  RRA ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            RRA(addr);
            usedCycles += 8;
        } break;
        case 0x74:  //---- 4  DOP
        {
            PC++;
            usedCycles += 4;
        } break;
        case 0x75:  //CVNZ 4  ADC $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            ADC(addr);
            usedCycles += 4;
        } break;
        case 0x76:  //C-NZ 6  ROR $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            ROR(addr);
            usedCycles += 6;
        } break;
        case 0x77:  //CVNZ 6  RRA $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            RRA(addr);
            usedCycles += 6;
        } break;
        case 0x78:  //SEI
        {
            I = TRUE;
            usedCycles += 2;
        } break;
        case 0x79:  //CVNZ 4+ ADC $4400,Y
        {
            UINT16 addr = Absolute_Y();
            ADC(addr);
            usedCycles += 4;
        } break;
        case 0x7A:  //NOP
        {
            usedCycles += 2;
        } break;
        case 0x7B:  //CVNZ 7  RRA $4400,Y
        {
            UINT16 addr = Absolute_Y();
            RRA(addr);
            usedCycles += 7;
        } break;
        case 0x7C:  //---- 4+ TOP
        {
            PC += 2;
            usedCycles += 4;
        } break;
        case 0x7D:  //CVNZ 4+ ADC $4400,X
        {
            UINT16 addr = Absolute_X();
            ADC(addr);
            usedCycles += 4;
        } break;
        case 0x7E:  //C-NZ 7  ROR $4400,X
        {
            UINT16 addr = Absolute_X();
            ROR(addr);
            usedCycles += 7;
        } break;
        case 0x7F:  //RRA $4400,X
        {
            UINT16 addr = Absolute_X();
            RRA(addr);
            usedCycles += 7;
        } break;
        case 0x80:  //---- 2 DOP
        {
            PC++;
            usedCycles += 2;
        } break;
        case 0x81:  //---- 6  STA ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            POKE(addr, AC);
            usedCycles += 6;
        } break;
        case 0x82:  //---- 2  DOP
        {
            PC++;
            usedCycles += 2;
        } break;
        case 0x83:  //---- 6  AAX ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            UINT8 val = (UINT8)(AC & XR);
            POKE(addr, val);
            usedCycles += 6;
        } break;
        case 0x84:  //---- 3  STY $44
        {
            UINT16 addr = ZERO_PAGE();
            POKE(addr, YR);
            usedCycles += 3;
        } break;
        case 0x85:  //---- 3  STA $44
        {
            UINT16 addr = ZERO_PAGE();
            POKE(addr, AC);
            usedCycles += 3;
        } break;
        case 0x86:  //---- 3  STX $44
        {
            UINT16 addr = ZERO_PAGE();
            POKE(addr, XR);
            usedCycles += 3;
        } break;
        case 0x87:  //---- 3  AAX $44
        {
            UINT16 addr = ZERO_PAGE();
            UINT8 val = (UINT8)(AC & XR);
            POKE(addr, val);
            usedCycles += 3;
        } break;
        case 0x88:  //--NZ 2  DEY
        {
            YR--;
            NZ(YR);
            usedCycles += 2;
        } break;
        case 0x89:  //---- 2  DOP
        {
            PC++;
            usedCycles += 2;
        } break;
        case 0x8A:  //--NZ 2  TXA
        {
            AC = XR;
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x8B:  //--NZ 2  XAA #$44
        {
            UINT16 addr = IMMEDIATE();
            UINT8 data = PEEK(addr);
		    UINT8 tmp = AC & XR & data;
            NZ(tmp);
		    AC &= XR & (data | 0xEF);
            usedCycles += 2;
        } break;
        case 0x8C:  //---- 4  STY $4400
        {
            UINT16 addr = Absolute();
            POKE(addr, YR);
            usedCycles += 4;
        } break;
        case 0x8D:  //---- 4  STA $4400
        {
            UINT16 addr = Absolute();
            POKE(addr, AC);
            usedCycles += 4;
        } break;
        case 0x8E:  //---- 4  STX $4400
        {
            UINT16 addr = Absolute();
            POKE(addr, XR);
            usedCycles += 4;
        } break;
        case 0x8F:  //---- 4  AAX $4400
        {
            UINT16 addr = Absolute();
            UINT8 val = (UINT8)(AC & XR);
            POKE(addr, val);
            usedCycles += 4;
        } break;
        case 0x90:  //---- 2+ BCC
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (!C) {
                usedCycles++;
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0x91:  //---- 6  STA ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            POKE(addr, AC);
            usedCycles += 6;
        } break;
        case 0x92:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0x93:  //---- 6  AXA ($44),Y
        {
            //TODO: behavior is suspiscious; need to verify
            UINT16 addr = INDIRECT_Y();
            UINT8 val = AC & XR & PEEK(addr);
            POKE(addr, val);
            usedCycles += 6;
        } break;
        case 0x94:  //---- 4  STY ($44,X)
        {
            UINT16 addr = ZERO_PAGE_X();
            POKE(addr, YR);
            usedCycles += 4;
        } break;
        case 0x95:  //---- 4  STA ($44,X)
        {
            UINT16 addr = ZERO_PAGE_X();
            POKE(addr, AC);
            usedCycles += 4;
        } break;
        case 0x96:  //---- 4  STX ($44),Y
        {
            UINT16 addr = ZERO_PAGE_Y();
            POKE(addr, XR);
            usedCycles += 4;
        } break;
        case 0x97:  //---- 4  AAX ($44),Y
        {
            UINT16 addr = ZERO_PAGE_Y();
            UINT8 val = (UINT8)(AC & XR);
            POKE(addr, val);
            usedCycles += 4;
        } break;
        case 0x98:  //--NZ 2  TYA
        {
            AC = YR;
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0x99:  //---- 5  STA $4400,Y
        {
            UINT16 addr = Absolute_Y();
            POKE(addr, AC);
            usedCycles += 5;
        } break;
        case 0x9A:  //---- 2  TXS
        {
            SP = XR;
            usedCycles += 2;
        } break;
        case 0x9B:  //---- 5  XAS ($44),Y
        {
            UINT16 addr = Absolute();
            SP = (UINT8)(AC & XR);
            UINT8 val = (UINT8)(SP & ((addr >> 8) + 1));
            addr = (UINT16)(addr+YR);
            POKE(addr, val);
            usedCycles += 5;
        } break;
        case 0x9C:  //---- 5  SYA $4400,X
        {
            UINT16 addr = Absolute();
            UINT8 val = (UINT8)(YR & (((addr & 0xFF00) >> 8) + 1));
            addr = (UINT16)(addr+XR);
            POKE(addr, val);
            usedCycles += 5;
        } break;
        case 0x9D:  //---- 5  STA $4400,X
        {
            UINT16 addr = Absolute_X();
            POKE(addr, AC);
            usedCycles += 5;
        } break;
        case 0x9E:  //---- 5  SXA $4400,Y
        {
            UINT16 addr = Absolute();
            UINT8 val = (UINT8)(XR & ((addr >> 8) + 1));
            addr = (UINT16)(addr+YR);
            POKE(addr, val);
            usedCycles += 5;
        } break;
        case 0x9F:  //---- 5  AXA $4400,Y
        {
            UINT16 addr = Absolute();
		    UINT8 val = (UINT8)(AC & XR & ((addr >> 8) + 1));
            addr = (UINT16)(addr+YR);
            POKE(addr, val);
            usedCycles += 5;
        } break;
        case 0xA0:  //--NZ 2  LDY #$44
        {
            UINT16 addr = IMMEDIATE();
            YR = PEEK(addr);
            NZ(YR);
            usedCycles += 2;
        } break;
        case 0xA1:  //--NZ 6  LDA ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            AC = PEEK(addr);
            NZ(AC);
            usedCycles += 6;
        } break;
        case 0xA2:  //--NZ 2  LDX #$44
        {
            UINT16 addr = IMMEDIATE();
            XR = PEEK(addr);
            NZ(XR);
            usedCycles += 2;
        } break;
        case 0xA3:  //--NZ 6  LAX ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            UINT8 val = PEEK(addr);
            XR = AC = val;
            NZ(XR);
            usedCycles += 6;
        } break;
        case 0xA4:  //--NZ 3  LDY $44
        {
            UINT16 addr = ZERO_PAGE();
            YR = PEEK(addr);
            NZ(YR);
            usedCycles += 3;
        } break;
        case 0xA5:  //--NZ 3  LDA $44
        {
            UINT16 addr = ZERO_PAGE();
            AC = PEEK(addr);
            NZ(AC);
            usedCycles += 3;
        } break;
        case 0xA6:  //--NZ 3  LDX $44
        {
            UINT16 addr = ZERO_PAGE();
            XR = PEEK(addr);
            NZ(XR);
            usedCycles += 3;
        } break;
        case 0xA7:  //--NZ 3  LAX $44
        {
            UINT16 addr = ZERO_PAGE();
            UINT8 val = PEEK(addr);
            XR = AC = val;
            NZ(XR);
            usedCycles += 3;
        } break;
        case 0xA8:  //--NZ 2  TAY
        {
            YR = AC;
            NZ(YR);
            usedCycles += 2;
        } break;
        case 0xA9:  //--NZ 2  LDA #$44
        {
            UINT16 addr = IMMEDIATE();
            AC = PEEK(addr);
            NZ(AC);
            usedCycles += 2;
        } break;
        case 0xAA:  //--NZ 2  TAX
        {
            XR = AC;
            NZ(XR);
            usedCycles += 2;
        } break;
        case 0xAB:  //--NZ 2  ATX #$44
        {
            UINT16 addr = IMMEDIATE();
            UINT8 val = PEEK(addr);
            XR = AC &= val;
            NZ(XR);
            usedCycles += 2;
        } break;
        case 0xAC:  //--NZ 4  LDY $4400
        {
            UINT16 addr = Absolute();
            YR = PEEK(addr);
            NZ(YR);
            usedCycles += 4;
        } break;
        case 0xAD:  //--NZ 4  LDA $4400
        {
            UINT16 addr = Absolute();
            AC = PEEK(addr);
            NZ(YR);
            usedCycles += 4;
        } break;
        case 0xAE:  //--NZ 4  LDX $4400
        {
            UINT16 addr = Absolute();
            XR = PEEK(addr);
            NZ(YR);
            usedCycles += 4;
        } break;
        case 0xAF:  //--NZ 4  LAX $4400
        {
            UINT16 addr = Absolute();
            UINT8 val = PEEK(addr);
            XR = AC = val;
            NZ(XR);
            usedCycles += 4;
        } break;
        case 0xB0:  //---- 2+ BCS
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (C) {
                usedCycles++;
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0xB1:  //--NZ 5+ LDA ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            AC = PEEK(addr);
            NZ(AC);
            usedCycles += 5;
        } break;
        case 0xB2:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0xB3:  //--NZ 5+ LAX ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            UINT8 val = PEEK(addr);
            XR = AC = val;
            NZ(XR);
            usedCycles += 5;
        } break;
        case 0xB4:  //--NZ 4  LDY $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            YR = PEEK(addr);
            NZ(YR);
            usedCycles += 4;
        } break;
        case 0xB5:  //--NZ 4  LDA $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            AC = PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0xB6:  //--NZ 4  LDX $44,Y
        {
            UINT16 addr = ZERO_PAGE_Y();
            XR = PEEK(addr);
            NZ(XR);
            usedCycles += 4;
        } break;
        case 0xB7:  //--NZ 4  LAX $44,Y
        {
            UINT16 addr = ZERO_PAGE();
            UINT8 val = PEEK(addr);
            XR = AC = val;
            NZ(XR);
            usedCycles += 4;
        } break;
        case 0xB8:  //-V-- 2  CLV
        {
            V = FALSE;
            usedCycles += 2;
        } break;
        case 0xB9:  //--NZ 4+ LDA $4400,Y
        {
            UINT16 addr = Absolute_Y();
            AC = PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0xBA:  //--NZ 2  TSX
        {
            XR = SP;
            NZ(XR);
            usedCycles += 2;
        } break;
        case 0xBB:  //--NZ 4+ LAR $4400,Y
        {
            UINT16 addr = Absolute_Y();
            UINT8 val = PEEK(addr);
            XR = AC = SP &= val;
            NZ(XR);
            POKE(addr, XR);
            usedCycles += 4;
        } break;
        case 0xBC:  //--NZ 4+ LDY $4400,X
        {
            UINT16 addr = Absolute_X();
            YR = PEEK(addr);
            NZ(YR);
            usedCycles += 4;
        } break;
        case 0xBD:  //--NZ 4+ LDA $4400,X
        {
            UINT16 addr = Absolute_X();
            AC = PEEK(addr);
            NZ(AC);
            usedCycles += 4;
        } break;
        case 0xBE:  //--NZ 4+ LDX $4400,Y
        {
            UINT16 addr = Absolute_Y();
            XR = PEEK(addr);
            NZ(XR);
            usedCycles += 4;
        } break;
        case 0xBF:  //--NZ 4+ LAX $4400,Y
        {
            UINT16 addr = Absolute_Y();
            UINT8 val = PEEK(addr);
            XR = AC = val;
            NZ(XR);
            usedCycles += 4;
        } break;
        case 0xC0:  //C-NZ 2  CPY #$44
        {
            UINT16 addr = IMMEDIATE();
            CPY(addr);
            usedCycles += 2;
        } break;
        case 0xC1:  //C-NZ 6  CMP ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            CMP(addr);
            usedCycles += 6;
        } break;
        case 0xC2:  //---- 2  DOP
        {
            PC++;
            usedCycles += 2;
        } break;
        case 0xC3:  //C-NZ 8  DCP ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            DCP(addr);
            usedCycles += 8;
        } break;
        case 0xC4:  //C-NZ 3  CPY $44
        {
            UINT16 addr = ZERO_PAGE();
            CPY(addr);
            usedCycles += 3;
        } break;
        case 0xC5:  //C-NZ 3  CMP $44
        {
            UINT16 addr = ZERO_PAGE();
            CMP(addr);
            usedCycles += 3;
        } break;
        case 0xC6:  //--NZ 5  DEC $44
        {
            UINT16 addr = ZERO_PAGE();
            DEC(addr);
            usedCycles += 5;
        } break;
        case 0xC7:  //C-NZ 5  DCP $44
        {
            UINT16 addr = ZERO_PAGE();
            DCP(addr);
            usedCycles += 5;
        } break;
        case 0xC8:  //--NZ 2  INY
        {
            YR++;
            NZ(YR);
            usedCycles += 2;
        } break;
        case 0xC9:  //C-NZ 2  CMP #$44
        {
            UINT16 addr = IMMEDIATE();
            CMP(addr);
            usedCycles += 2;
        } break;
        case 0xCA:  //--NZ 2  DEX
        {
            XR--;
            NZ(XR);
            usedCycles += 2;
        } break;
        case 0xCB:  //C-NZ 2  AXS #$44
        {
            XR &= AC;
            UINT16 addr = IMMEDIATE();
            UINT8 data = PEEK(addr);
		    C = XR >= data;
		    XR = (UINT8)(XR-data);
            NZ(XR);
            usedCycles += 2;
        } break;
        case 0xCC:  //C-NZ 4  CPY $4400
        {
            UINT16 addr = Absolute();
            CPY(addr);
            usedCycles += 4;
        } break;
        case 0xCD:  //C-NZ 4  CMP $4400
        {
            UINT16 addr = Absolute();
            CMP(addr);
            usedCycles += 4;
        } break;
        case 0xCE:  //--NZ 6  DEC $4400
        {
            UINT16 addr = Absolute();
            DEC(addr);
            usedCycles += 6;
        } break;
        case 0xCF:  //DCP $4400
        {
            UINT16 addr = ZERO_PAGE();
            DCP(addr);
            usedCycles += 5;
        } break;
        case 0xD0:  //---- 2+ BNE
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (!Z) {
                usedCycles++;
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0xD1:  //C-NZ 5+ CMP ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            CMP(addr);
            usedCycles += 5;
        } break;
        case 0xD2:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0xD3:  //C-NZ 8  DCP ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            DCP(addr);
            usedCycles += 8;
        } break;
        case 0xD4:  //---- 4  DOP
        {
            PC++;
            usedCycles += 4;
        } break;
        case 0xD5:  //C-NZ 4  CMP $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            CMP(addr);
            usedCycles += 4;
        } break;
        case 0xD6:  //--NZ 6  DEC $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            DEC(addr);
            usedCycles += 6;
        } break;
        case 0xD7:  //DCP $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            DCP(addr);
            usedCycles += 6;
        } break;
        case 0xD8:  //---- 2  CLD
        {
            D = FALSE;
            usedCycles += 2;
        } break;
        case 0xD9:  //C-NZ 4+ CMP $4400,Y
        {
            UINT16 addr = Absolute_Y();
            CMP(addr);
            usedCycles += 4;
        } break;
        case 0xDA:  //---- 2  NOP
        {
            usedCycles += 2;
        } break;
        case 0xDB:  //C-NZ 7  DCP $4400,Y
        {
            UINT16 addr = Absolute_Y();
            DCP(addr);
            usedCycles += 7;
        } break;
        case 0xDC:  //---- 4+ TOP
        {
            PC += 2;
            usedCycles += 4;
        } break;
        case 0xDD:  //C-NZ 4+ CMP $4400,X
        {
            UINT16 addr = Absolute_X();
            CMP(addr);
            usedCycles += 4;
        } break;
        case 0xDE:  //--NZ 7  DEC $4400,X
        {
            UINT16 addr = Absolute_X();
            DEC(addr);
            usedCycles += 7;
        } break;
        case 0xDF:  //C-NZ 7  DCP $4400,X
        {
            UINT16 addr = Absolute_X();
            DCP(addr);
            usedCycles += 7;
        } break;
        case 0xE0:  //C-NZ 2  CPX #$44
        {
            UINT16 addr = IMMEDIATE();
            CPX(addr);
            usedCycles += 2;
        } break;
        case 0xE1:  //CVNZ 6  SBC ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            SBC(addr);
            usedCycles += 6;
        } break;
        case 0xE2:  //---- 2  DOP
        {
            PC++;
            usedCycles += 2;
        } break;
        case 0xE3:  //CVNZ 8  ISC ($44,X)
        {
            UINT16 addr = INDIRECT_X();
            ISC(addr);
            usedCycles += 8;
        } break;
        case 0xE4:  //C-NZ 3  CPX $44
        {
            UINT16 addr = ZERO_PAGE();
            CPX(addr);
            usedCycles += 3;
        } break;
        case 0xE5:  //CVNZ 3  SBC $44
        {
            UINT16 addr = ZERO_PAGE();
            SBC(addr);
            usedCycles += 3;
        } break;
        case 0xE6:  //--NZ 5  INC $44
        {
            UINT16 addr = ZERO_PAGE();
            INC(addr);
            usedCycles += 5;
        } break;
        case 0xE7:  //CVNZ 5  ISC $44
        {
            UINT16 addr = ZERO_PAGE();
            ISC(addr);
            usedCycles += 5;
        } break;
        case 0xE8:  //--NZ 2  INX
        {
            XR++;
            NZ(XR);
            usedCycles += 2;
        } break;
        case 0xE9:  //CVNZ 2  SBC #$44
        {
            UINT16 addr = IMMEDIATE();
            SBC(addr);
            usedCycles += 2;
        } break;
        case 0xEA:  //---- 2  NOP
        {
            usedCycles += 2;
        } break;
        case 0xEB:  //CVNZ 2  SBC #$44
        {
            UINT16 addr = IMMEDIATE();
            SBC(addr);
            usedCycles += 2;
        } break;
        case 0xEC:  //C-NZ 4  CPX $4400
        {
            UINT16 addr = Absolute();
            CPX(addr);
            usedCycles += 4;
        } break;
        case 0xED:  //CVNZ 4  SBC $4400
        {
            UINT16 addr = Absolute();
            SBC(addr);
            usedCycles += 4;
        } break;
        case 0xEE:  //--NZ 6  INC $4400
        {
            UINT16 addr = Absolute();
            INC(addr);
            usedCycles += 6;
        } break;
        case 0xEF:  //CVNZ 6  ISC $4400
        {
            UINT16 addr = Absolute();
            ISC(addr);
            usedCycles += 6;
        } break;
        case 0xF0:  //---- 2|3+ BEQ
        {
            UINT16 addr = IMMEDIATE();
            INT8 val = (INT8)PEEK(addr);
            if (Z) {
                usedCycles++;
                BRANCH(val);
            }
            usedCycles += 2;
        } break;
        case 0xF1:  //CVNZ 5+ SBC ($44),Y
        {
            UINT16 addr = INDIRECT_Y();
            SBC(addr);
            usedCycles += 5;
        } break;
        case 0xF2:  //---- 1  KIL
        {
            PC--;
            usedCycles += 1;
        } break;
        case 0xF3:  //CVNZ 8  ISC ($44),Y
        {
            UINT16 addr = ZERO_PAGE();
            ISC(addr);
            usedCycles += 8;
        } break;
        case 0xF4:  //---- 4  DOP
        {
            PC++;
            usedCycles += 4;
        } break;
        case 0xF5:  //CVNZ 4  SBC $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            SBC(addr);
            usedCycles += 4;
        } break;
        case 0xF6:  //--NZ 6  INC $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            INC(addr);
            usedCycles += 6;
        } break;
        case 0xF7:  //ISC $44,X
        {
            UINT16 addr = ZERO_PAGE_X();
            ISC(addr);
            usedCycles += 6;
        } break;
        case 0xF8:  //SED
        {
            D = TRUE;
            usedCycles += 2;
        } break;
        case 0xF9:  //CVNZ 4+ SBC $4400,Y
        {
            UINT16 addr = Absolute_Y();
            SBC(addr);
            usedCycles += 4;
        } break;
        case 0xFA:  //---- 2  NOP
        {
            usedCycles += 2;
        } break;
        case 0xFB:  //ISC $4400,Y
        {
            UINT16 addr = Absolute_Y();
            ISC(addr);
            usedCycles += 6;
        } break;
        case 0xFC:  //---- 4+ TOP
        {
            PC += 2;
            usedCycles += 4;
        } break;
        case 0xFD:  //CVNZ 4+ SBC $4400,X
        {
            UINT16 addr = Absolute_X();
            SBC(addr);
            usedCycles += 4;
        } break;
        case 0xFE:  //--NZ 7  INC $4400,X
        {
            UINT16 addr = Absolute_X();
            INC(addr);
            usedCycles += 7;
        } break;
        case 0xFF:  //CVNZ 7  ISC $4400,X
        {
            UINT16 addr = Absolute_X();
            ISC(addr);
            usedCycles += 7;
        } break;
    }
    } while (usedCycles < minimum);
    
    return usedCycles;
}

UINT8 _6502c::MERGE_SR()
{
    return (UINT8)((N ? 0x80 : 0) | (V ? 0x40 : 0) | (B ? 0x10 : 0) |
            (D ? 0x08 : 0) | (I ? 0x04 : 0) | (Z ? 0x02 : 0) | (C ? 0x01 : 0));
}

void _6502c::SPLIT_SR(UINT8 x)
{
    N = !!(x & 0x80);
    V = !!(x & 0x40);
    B = !!(x & 0x10);
    D = !!(x & 0x08);
    I = !!(x & 0x04);
    Z = !!(x & 0x02);
    C = !!(x & 0x01);
}

void _6502c::PUSH(UINT8 x)
{
    POKE((UINT16)(0x100|SP), x);
    SP--;
}

void _6502c::dPUSH(UINT16 x)
{
    POKE((UINT16)(0x100|SP), (UINT16)((x & 0xFF00) >> 8));
    SP--;
    POKE((UINT16)(0x100|SP), (UINT16)(x & 0x00FF));
    SP--;
}

UINT8 _6502c::POP()
{
    SP++;
    return PEEK((UINT16)(0x100|SP));
}

UINT16 _6502c::dPOP()
{
    SP++;
    UINT16 x = PEEK((UINT16)(0x100|SP));
    SP++;
    x |= (UINT16)(PEEK((UINT16)(0x100|SP)) << 8);
    return x;
}

void _6502c::BRANCH(INT8 x)
{
    PC = (UINT16)(PC+x);
}

UINT16 _6502c::IMMEDIATE()
{
    UINT16 addr = PC;
    PC++;
    return addr;
}

UINT16 _6502c::ZERO_PAGE()
{
    UINT8 addr = PEEK(PC);
    PC++;
    return addr;
}

UINT16 _6502c::ZERO_PAGE_X()
{
    UINT8 addr = (UINT8)(PEEK(PC)+XR);
    PC++;
    return addr;
}

UINT16 _6502c::ZERO_PAGE_Y()
{
    UINT8 addr = (UINT8)(PEEK(PC)+YR);
    PC++;
    return addr;
}

UINT16 _6502c::Absolute()
{
    UINT16 addr = (UINT16)(PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8));
    PC += 2;
    return addr;
}

UINT16 _6502c::Absolute_X()
{
    UINT16 addr = (UINT16)((PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8))+XR);
    PC += 2;
    return addr;
}

UINT16 _6502c::Absolute_Y()
{
    UINT16 addr = (UINT16)((PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8))+YR);
    PC += 2;
    return addr;
}

UINT16 _6502c::INDIRECT_X()
{
    UINT16 addr = (UINT16)(PEEK((UINT8)(PEEK(PC)+XR)) |
        (PEEK((UINT8)(PEEK(PC)+XR+1)) << 8));
    PC++;
    return addr;
}

UINT16 _6502c::INDIRECT_Y()
{
    UINT16 addr = (UINT16)((PEEK((UINT8)PEEK(PC)) |
        (PEEK((UINT8)(PEEK(PC)+1)) << 8)) + YR);
    PC++;
    return addr;
}

void _6502c::INC(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    val++;
    POKE(addr, val);
    NZ(val);
}

void _6502c::DEC(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    val--;
    POKE(addr, val);
    NZ(val);
}

void _6502c::SLO(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    C = !!(val & 0x80);
    val <<= 1;
    POKE(addr, val);
    AC |= val;
    NZ(AC);
}

void _6502c::ASL(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    C = !!(val & 0x80);
    val <<= 1;
    POKE(addr, val);
    NZ(val);
}

void _6502c::AAC(UINT16 addr)
{
    AC &= PEEK(addr);
    C = !!(AC & 0x80);
    NZ(AC);
}
void _6502c::ASR(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    C = (val & 0x01) != 0;
    val >>= 1; POKE(addr, val);
    NZ(val);
}

void _6502c::LSL(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    C = 0 != (val & 0x80);
    val = (UINT8)((val<<1) | (C ? 1 : 0));
    POKE(addr, val);
    NZ(val);
}

void _6502c::RLA(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    BOOL c = C;
    C = !!(val & 0x80);
    val = (UINT8)((val<<1) | (c ? 1 : 0));
    POKE(addr, val);
    AC &= val;
    NZ(AC);
}

void _6502c::ROL(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    BOOL c = C;
    C = !!(val & 0x80);
    val = (UINT8)((val<<1) | (c ? 1 : 0));
    POKE(addr, val);
    NZ(val);
}

void _6502c::SRE(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    C = (val & 0x01);
    val >>= 1;
    POKE(addr, val);
    AC ^= val;
    NZ(AC);
}

void _6502c::LSR(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    C = (val & 0x01);
    val >>= 1;
    POKE(addr, val);
    NZ(val);
}

void _6502c::ROR(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    UINT8 c = (C ? 0x80 : 0);
    C = (val & 0x01);
    val = (UINT8)((val>>1) | c);
    POKE(addr, val);
    NZ(val);
}

void _6502c::ALR(UINT16 addr)
{
    AC &= PEEK(addr);
    C = (AC & 0x01);
    AC >>= 1;
    NZ(AC);
}

void _6502c::RRA(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    UINT8 c = (C ? 0x80 : 0);
    C = (val & 0x01);
    val = (UINT8)((val>>1) | c);
    POKE(addr, val);
    if (D) {
        dADC(val);
    }
    else {
        bADC(val);
    }
}

void _6502c::BIT(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    Z = (val & AC) == 0;
    N = !!(val & 0x80);
    V = !!(val & 0x40);
}

void _6502c::ADC(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    if (D) {
        dADC(val);
    } 
    else {
        bADC(val);
    }
}

void _6502c::SBC(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    if (D) {
        dSBC(val);
    } 
    else {
        bSBC(val);
    }
}

void _6502c::CMP(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    UINT16 t = (UINT16)(AC - val);
    C = (AC >= val);
    NZ(t);
}

void _6502c::CPY(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    UINT16 t = (UINT16)(YR - val);
    C = (YR >= val);
    NZ(t);
}

void _6502c::CPX(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    UINT16 t = (UINT16)(XR - val);
    C = (XR >= val);
    NZ(t);
}

void _6502c::DCP(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    val--;
    POKE(addr, val);
    UINT16 t = (UINT16)(AC - val);
    C = (AC >= val);
    NZ(t);
}

void _6502c::ISC(UINT16 addr)
{
    UINT8 val = PEEK(addr);
    val++;
    POKE(addr, val);
    if (D) {
        dSBC(val);
    } 
    else {
        bSBC(val);
    }
}

void _6502c::bADC(UINT16 x)
{
    UINT16 tmp = (UINT16)(AC + x + (C ? 1 : 0));
    C = !!(tmp & 0x100);
    V = (((AC ^ x) & 0x80) & ((AC ^ tmp) & 0x80)) == 0;
    AC = (UINT8)tmp;
    NZ(AC);
}

void _6502c::dADC(UINT16 x)
{
    UINT16 tmp = (AC & 0x0F) + (x & 0x0F) + (C ? 1 : 0);
    if (tmp >= 10)
        tmp = (tmp - 10) | 0x10;
    tmp += (AC & 0xF0) + (x & 0xF0);

    Z = (AC + x + (C ? 1 : 0)) == 0;
    N = !!(tmp & 0x80);
    V = (((AC ^ x) & 0x80) & ((AC ^ tmp) & 0x80)) == 0;
    if (tmp >= 0xA0)
	    tmp += 0x60;
    C = !!(tmp & 0x100);
    AC = (UINT8)tmp;
}

void _6502c::bSBC(UINT16 x)
{
    UINT16 tmp = (UINT16)(AC - x - (C ? 0 : 1));
    C = !(tmp & 0x100);
	V = ((AC ^ tmp) & 0x80) && ((AC ^ x) & 0x80);
    AC = (UINT8)tmp;
    NZ(AC);
}

void _6502c::dSBC(UINT16 x)
{
    UINT16 tmp = (UINT16)(AC - x - (C ? 0 : 1));
    UINT16 al = (UINT16)((AC & 0x0F) - (x & 0x0F) - (C ? 0 : 1));
    UINT16 ah = (UINT16)((AC >> 4) - (x >> 4));
    if (al & 0x10) {
        al -= 6;
        ah--;
    }
    if (ah & 0x10)
        ah -= 6;
    C = !(tmp & 0x100);
    V = (((AC ^ tmp) & 0x80) & ((AC ^ x) & 0x80));
    NZ(tmp);
    AC = (UINT8)((ah << 4) | (al & 0x0f));
}
