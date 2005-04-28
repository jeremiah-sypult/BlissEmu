
#include <string.h>
#include "6502c.h"
#include "core/types.h"
#include "core/memory/MemoryBus.h"

typedef enum _OPCODE_MODE {
    IMPLIED     = 0,
    IMMEDIATE   = 1,
    ZERO_PAGE   = 2,
    ZERO_PAGE_X = 3,
    ZERO_PAGE_Y = 4,
    ABSOLUTE    = 5,
    ABSOLUTE_X  = 6,
    ABSOLUTE_Y  = 7,
    RELATIVE    = 8,
    INDIRECT    = 9,
    INDIRECT_X  = 10,
    INDIRECT_Y  = 11,
} OPCODE_MODE;

typedef struct _OPCODE_DESCRIPTOR {
    CHAR*       mnemonic;
    OPCODE_MODE mode;
} OPCODE_DESCRIPTOR;

#define MERGE_SR()         (UINT8)((N << 7) | (V << 6) | (B << 4) | \
                           (D << 3) | (I << 2) | (Z << 1) | C)
#define SPLIT_SR(x)        N = !!(x & 0x80); V = !!(x & 0x40); \
                           B = !!(x & 0x10); D = !!(x & 0x08); \
                           I = !!(x & 0x04); Z = !!(x & 0x02); C = (x & 0x01);

#define PEEK(x)            (UINT8)memoryBus->peek(x)
#define POKE(x, y)         memoryBus->poke(x, (UINT8)y)
#define PUSH(x)            POKE(0x100|SP, x); SP--
#define dPUSH(x)           POKE(0x100|SP, ((x & 0xFF00) >> 8)); SP--; \
                           POKE(0x100|SP, (x & 0x00FF)); SP--
#define POP(x)             SP++; x = PEEK(0x100|SP)
#define dPOP(x)            SP++; x = PEEK(0x100|SP); \
                           SP++; x |= (PEEK(0x100|SP) << 8)

#define BRANCH(x)          PC = (UINT16)(PC+x)
#define IMMEDIATE()        UINT16 addr = PC; PC++
#define ZERO_PAGE()        UINT8 addr = PEEK(PC); PC++
#define ZERO_PAGE_X()      UINT8 addr = PEEK(PC)+XR; PC++
#define ZERO_PAGE_Y()      UINT8 addr = PEEK(PC)+YR; PC++
#define ABSOLUTE()         UINT16 addr = (PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8)); \
                           PC += 2
#define ABSOLUTE_X()       UINT16 addr = (PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8))+XR; \
                           PC += 2
#define ABSOLUTE_Y()       UINT16 addr = (PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8))+XR; \
                           PC += 2
#define INDIRECT_X()       UINT16 addr = PEEK((UINT8)(PEEK(PC)+XR)) | \
                           (PEEK((UINT8)(PEEK(PC)+XR+1)) << 8); PC++
#define INDIRECT_Y()       UINT16 addr = (PEEK((UINT8)PEEK(PC)) | \
                           (PEEK((UINT8)(PEEK(PC)+1)) << 8)) + YR; PC++

#define NZ(value)          N = !!(value & 0x80); Z = !value
#define INC(addr)          UINT8 value = PEEK(addr); value++; \
                           POKE(addr, value); NZ(value)
#define DEC(addr)          UINT8 value = PEEK(addr); value--; \
                           POKE(addr, value); NZ(value)
#define ORA(addr)          AC |= PEEK(addr); NZ(AC)
#define AND(addr)          AC &= PEEK(addr); NZ(AC)
#define EOR(addr)          AC ^= PEEK(addr); NZ(AC)
#define SLO(addr)          UINT8 value = PEEK(addr); C = !!(value & 0x80); \
                           value <<= 1; POKE(addr, value); AC |= value; NZ(AC)
#define ASL(addr)          UINT8 value = PEEK(addr); C = !!(value & 0x80); \
                           value <<= 1; POKE(addr, value); NZ(value)
#define AAC(addr)          UINT8 value = (PEEK(addr) & AC); \
                           C = !!(value & 0x80); POKE(addr, value); NZ(value)
#define ASR(addr)          UINT8 value = PEEK(addr); C = (value & 0x01); \
                           value >>= 1; POKE(addr, value); NZ(value)
#define LSL(addr)          UINT8 value = PEEK(addr); C = !!(value & 0x80); \
                           value = (value<<1) | C; POKE(addr, value); NZ(value)
#define RLA(addr)          UINT8 value = PEEK(addr); C = !!(value & 0x80); \
                           value = (UINT8)((value<<1) | C); value &= AC; \
                           POKE(addr, value); NZ(value)
#define ROL(addr)          UINT8 value = PEEK(addr); BOOL c = C; \
                           C = !!(value & 0x80); value = (UINT8)((value<<1) | c); \
                           POKE(addr, value); NZ(value)
#define SRE(addr)          UINT8 value = PEEK(addr); C = (value & 0x01); \
                           value >>= 1; value ^= AC; POKE(addr, value); \
                           NZ(value)
#define LSR(addr)          UINT8 value = PEEK(addr); C = (value & 0x01); \
                           value >>= 1; \
                           POKE(addr, value); NZ(value)
#define ROR(addr)          UINT8 value = PEEK(addr); UINT8 c = (UINT8)C; \
                           C = (value & 0x01); value = (UINT8)((value>>1) | (c<<7)); \
                           POKE(addr, value); NZ(value)
#define ALR(addr)          UINT8 value = PEEK(addr); AC &= value; \
                           C = (AC & 0x01); AC >>= 1; NZ(AC)
#define RRA(addr)          UINT8 value = PEEK(addr); BOOL c = C; \
                           C = (value & 0x01); value = (UINT8)((value>>1) | (c<<7)); \
                           POKE(addr, value); \
                           if (D) { dADC(value); } else { bADC(value); }
#define BIT(addr)          UINT8 value = PEEK(addr); Z = !(value & AC); \
                           N = !!(value & 0x80); V = !!(value & 0x40)
#define ADC(addr)          UINT8 value = PEEK(addr); \
                           if (D) { dADC(value); } else { bADC(value); }
#define SBC(addr)          UINT8 value = PEEK(addr); \
                           if (D) { dSBC(value); } else { bSBC(value); }
#define CMP(addr)          UINT8 value = PEEK(addr); \
                           UINT16 t = AC - value; C = (AC >= value); NZ(t)
#define CPY(addr)          UINT8 value = PEEK(addr); \
                           UINT16 t = YR - value; C = (YR >= value); NZ(t)
#define CPX(addr)          UINT8 value = PEEK(addr); \
                           UINT16 t = XR - value; C = (XR >= value); NZ(t)
#define DCP(addr)          UINT8 value = PEEK(addr); value--; \
                           POKE(addr, value); NZ(value)
#define ISC(addr)          UINT8 value = PEEK(addr); value++; \
                           if (D) { dSBC(value); } else { bSBC(value); }

#define bADC(x)            UINT16 t = (UINT16)(AC + x + C); C = !!(t & 0x100);  \
                           AC = (UINT8)t; NZ(AC); \
                           V = !((AC ^ x) & 0x80) && ((AC ^ t) & 0x80)

#define dADC(x)            UINT16 t = (UINT16)(AC + x + C); \
                           if ((t & 0x0F) > 0x09) t += 6; \
                           if ((t & 0xF0) > 0x90) { t += 0x60; C = TRUE; } \
                           AC = (UINT8)t; NZ(AC); \
                           V = !((AC ^ x) & 0x80) && ((AC ^ t) & 0x80)

#define bSBC(x)            UINT16 t = (UINT16)(AC - x - !C); C = !!(t & 0x100); \
                           AC = (UINT8)t; NZ(AC); \
                           V = ((AC ^ t) & 0x80) && ((AC ^ x) & 0x80)

#define dSBC(x)            UINT16 t = (UINT16)(AC - x - !C); \
                           UINT16 al = (UINT16)((AC & 0x0f) - (x & 0x0f) - !C); \
                           UINT16 ah = (UINT16)((AC >> 4) - (x >> 4)); \
                           if (al & 0x10) { al -= 6; ah--; } \
                           if (ah & 0x10) ah -= 6; \
                           C = t < 0x100; AC = (UINT8)((ah << 4) | (al & 0x0f)); \
                           NZ(AC); V = ((AC ^ t) & 0x80) && ((AC ^ x) & 0x80)

_6502c::_6502c(MemoryBus* mb)
    : Processor("6502C"),
      memoryBus(mb),
      N(FALSE), V(FALSE), B(FALSE), D(FALSE), I(FALSE), Z(FALSE), C(FALSE),
      AC(0), XR(0), YR(0), SP(0),
      PC(0),
      nmiVector(0xFFFA),
      resetVector(0xFFFC),
      irqVector(0xFFFE)
{
}

#ifdef DEVELOPER_VERSION
UINT32 _6502c::getProgramCounter()
{
    return (pinIn[_6502C_PIN_IN_NMI]->isHigh
            ? (PEEK(nmiVector) | PEEK(nmiVector+1) << 8) : PC);
}

OPCODE_DESCRIPTOR opcodes[256] = {
    { "BRK", IMPLIED     },
    { "ORA", INDIRECT_X  },
    { "KIL", IMPLIED     },
    { "SLO", INDIRECT_X  },
    { "DOP", ZERO_PAGE   },
    { "ORA", ZERO_PAGE   },
    { "ASL", ZERO_PAGE   },
    { "SLO", ZERO_PAGE   },
    { "PHP", IMPLIED     },
    { "ORA", IMMEDIATE   },
    { "ASL", IMPLIED     },
    { "AAC", IMMEDIATE   },
    { "TOP", ABSOLUTE    },
    { "ORA", ABSOLUTE    },
    { "ASL", ABSOLUTE    },
    { "SLO", ABSOLUTE    },
    { "BPL", RELATIVE    },
    { "ORA", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "SLO", INDIRECT_Y  },
    { "DOP", IMPLIED     },
    { "ORA", ZERO_PAGE_X },
    { "ASL", ZERO_PAGE_X },
    { "SLO", ZERO_PAGE_X },
    { "CLC", IMPLIED     },
    { "ORA", ABSOLUTE_Y  },
    { "NOP", IMPLIED     },
    { "SLO", ABSOLUTE_Y  },
    { "TOP", IMPLIED     },
    { "ORA", ABSOLUTE_X  },
    { "ASL", ABSOLUTE_X  },
    { "SLO", ABSOLUTE_X  },
    { "JSR", ABSOLUTE    },
    { "AND", INDIRECT_X  },
    { "KIL", IMPLIED     },
    { "RLA", INDIRECT_X  },
    { "BIT", ZERO_PAGE   },
    { "AND", ZERO_PAGE   },
    { "ROL", ZERO_PAGE   },
    { "RLA", ZERO_PAGE   },
    { "PLP", IMPLIED     },
    { "AND", IMMEDIATE   },
    { "ROL", IMPLIED     },
    { "AAC", IMMEDIATE   },
    { "BIT", ABSOLUTE    },
    { "AND", ABSOLUTE    },
    { "ROL", ABSOLUTE    },
    { "RLA", ABSOLUTE    },
    { "BMI", RELATIVE    },
    { "AND", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "RLA", INDIRECT_Y  },
    { "DOP", IMPLIED     },
    { "AND", ZERO_PAGE_X },
    { "ROL", ZERO_PAGE_X },
    { "RLA", ZERO_PAGE_X },
    { "SEC", IMPLIED     },
    { "AND", ABSOLUTE_Y  },
    { "NOP", IMPLIED     },
    { "RLA", ABSOLUTE_Y  },
    { "TOP", IMPLIED     },
    { "AND", ABSOLUTE_X  },
    { "ROL", ABSOLUTE_X  },
    { "RLA", ABSOLUTE_X  },
    { "RTI", IMPLIED     },
    { "EOR", INDIRECT_X  },
    { "KIL", IMPLIED     },
    { "SRE", INDIRECT_X  },
    { "DOP", IMPLIED     },
    { "EOR", ZERO_PAGE   },
    { "LSR", ZERO_PAGE   },
    { "SRE", ZERO_PAGE   },
    { "PHA", IMPLIED     },
    { "EOR", IMMEDIATE   },
    { "LSR", IMPLIED     },
    { "ALR", IMMEDIATE   },
    { "JMP", ABSOLUTE    },
    { "EOR", ABSOLUTE    },
    { "LSR", ABSOLUTE    },
    { "SRE", ABSOLUTE    },
    { "BVC", RELATIVE    },
    { "EOR", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "SRE", INDIRECT_Y  },
    { "DOP", IMPLIED     },
    { "EOR", ZERO_PAGE_X },
    { "LSR", ZERO_PAGE_X },
    { "SRE", ZERO_PAGE_X },
    { "CLI", IMPLIED     },
    { "EOR", ABSOLUTE_Y  },
    { "NOP", IMPLIED     },
    { "SRE", ABSOLUTE_Y  },
    { "TOP", IMPLIED     },
    { "EOR", ABSOLUTE_X  },
    { "LSR", ABSOLUTE_X  },
    { "SRE", ABSOLUTE_X  },
    { "RTS", IMPLIED     },
    { "ADC", INDIRECT_X  },
    { "KIL", IMPLIED     },
    { "RRA", INDIRECT_X  },
    { "DOP", IMPLIED     },
    { "ADC", ZERO_PAGE   },
    { "ROR", ZERO_PAGE   },
    { "RRA", ZERO_PAGE   },
    { "PLA", IMPLIED     },
    { "ADC", IMMEDIATE   },
    { "ROR", IMPLIED     },
    { "ARR", IMMEDIATE   },
    { "JMP", INDIRECT    },
    { "ADC", ABSOLUTE    },
    { "ROR", ABSOLUTE    },
    { "RRA", ABSOLUTE    },
    { "BVS", RELATIVE    },
    { "ADC", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "RRA", INDIRECT_Y  },
    { "DOP", IMPLIED     },
    { "ADC", ZERO_PAGE_X },
    { "ROR", ZERO_PAGE_X },
    { "RRA", ZERO_PAGE_X },
    { "SEI", IMPLIED     },
    { "ADC", ABSOLUTE_Y  },
    { "NOP", IMPLIED     },
    { "RRA", ABSOLUTE_Y  },
    { "TOP", IMPLIED     },
    { "ADC", ABSOLUTE_X  },
    { "ROR", ABSOLUTE_X  },
    { "RRA", ABSOLUTE_X  },
    { "DOP", IMPLIED     },
    { "STA", INDIRECT_X  },
    { "DOP", IMPLIED     },
    { "AAX", INDIRECT_X  },
    { "STY", ZERO_PAGE   },
    { "STA", ZERO_PAGE   },
    { "STX", ZERO_PAGE   },
    { "AAX", ZERO_PAGE   },
    { "DEY", IMPLIED     },
    { "DOP", IMPLIED     },
    { "TXA", IMPLIED     },
    { "XAA", IMPLIED     }, /* NOTE: BEHAVIOR UNKNOWN */
    { "STY", ABSOLUTE    },
    { "STA", ABSOLUTE    },
    { "STX", ABSOLUTE    },
    { "AAX", ABSOLUTE    },
    { "BCC", RELATIVE    },
    { "STA", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "AXA", ZERO_PAGE   },
    { "STY", INDIRECT_X  },
    { "STA", INDIRECT_X  },
    { "STX", ZERO_PAGE_Y },
    { "AAX", ZERO_PAGE_Y },
    { "TYA", IMPLIED     },
    { "STA", ABSOLUTE_Y  },
    { "TXS", IMPLIED     },
    { "XAS", INDIRECT_Y  },
    { "SYA", ABSOLUTE_X  },
    { "STA", ABSOLUTE_X  },
    { "SXA", ABSOLUTE_Y  },
    { "AXA", ABSOLUTE_Y  },
    { "LDY", IMMEDIATE   },
    { "LDA", INDIRECT_X  },
    { "LDX", IMMEDIATE   },
    { "LAX", INDIRECT_X  },
    { "LDY", ZERO_PAGE   },
    { "LDA", ZERO_PAGE   },
    { "LDX", ZERO_PAGE   },
    { "LAX", ZERO_PAGE   },
    { "TAY", IMPLIED     },
    { "LDA", IMMEDIATE   },
    { "TAX", IMPLIED     },
    { "ATX", IMPLIED     },
    { "LDY", ABSOLUTE    },
    { "LDA", ABSOLUTE    },
    { "LDX", ABSOLUTE    },
    { "LAX", ABSOLUTE    },
    { "BCS", RELATIVE    },
    { "LDA", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "LAX", INDIRECT_Y  },
    { "LDY", ZERO_PAGE_X },
    { "LDA", ZERO_PAGE_X },
    { "LDX", ZERO_PAGE_Y },
    { "LAX", ZERO_PAGE_Y },
    { "CLV", IMPLIED     },
    { "LDA", ABSOLUTE_Y  },
    { "TSX", IMPLIED     },
    { "LAR", ABSOLUTE_Y  },
    { "LDY", ABSOLUTE_X  },
    { "LDA", ABSOLUTE_X  },
    { "LDX", ABSOLUTE_Y  },
    { "LAX", ABSOLUTE_Y  },
    { "CPY", IMMEDIATE   },
    { "CPY", INDIRECT_X  },
    { "DOP", IMPLIED     },
    { "DCP", INDIRECT_X  },
    { "CPY", ZERO_PAGE   },
    { "CMP", ZERO_PAGE   },
    { "DEC", ZERO_PAGE   },
    { "DCP", ZERO_PAGE   },
    { "INY", IMPLIED     },
    { "CMP", IMMEDIATE   },
    { "DEX", IMPLIED     },
    { "AXS", IMMEDIATE   },
    { "CPY", ABSOLUTE    },
    { "CMP", ABSOLUTE    },
    { "DEC", ABSOLUTE    },
    { "DCP", ABSOLUTE    },
    { "BNE", RELATIVE    },
    { "CMP", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "DCP", INDIRECT_Y  },
    { "DOP", IMPLIED     },
    { "CMP", ZERO_PAGE_X },
    { "DEC", ZERO_PAGE_X },
    { "DCP", ZERO_PAGE_X },
    { "CLD", IMPLIED     },
    { "CMP", ABSOLUTE_Y  },
    { "NOP", IMPLIED     },
    { "DCP", ABSOLUTE_Y  },
    { "TOP", IMPLIED     },
    { "CMP", ABSOLUTE_X  },
    { "DEC", ABSOLUTE_X  },
    { "DCP", ABSOLUTE_X  },
    { "CPX", IMMEDIATE   },
    { "SBC", INDIRECT_X  },
    { "DOP", IMPLIED     },
    { "ISC", INDIRECT_X  },
    { "CPX", ZERO_PAGE   },
    { "SBC", ZERO_PAGE   },
    { "INC", ZERO_PAGE   },
    { "ISC", ZERO_PAGE   },
    { "INX", IMPLIED     },
    { "SBC", IMMEDIATE   },
    { "NOP", IMPLIED     },
    { "SBC", IMMEDIATE   },
    { "CPX", ABSOLUTE    },
    { "SBC", ABSOLUTE    },
    { "INC", ABSOLUTE    },
    { "ISC", ABSOLUTE    },
    { "BEQ", RELATIVE    },
    { "SBC", INDIRECT_Y  },
    { "KIL", IMPLIED     },
    { "ISC", INDIRECT_Y  },
    { "DOP", IMPLIED     },
    { "SBC", ZERO_PAGE_X },
    { "INC", ZERO_PAGE_X },
    { "ISC", ZERO_PAGE_X },
    { "SED", IMPLIED     },
    { "SBC", ABSOLUTE_Y  },
    { "NOP", IMPLIED     },
    { "ISC", ABSOLUTE_Y  },
    { "TOP", IMPLIED     },
    { "SBC", ABSOLUTE_X  },
    { "INC", ABSOLUTE_X  },
    { "ISC", ABSOLUTE_X  },
};

UINT32 _6502c::decode(CHAR output[256], UINT32 memoryLocation)
{
    UINT8 op = PEEK(memoryLocation);
    output[0] = 0;
    itoa16(output, op, 2);
    UINT8 opCodeSize = 1;

    switch (opcodes[op].mode) {
        case IMPLIED:
            strcat(output, "          ");
            break;
        case IMMEDIATE:
        case ZERO_PAGE:
        case ZERO_PAGE_X:
        case ZERO_PAGE_Y:
        case RELATIVE:
        case INDIRECT_X:
        case INDIRECT_Y:
            strcat(output, " ");
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, "       ");
            opCodeSize = 2;
            break;
        case ABSOLUTE:
        case ABSOLUTE_X:
        case ABSOLUTE_Y:
        case INDIRECT:
            strcat(output, " ");
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, " ");
            itoa16(output, PEEK(memoryLocation+2), 2);
            strcat(output, "    ");
            opCodeSize = 3;
            break;
    }
    strcat(output, opcodes[op].mnemonic);
    switch (opcodes[op].mode) {
        case IMMEDIATE:
            strcat(output, " #$");
            itoa16(output, PEEK(memoryLocation+1), 2);
            opCodeSize = 2;
            break;
        case ZERO_PAGE:
            strcat(output, " $");
            itoa16(output, PEEK(memoryLocation+1), 2);
            opCodeSize = 2;
            break;
        case ZERO_PAGE_X:
            strcat(output, " $");
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, ",X");
            opCodeSize = 2;
            break;
        case ZERO_PAGE_Y:
            strcat(output, " $");
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, ",Y");
            opCodeSize = 2;
            break;
        case ABSOLUTE:
            strcat(output, " $");
            itoa16(output, PEEK(memoryLocation+2), 2);
            itoa16(output, PEEK(memoryLocation+1), 2);
            opCodeSize = 3;
            break;
        case ABSOLUTE_X:
            strcat(output, " $");
            itoa16(output, PEEK(memoryLocation+2), 2);
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, ",X");
            opCodeSize = 3;
            break;
        case ABSOLUTE_Y:
            strcat(output, " $");
            itoa16(output, PEEK(memoryLocation+2), 2);
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, ",Y");
            opCodeSize = 3;
            break;
        case RELATIVE:
        {
            strcat(output, " $");
            INT8 value = PEEK(memoryLocation+1);
            memoryLocation += 2;
            memoryLocation += value;
            itoa16(output, memoryLocation, 4);
            opCodeSize = 2;
            break;
        }
        case INDIRECT:
            strcat(output, " ($");
            itoa16(output, PEEK(memoryLocation+2), 2);
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, ")");
            opCodeSize = 3;
            break;
        case INDIRECT_X:
            strcat(output, " ($");
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, ",X)");
            opCodeSize = 2;
            break;
        case INDIRECT_Y:
            strcat(output, " ($");
            itoa16(output, PEEK(memoryLocation+1), 2);
            strcat(output, "),Y");
            opCodeSize = 2;
    }
    return opCodeSize;;
}

void _6502c::getRegisters(CHAR sr[40])
{
    strcpy(sr, "AC=");
    itoa16(sr, AC, 2);
    strcat(sr, " XR=");
    itoa16(sr, XR, 2);
    strcat(sr, " YR=");
    itoa16(sr, YR, 2);
    strcat(sr, " SP=");
    itoa16(sr, SP, 2);
    strcat(sr, "    ");
    strcat(sr, (N ? "N" : "-"));
    strcat(sr, (V ? "V" : "-"));
    strcat(sr, "-");
    strcat(sr, (B ? "B" : "-"));
    strcat(sr, (D ? "D" : "-"));
    strcat(sr, (I ? "I" : "-"));
    strcat(sr, (Z ? "Z" : "-"));
    strcat(sr, (C ? "C" : "-"));
}
#endif

/**
 * Processor clock speed of the 6502c is 1.78mhz.
 */
INT32 _6502c::getClockSpeed()
{
    return 1792080;  //NTSC/2
}

void _6502c::resetProcessor()
{
    N = V = B = D = I = Z = C = FALSE;
    AC = XR = YR = SP = 0;
fprintf(stderr, "resetting reset vector\n");
    PC = (UINT16)(PEEK(resetVector) | PEEK(resetVector+1) << 8);
fprintf(stderr, "done resetting reset vector\n");
}

/**
 * This method ticks the CPU and returns the number of cycles that were
 * used up, indicating to the main emulation loop when the CPU will need
 * to be ticked again.
 */
INT32 _6502c::tick(INT32)
{
#ifdef _TRACE_TICKS
    fprintf(stderr, "_6502c::tick()\n");
#endif

    if (pinIn[_6502C_PIN_IN_NMI]->isHigh) {
        pinIn[_6502C_PIN_IN_NMI]->isHigh = FALSE;
        dPUSH(PC);
        UINT8 SR = MERGE_SR();
        PUSH(SR);
        PC = (PEEK(nmiVector) | PEEK(nmiVector+1) << 8);
    }

fprintf(stderr, "peeking for opcode\n");
    UINT8 op = PEEK(PC);
fprintf(stderr, "op: %02X\n", op);
    PC++;
    switch (op) {
        default:
        case 0x00:  //---- 7  BRK
        {
            PC += 2;
            if (!I) {
                B = TRUE;
                dPUSH(PC);
                UINT8 SR = MERGE_SR();
                PUSH(SR);
                PC = (PEEK(irqVector) | PEEK(irqVector+1) << 8);
            }
            return 7;
        }
        case 0x01:  //--NZ 6  ORA ($44,X)
        {
            INDIRECT_X();
            ORA(addr);
            return 6;
        }
        case 0x02:  //---- -  KIL
        {
            PC--;
            return 1;
        }
        case 0x03:  //C-NZ 8  SLO ($44,X)
        {
            INDIRECT_X();
            SLO(addr);
            return 8;
        }
        case 0x04:  //---- 3  DOP $44
        {
            PC++;
            return 3;
        }
        case 0x05:  //--NZ 2  ORA $44
        {
            ZERO_PAGE();
            ORA(addr);
            return 2;
        }
        case 0x06:  //C-NZ 5  ASL $44
        {
            ZERO_PAGE();
            ASL(addr);
            return 5;
        }
        case 0x07:  //C-NZ 5  SLO $44
        {
            ZERO_PAGE();
            SLO(addr);
            return 5;
        }
        case 0x08:  //---- 3  PHP
        {
            UINT8 SR = MERGE_SR();
            PUSH(SR);
            return 3;
        }
        case 0x09:  //--NZ 2  ORA #$44
        {
            IMMEDIATE();
            ORA(addr);
            return 2;
        }
        case 0x0A:  //C-NZ 2  ASL A
        {
            C = !!(AC & 0x80);
            AC <<= 1;
            NZ(AC);
            return 2;
        }
        case 0x0B:  //C-NZ 2  AAC #$44
        {
            IMMEDIATE();
            AAC(addr);
            return 2;
        }
        case 0x0C:  //---- 4  TOP $4400
        {
            PC += 2;
            return 4;
        }
        case 0x0D:  //--NZ 4  ORA $4400
        {
            ABSOLUTE();
            ORA(addr);
            return 4;
        }
        case 0x0E:  //C-NZ 6  ASL $4400
        {
            ABSOLUTE();
            ASL(addr);
            return 6;
        }
        case 0x0F:  //C-NZ 6  SLO $4400
        {
            ABSOLUTE();
            SLO(addr);
            return 6;
        }
        case 0x10:  //---- 2|3+ BPL
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (!N) {
                BRANCH(value);
            }
            return 2 + !N;
        }
        case 0x11:  //--NZ 5+ ORA ($44),Y;
        {
            INDIRECT_Y();
            ORA(addr);
            return 5;
        }
        case 0x12:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x13:  //SLO ($44),Y
        {
            INDIRECT_Y();
            SLO(addr);
            return 5;
        }
        case 0x14:  //DOP
        {
            PC++;
            return 3;
        }
        case 0x15:  //ORA $44,X
        {
            ZERO_PAGE_X();
            ORA(addr);
            return 3;
        }
        case 0x16:  //C-NZ 6  ASL $44,X
        {
            ZERO_PAGE_X();
            ASL(addr);
            return 6;
        }
        case 0x17:  //SLO $44,X
        {
            ZERO_PAGE_X();
            SLO(addr);
            return 5;
        }
        case 0x18:  //---- 2  CLC
        {
            C = FALSE;
            return 2;
        }
        case 0x19:  //--NZ 4+ ORA $4400,Y
        {
            ABSOLUTE_Y();
            ORA(addr);
            return 4;
        }
        case 0x1A:  //NOP
        {
            return 2;
        }
        case 0x1B:  //SLO $4400,Y
        {
            ABSOLUTE_Y();
            SLO(addr);
            return 5;
        }
        case 0x1C:  //TOP
        {
            PC += 2;
            return 4;
        }
        case 0x1D:  //--NZ 4+ ORA $4400,X
        {
            ABSOLUTE_X();
            ORA(addr);
            return 4;
        }
        case 0x1E:  //C-NZ 7  ASL $4400,X
        {
            ABSOLUTE_X();
            ASL(addr);
            return 7;
        }
        case 0x1F:  //SLO $4400,X
        {
            ABSOLUTE_X();
            SLO(addr);
            return 5;
        }
        case 0x20:  //---- 6  JSR $4400
        {
            UINT16 addr = PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8);
            PC += 2;
            dPUSH(PC);
            PC = addr;
            return 6;
        }
        case 0x21:  //AND ($44,X)
        {
            INDIRECT_X();
            AND(addr);
            return 6;
        }
        case 0x22:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x23:  //RLA ($44,X)
        {
            INDIRECT_X();
            RLA(addr);
            return 8;
        }
        case 0x24:  //-VNZ 3  BIT $44
        {
            ZERO_PAGE();
            BIT(addr);
            return 3;
        }
        case 0x25:  //--NZ 2  AND $44
        {
            ZERO_PAGE();
            AND(addr);
            return 2;
        }
        case 0x26:  //C-NZ 5  ROL $44
        {
            ZERO_PAGE();
            ROL(addr);
            return 5;
        }
        case 0x27:  //RLA $44
        {
            ZERO_PAGE();
            RLA(addr);
            return 5;
        }
        case 0x28:  //PLP
        {
            UINT8 SR;
            POP(SR);
            SPLIT_SR(SR);
            return 4;
        }
        case 0x29:  //--NZ 2  AND #$44
        {
            IMMEDIATE();
            AND(addr);
            return 2;
        }
        case 0x2A:  //C-NZ 2  ROL A
        {
            BOOL c = C;
            C = !!(AC & 0x80);
            AC = (UINT8)((AC<<1) | c);
            NZ(AC);
            return 2;
        }
        case 0x2B:  //AAC #$44
        {
            IMMEDIATE();
            AAC(addr);
            return 2;
        }
        case 0x2C:  //-VNZ 4  BIT $4400
        {
            ABSOLUTE();
            BIT(addr);
            return 4;
        }
        case 0x2D:  //--NZ 4  AND $4400
        {
            ABSOLUTE();
            AND(addr);
            return 4;
        }
        case 0x2E:  //C-NZ 6  ROL $4400
        {
            ABSOLUTE();
            ROL(addr);
            return 6;
        }
        case 0x2F:  //RLA $4400
        {
            ABSOLUTE();
            RLA(addr);
            return 6;
        }
        case 0x30:  //BMI
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (N) {
                BRANCH(value);
            }
            return 2 + N;
        }
        case 0x31:  //--NZ 5+ AND ($44),Y
        {
            INDIRECT_Y();
            AND(addr);
            return 5;
        }
        case 0x32:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x33:  //RLA ($44),Y
        {
            INDIRECT_Y();
            RLA(addr);
            return 8;
        }
        case 0x34:  //DOP
        {
            PC++;
            return 4;
        }
        case 0x35:  //--NZ 3  AND $44,X
        {
            ZERO_PAGE_X();
            AND(addr);
            return 3;
        }
        case 0x36:  //C-NZ 6  ROL $44,X
        {
            ZERO_PAGE_X();
            ROL(addr);
            return 6;
        }
        case 0x37:  //RLA $44,X
        {
            ZERO_PAGE_X();
            RLA(addr);
            return 6;
        }
        case 0x38:  //SEC
        {
            C = TRUE;
            return 2;
        }
        case 0x39:  //--NZ 4+ AND $4400,Y
        {
            ABSOLUTE_Y();
            AND(addr);
            return 4;
        }
        case 0x3A:  //NOP
        {
            return 2;
        }
        case 0x3B:  //RLA $4400,Y
        {
            ABSOLUTE_Y();
            RLA(addr);
            return 7;
        }
        case 0x3C:  //TOP
        {
            PC += 2;
            return 4;
        }
        case 0x3D:  //--NZ 4+ AND $4400,X
        {
            ABSOLUTE_X();
            AND(addr);
            return 4;
        }
        case 0x3E:  //C-NZ 7  ROL $4400,X
        {
            ABSOLUTE_X();
            ROL(addr);
            return 7;
        }
        case 0x3F:  //RLA $4400,X
        {
            ABSOLUTE_X();
            RLA(addr);
            return 7;
        }
        case 0x40:  //RTI
        {
            UINT8 SR;
            POP(SR);
            SPLIT_SR(SR);
            dPOP(PC);
            return 6;
        }
        case 0x41:  //--NZ 6  EOR ($44,X)
        {
            INDIRECT_X();
            EOR(addr);
            return 6;
        }
        case 0x42:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x43:  //SRE ($44,X)
        {
            INDIRECT_X();
            SRE(addr);
            return 7;
        }
        case 0x44:  //DOP
        {
            PC++;
            return 3;
        }
        case 0x45:  //--NZ 3  EOR $44
        {
            ZERO_PAGE();
            EOR(addr);
            return 3;
        }
        case 0x46:  //C-NZ 5  LSR $44
        {
            ZERO_PAGE();
            LSR(addr);
            return 5;
        }
        case 0x47:  //SRE $44
        {
            ZERO_PAGE();
            SRE(addr);
            return 5;
        }
        case 0x48:  //PHA
        {
            PUSH(AC);
            return 3;
        }
        case 0x49:  //--NZ 2  EOR #$44
        {
            IMMEDIATE();
            EOR(addr);
            return 2;
        }
        case 0x4A:  //C-NZ 2  LSR A
        {
            C = (AC & 0x01);
            AC >>= 1;
            NZ(AC);
            return 2;
        }
        case 0x4B:  //ALR #$44
        {
            IMMEDIATE();
            ALR(addr);
            return 2;
        }
        case 0x4C:  //---- 3  JMP $4400
        {
            PC = PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8);
            return 3;
        }
        case 0x4D:  //--NZ 4  EOR $4400
        {
            ABSOLUTE();
            EOR(addr);
            return 4;
        }
        case 0x4E:  //C-NZ 6  LSR $4400
        {
            ABSOLUTE();
            LSR(addr);
            return 6;
        }
        case 0x4F:  //SRE $4400
        {
            ABSOLUTE();
            SRE(addr);
            return 6;
        }
        case 0x50:  //BVC
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (!V) {
                BRANCH(value);
            }
            return 2 + !V;
        }
        case 0x51:  //--NZ 5+ EOR ($44),Y
        {
            INDIRECT_Y();
            EOR(addr);
            return 5;
        }
        case 0x52:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x53:  //SRE ($44),Y
        {
            INDIRECT_Y();
            SRE(addr);
            return 7;
        }
        case 0x54:  //DOP
        {
            PC++;
            return 4;
        }
        case 0x55:  //--NZ 4  EOR $44,X
        {
            ZERO_PAGE_X();
            EOR(addr);
            return 4;
        }
        case 0x56:  //C-NZ 6  LSR $44,X
        {
            ZERO_PAGE_X();
            LSR(addr);
            return 6;
        }
        case 0x57:  //SRE $44,X
        {
            ZERO_PAGE_X();
            SRE(addr);
            return 6;
        }
        case 0x58:  //CLI
        {
            I = FALSE;
            return 2;
        }
        case 0x59:  //--NZ 4+ EOR $4400,Y
        {
            ABSOLUTE_Y();
            EOR(addr);
            return 4;
        }
        case 0x5A:  //---- 2  NOP
        {
            return 2;
        }
        case 0x5B:  //SRE $4400,Y
        {
            ABSOLUTE_Y();
            SRE(addr);
            return 7;
        }
        case 0x5C:  //TOP
        {
            PC += 2;
            return 4;
        }
        case 0x5D:  //--NZ 4+ EOR $4400,X
        {
            ABSOLUTE_X();
            EOR(addr);
            return 4;
        }
        case 0x5E:  //C-NZ 7  LSR $4400,X
        {
            ABSOLUTE_X();
            LSR(addr);
            return 7;
        }
        case 0x5F:  //SRE $4400,X
        {
            ABSOLUTE_X();
            SRE(addr);
            return 7;
        }
        case 0x60:  //---- 6  RTS
        {
            dPOP(PC);
            return 6;
        }
        case 0x61:  //CVNZ 6  ADC ($44,X)
        {
            INDIRECT_X();
            ADC(addr);
            return 6;
        }
        case 0x62:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x63:  //RRA ($44,X)
        {
            INDIRECT_X();
            RRA(addr);
            return 8;
        }
        case 0x64:  //DOP
        {
            PC++;
            return 3;
        }
        case 0x65:  //CVNZ 3  ADC $44
        {
            ZERO_PAGE();
            ADC(addr);
            return 3;
        }
        case 0x66:  //C-NZ 5  ROR $44
        {
            ZERO_PAGE();
            ROR(addr);
            return 5;
        }
        case 0x67:  //RRA $44
        {
            ZERO_PAGE();
            RRA(addr);
            return 5;
        }
        case 0x68:  //---- 4  PLA
        {
            POP(AC);
            return 4;
        }
        case 0x69:  //CVNZ 2  ADC #$44
        {
            IMMEDIATE();
            ADC(addr);
            return 2;
        }
        case 0x6A:  //C-NZ 2  ROR A
        {
            UINT8 c = (UINT8)C;
            C = (AC & 0x01);
            AC = (AC>>1) | (c<<7);
            NZ(AC);
            return 2;
        }
        case 0x6B:  //ARR #$44
        {
            IMMEDIATE();
            AC &= PEEK(addr);
            AC = (AC>>1) | ((AC & 0x01)<<7);
            C = !!(AC & 0x40);
            V = ((AC & 0x40) >> 6) ^ ((AC & 0x20) >> 5);
            NZ(AC);
            return 2;
        }
        case 0x6C:  //---- 5  JMP ($4400)
        {
            UINT16 addr = PEEK(PC) | (PEEK((UINT16)(PC+1)) << 8);
            PC = PEEK(addr) | (PEEK((UINT16)(addr+1)) << 8);
            return 5;
        }
        case 0x6D:  //CVNZ 4  ADC $4400
        {
            ABSOLUTE();
            ADC(addr);
            return 4;
        }
        case 0x6E:  //C-NZ 6  ROR $4400
        {
            ABSOLUTE();
            ROR(addr);
            return 6;
        }
        case 0x6F:  //RRA $4400
        {
            ABSOLUTE();
            RRA(addr);
            return 6;
        }
        case 0x70:  //BVS
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (V) {
                BRANCH(value);
            }
            return 2 + V;
        }
        case 0x71:  //CVNZ 5+ ADC ($44),Y
        {
            INDIRECT_Y();
            ADC(addr);
            return 5;
        }
        case 0x72:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x73:  //RRA ($44),Y
        {
            INDIRECT_Y();
            RRA(addr);
            return 8;
        }
        case 0x74:  //DOP
        {
            PC++;
            return 4;
        }
        case 0x75:  //CVNZ 4  ADC $44,X
        {
            ZERO_PAGE_X();
            ADC(addr);
            return 4;
        }
        case 0x76:  //C-NZ 6  ROR $44,X
        {
            ZERO_PAGE_X();
            ROR(addr);
            return 6;
        }
        case 0x77:  //RRA $44,X
        {
            ZERO_PAGE_X();
            RRA(addr);
            return 6;
        }
        case 0x78:  //SEI
        {
            I = TRUE;
            return 2;
        }
        case 0x79:  //CVNZ 4+ ADC $4400,Y
        {
            ABSOLUTE_Y();
            ADC(addr);
            return 4;
        }
        case 0x7A:  //NOP
        {
            return 2;
        }
        case 0x7B:  //RRA $4400,Y
        {
            ABSOLUTE_Y();
            RRA(addr);
            return 7;
        }
        case 0x7C:  //TOP
        {
            PC += 2;
            return 4;
        }
        case 0x7D:  //CVNZ 4+ ADC $4400,X
        {
            ABSOLUTE_X();
            ADC(addr);
            return 4;
        }
        case 0x7E:  //C-NZ 7  ROR $4400,X
        {
            ABSOLUTE_X();
            ROR(addr);
            return 7;
        }
        case 0x7F:  //RRA $4400,X
        {
            ABSOLUTE_X();
            RRA(addr);
            return 7;
        }
        case 0x80:  //DOP
        {
            PC++;
            return 3;
        }
        case 0x81:  //---- 6  STA ($44,X)
        {
            INDIRECT_X();
            POKE(addr, AC);
            return 6;
        }
        case 0x82:  //DOP
        {
            PC++;
            return 3;
        }
        case 0x83:  //AAX ($44,X)
        {
            INDIRECT_X();
            UINT8 value = XR & AC;
            POKE(addr, value);
            NZ(value);
            return 6;
        }
        case 0x84:  //---- 3  STY $44
        {
            ZERO_PAGE();
            POKE(addr, YR);
            return 3;
        }
        case 0x85:  //---- 3  STA $44
        {
            ZERO_PAGE();
            POKE(addr, AC);
            return 3;
        }
        case 0x86:  //---- 3  STX $44
        {
            ZERO_PAGE();
            POKE(addr, XR);
            return 3;
        }
        case 0x87:  //AAX $44
        {
            ZERO_PAGE();
            UINT8 value = XR & AC;
            POKE(addr, value);
            NZ(value);
            return 3;
        }
        case 0x88:  //--NZ 2  DEY
        {
            YR--;
            NZ(YR);
            return 2;
        }
        case 0x89:  //DOP
        {
            PC++;
            return 3;
        }
        case 0x8A:  //--NZ 2  TXA
        {
            AC = XR;
            NZ(AC);
            return 2;
        }
        case 0x8B:  //XAA  ?!? BEHAVIOR UNKNOWN ?!?
        {
            return 2;
        }
        case 0x8C:  //---- 4  STY $4400
        {
            ABSOLUTE();
            POKE(addr, YR);
            return 4;
        }
        case 0x8D:  //---- 4  STA $4400
        {
            ABSOLUTE();
            POKE(addr, AC);
            return 4;
        }
        case 0x8E:  //---- 4  STX $4400
        {
            ABSOLUTE();
            POKE(addr, XR);
            return 4;
        }
        case 0x8F:  //AAX $4400
        {
            ABSOLUTE();
            UINT8 value = XR & AC;
            POKE(addr, value);
            NZ(value);
            return 4;
        }
        case 0x90:  //---- 2|3+ BCC
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (!C) {
                BRANCH(value);
            }
            return 2 + !C;
        }
        case 0x91:  //---- 6  STA ($44),Y
        {
            INDIRECT_Y();
            POKE(addr, AC);
            return 6;
        }
        case 0x92:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0x93:  //AXA $44
        {
            INDIRECT_Y();
            UINT8 value = XR & AC & 0x7;
            POKE(addr, value);
            NZ(value);
            return 6;
        }
        case 0x94:  //---- 4  STY ($44,X)
        {
            ZERO_PAGE_X();
            POKE(addr, YR);
            return 4;
        }
        case 0x95:  //---- 4  STA $44,X
        {
            ZERO_PAGE_X();
            POKE(addr, AC);
            return 4;
        }
        case 0x96:  //---- 4  STX $44,Y
        {
            ZERO_PAGE_Y();
            POKE(addr, XR);
            return 4;
        }
        case 0x97:  //AAX $44,Y
        {
            ZERO_PAGE_Y();
            UINT8 value = XR & AC;
            POKE(addr, value);
            NZ(value);
            return 4;
        }
        case 0x98:  //--NZ 2  TYA
        {
            AC = YR;
            NZ(AC);
            return 2;
        }
        case 0x99:  //---- 5  STA $4400,Y
        {
            ABSOLUTE_Y();
            POKE(addr, AC);
            return 5;
        }
        case 0x9A:  //--NZ 2  TXS
        {
            SP = XR;
            NZ(SP);
            return 2;
        }
        case 0x9B:  //XAS ($44),Y
        {
            ABSOLUTE_Y();
            SP = XR & AC;
            UINT8 value = (UINT8)(SP & (((addr & 0xFF00) >> 8) + 1));
            POKE(addr, value);
            NZ(value);
            return 5;
        }
        case 0x9C:  //SYA $4400,X
        {
            ABSOLUTE_X();
            UINT8 value = (UINT8)(YR & (((addr & 0xFF00) >> 8) + 1));
            POKE(addr, value);
            NZ(value);
            return 5;
        }
        case 0x9D:  //---- 5  STA $4400,X
        {
            ABSOLUTE_X();
            POKE(addr, AC);
            return 5;
        }
        case 0x9E:  //SXA $4400,Y
        {
            ABSOLUTE_Y();
            UINT8 value = (UINT8)(XR & (((addr & 0xFF00) >> 8) + 1));
            POKE(addr, value);
            NZ(value);
            return 5;
        }
        case 0x9F:  //AXA $4400,Y
        {
            ABSOLUTE_Y();
            UINT8 value = XR & AC & 0x7;
            POKE(addr, value);
            NZ(value);
            return 6;
        }
        case 0xA0:  //--NZ 2  LDY #$44
        {
            IMMEDIATE();
            YR = PEEK(addr);
            NZ(YR);
            return 2;
        }
        case 0xA1:  //--NZ 6  LDA ($44,X)
        {
            INDIRECT_X();
            AC = PEEK(addr);
            NZ(AC);
            return 6;
        }
        case 0xA2:  //--NZ 2  LDX #$44
        {
            IMMEDIATE();
            XR = PEEK(addr);
            NZ(XR);
            return 2;
        }
        case 0xA3:  //LAX ($44,X)
        {
            INDIRECT_X();
            UINT8 value = PEEK(addr);
            XR = AC = value;
            return 4;
        }
        case 0xA4:  //--NZ 3  LDY $44
        {
            ZERO_PAGE();
            YR = PEEK(addr);
            NZ(YR);
            return 3;
        }
        case 0xA5:  //--NZ 3  LDA $44
        {
            ZERO_PAGE();
            AC = PEEK(addr);
            NZ(AC);
            return 3;
        }
        case 0xA6:  //--NZ 3  LDX $44
        {
            ZERO_PAGE();
            XR = PEEK(addr);
            NZ(XR);
            return 3;
        }
        case 0xA7:  //LAX $44
        {
            ZERO_PAGE();
            UINT8 value = PEEK(addr);
            XR = AC = value;
            NZ(value);
            return 3;
        }
        case 0xA8:  //--NZ 2  TAY
        {
            YR = AC;
            NZ(YR);
            return 2;
        }
        case 0xA9:  //--NZ 2  LDA #$44
        {
            IMMEDIATE();
            AC = PEEK(addr);
            NZ(AC);
            return 2;
        }
        case 0xAA:  //--NZ 2  TAX
        {
            XR = AC;
            NZ(XR);
            return 2;
        }
        case 0xAB:  //ATX
        {
            IMMEDIATE();
            AND(addr);
            XR = AC;
            return 2;
        }
        case 0xAC:  //--NZ 4  LDY $4400
        {
            ABSOLUTE();
            YR = PEEK(addr);
            NZ(YR);
            return 4;
        }
        case 0xAD:  //--NZ 4  LDA $4400
        {
            ABSOLUTE();
            AC = PEEK(addr);
            NZ(YR);
            return 4;
        }
        case 0xAE:  //--NZ 4  LDX $4400
        {
            ABSOLUTE();
            XR = PEEK(addr);
            NZ(YR);
            return 4;
        }
        case 0xAF:  //LAX $4400
        {
            ABSOLUTE();
            UINT8 value = PEEK(addr);
            XR = AC = value;
            NZ(value);
            return 4;
        }
        case 0xB0:  //---- 2|3+ BCS
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (C) {
                BRANCH(value);
            }
            return 2 + C;
        }
        case 0xB1:  //--NZ 5+ LDA ($44),Y
        {
            INDIRECT_Y();
            AC = PEEK(addr);
            NZ(AC);
            return 5;
        }
        case 0xB2:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0xB3:  //LAX ($44),Y
        {
            INDIRECT_Y();
            UINT8 value = PEEK(addr);
            XR = AC = value;
            NZ(value);
            return 4;
        }
        case 0xB4:  //--NZ 4  LDY $44,X
        {
            ZERO_PAGE_X();
            YR = PEEK(addr);
            NZ(YR);
            return 4;
        }
        case 0xB5:  //--NZ 4  LDA $44,X
        {
            ZERO_PAGE_X();
            AC = PEEK(addr);
            NZ(AC);
            return 4;
        }
        case 0xB6:  //--NZ 4  LDX $44,Y
        {
            ZERO_PAGE_Y();
            XR = PEEK(addr);
            NZ(XR);
            return 4;
        }
        case 0xB7:  //LAX $44,Y
        {
            ZERO_PAGE();
            UINT8 value = PEEK(addr);
            XR = AC = value;
            NZ(value);
            return 4;
        }
        case 0xB8:  //CLV
        {
            V = FALSE;
            return 2;
        }
        case 0xB9:  //--NZ 4+ LDA $4400,Y
        {
            ABSOLUTE_Y();
            AC = PEEK(addr);
            NZ(AC);
            return 4;
        }
        case 0xBA:  //--NZ 2  TSX
        {
            XR = SP;
            NZ(XR);
            return 2;
        }
        case 0xBB:  //LAR $4400,Y
        {
            ABSOLUTE_Y();
            UINT8 value = PEEK(addr);
            value &= SP;
            POKE(addr, value);
            AC = XR = SP = value;
            NZ(value);
            return 4;
        }
        case 0xBC:  //--NZ 4+ LDY $4400,X
        {
            ABSOLUTE_X();
            YR = PEEK(addr);
            NZ(YR);
            return 4;
        }
        case 0xBD:  //--NZ 4+ LDA $4400,X
        {
            ABSOLUTE_X();
            AC = PEEK(addr);
            NZ(AC);
            return 4;
        }
        case 0xBE:  //--NZ 4+ LDX $4400,Y
        {
            ABSOLUTE_Y();
            XR = PEEK(addr);
            NZ(XR);
            return 4;
        }
        case 0xBF:  //LAX $4400,Y
        {
            ABSOLUTE_Y();
            UINT8 value = PEEK(addr);
            XR = AC = value;
            NZ(value);
            return 4;
        }
        case 0xC0:  //C-NZ 2  CPY #$44
        {
            IMMEDIATE();
            CPY(addr);
            return 2;
        }
        case 0xC1:  //C-NZ 6  CMP ($44,X)
        {
            INDIRECT_X();
            CMP(addr);
            return 6;
        }
        case 0xC2:  //DOP
        {
            PC++;
            return 3;
        }
        case 0xC3:  //DCP ($44,X)
        {
            INDIRECT_X();
            DCP(addr);
            return 3;
        }
        case 0xC4:  //C-NZ 3  CPY $44
        {
            ZERO_PAGE();
            CPY(addr);
            return 3;
        }
        case 0xC5:  //C-NZ 3  CMP $44
        {
            ZERO_PAGE();
            CMP(addr);
            return 3;
        }
        case 0xC6:  //--NZ 5  DEC $44
        {
            ZERO_PAGE();
            DEC(addr);
            return 5;
        }
        case 0xC7:  //DCP $44
        {
            ZERO_PAGE();
            DCP(addr);
            return 5;
        }
        case 0xC8:  //--NZ 2  INY
        {
            YR++;
            NZ(YR);
            return 2;
        }
        case 0xC9:  //CMP #$44
        {
            IMMEDIATE();
            CMP(addr);
            return 2;
        }
        case 0xCA:  //--NZ 2  DEX
        {
            XR--;
            NZ(XR);
            return 2;
        }
        case 0xCB:  //AXS #$44
        {
            XR &= AC;
            IMMEDIATE();
            XR = (UINT8)(XR-PEEK(addr));
            return 2;
        }
        case 0xCC:  //C-NZ 4  CPY $4400
        {
            ABSOLUTE();
            CPY(addr);
            return 4;
        }
        case 0xCD:  //C-NZ 4  CMP $4400
        {
            ABSOLUTE();
            CMP(addr);
            return 4;
        }
        case 0xCE:  //--NZ 6  DEC $4400
        {
            ABSOLUTE();
            DEC(addr);
            return 6;
        }
        case 0xCF:  //DCP $4400
        {
            ZERO_PAGE();
            DCP(addr);
            return 5;
        }
        case 0xD0:  //---- 2|3+ BNE
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (!Z) {
                BRANCH(value);
            }
            return 2 + !Z;
        }
        case 0xD1:  //C-NZ 5+ CMP ($44),Y
        {
            INDIRECT_Y();
            CMP(addr);
            return 5;
        }
        case 0xD2:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0xD3:  //DCP ($44),Y
        {
            INDIRECT_Y();
            DCP(addr);
            return 8;
        }
        case 0xD4:  //DOP
        {
            PC++;
            return 4;
        }
        case 0xD5:  //C-NZ 4  CMP $44,X
        {
            ZERO_PAGE_X();
            CMP(addr);
            return 4;
        }
        case 0xD6:  //--NZ 6  DEC $44,X
        {
            ZERO_PAGE_X();
            DEC(addr);
            return 6;
        }
        case 0xD7:  //DCP $44,X
        {
            ZERO_PAGE_X();
            DCP(addr);
            return 6;
        }
        case 0xD8:  //---- 2  CLD
        {
            D = FALSE;
            return 2;
        }
        case 0xD9:  //C-NZ 4+ CMP $4400,Y
        {
            ABSOLUTE_Y();
            CMP(addr);
            return 4;
        }
        case 0xDA:  //NOP
        {
            return 2;
        }
        case 0xDB:  //DCP $4400,Y
        {
            ABSOLUTE_Y();
            DCP(addr);
            return 6;
        }
        case 0xDC:  //TOP
        {
            PC += 2;
            return 4;
        }
        case 0xDD:  //C-NZ 4+ CMP $4400,X
        {
            ABSOLUTE_X();
            CMP(addr);
            return 4;
        }
        case 0xDE:  //--NZ 7  DEC $4400,X
        {
            ABSOLUTE_X();
            DEC(addr);
            return 7;
        }
        case 0xDF:  //DCP $4400,X
        {
            ABSOLUTE_X();
            DCP(addr);
            return 6;
        }
        case 0xE0:  //C-NZ 2  CPX #$44
        {
            IMMEDIATE();
            CPX(addr);
            return 2;
        }
        case 0xE1:  //CVNZ 6  SBC ($44,X)
        {
            INDIRECT_X();
            SBC(addr);
            return 6;
        }
        case 0xE2:  //DOP
        {
            PC++;
            return 3;
        }
        case 0xE3:  //ISC ($44,X)
        {
            INDIRECT_X();
            ISC(addr);
            return 8;
        }
        case 0xE4:  //C-NZ 3  CPX $44
        {
            ZERO_PAGE();
            CPX(addr);
            return 3;
        }
        case 0xE5:  //CVNZ 3  SBC $44
        {
            ZERO_PAGE();
            SBC(addr);
            return 3;
        }
        case 0xE6:  //--NZ 5  INC $44
        {
            ZERO_PAGE();
            INC(addr);
            return 5;
        }
        case 0xE7:  //ISC $44
        {
            ZERO_PAGE();
            ISC(addr);
            return 8;
        }
        case 0xE8:  //--NZ 2  INX
        {
            XR++;
            NZ(XR);
            return 2;
        }
        case 0xE9:  //CVNZ 2  SBC #$44
        {
            IMMEDIATE();
            SBC(addr);
            return 2;
        }
        case 0xEA:  //NOP
        {
            return 2;
        }
        case 0xEB:  //CVNZ 2  SBC #$44
        {
            IMMEDIATE();
            SBC(addr);
            return 2;
        }
        case 0xEC:  //C-NZ 4  CPX $4400
        {
            ABSOLUTE();
            CPX(addr);
            return 4;
        }
        case 0xED:  //CVNZ 4  SBC $4400
        {
            ABSOLUTE();
            SBC(addr);
            return 4;
        }
        case 0xEE:  //--NZ 6  INC $4400
        {
            ABSOLUTE();
            INC(addr);
            return 6;
        }
        case 0xEF:  //ISC $4400
        {
            ABSOLUTE();
            ISC(addr);
            return 8;
        }
        case 0xF0:  //---- 2|3+ BEQ
        {
            IMMEDIATE();
            INT8 value = PEEK(addr);
            if (Z) {
                BRANCH(value);
            }
            return 2 + Z;
        }
        case 0xF1:  //CVNZ 5+ SBC ($44),Y
        {
            INDIRECT_Y();
            SBC(addr);
            return 5;
        }
        case 0xF2:  //---- 1  KIL
        {
            PC--;
            return 1;
        }
        case 0xF3:  //ISC ($44),Y
        {
            ZERO_PAGE();
            ISC(addr);
            return 8;
        }
        case 0xF4:  //DOP
        {
            PC++;
            return 4;
        }
        case 0xF5:  //CVNZ 4  SBC $44,X
        {
            ZERO_PAGE_X();
            SBC(addr);
            return 4;
        }
        case 0xF6:  //--NZ 6  INC $44,X
        {
            ZERO_PAGE_X();
            INC(addr);
            return 6;
        }
        case 0xF7:  //ISC $44,X
        {
            ZERO_PAGE_X();
            ISC(addr);
            return 6;
        }
        case 0xF8:  //SED
        {
            D = TRUE;
            return 2;
        }
        case 0xF9:  //CVNZ 4+ SBC $4400,Y
        {
            ABSOLUTE_Y();
            SBC(addr);
            return 4;
        }
        case 0xFA:  //NOP
        {
            return 2;
        }
        case 0xFB:  //ISC $4400,Y
        {
            ABSOLUTE_Y();
            ISC(addr);
            return 6;
        }
        case 0xFC:  //TOP
        {
            PC += 2;
            return 4;
        }
        case 0xFD:  //CVNZ 4+ SBC $4400,X
        {
            ABSOLUTE_X();
            SBC(addr);
            return 4;
        }
        case 0xFE:  //--NZ 7  INC $4400,X
        {
            ABSOLUTE_X();
            INC(addr);
            return 7;
        }
        case 0xFF:  //ISC $4400,X
        {
            ABSOLUTE_X();
            ISC(addr);
            return 6;
        }
    }
}

void itoa16(char* c, int i, int digits)
{
    int start = (int)strlen(c);
    int mask = (0xF << ((digits-1) << 2));
    char outputs[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    int x;
    for (x = 0; x < digits; x++) {
        c[start+x] = outputs[(i & mask) >> ((digits-x-1) << 2)];
        mask >>= 4;
    }
    c[start+x] = 0;
}

