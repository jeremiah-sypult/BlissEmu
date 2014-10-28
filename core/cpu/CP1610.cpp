
#include <string.h>
#include "CP1610.h"
#include "core/types.h"

#define MAX(v1, v2) (v1 > v2 ? v1 : v2)

#pragma warning(disable:4786)    // Suppress STL debug info > 255 chars messages

const CHAR* REGISTER_NAMES[8] = { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7" };
CHAR STATUS_BITS[8] = { "SZOCIDN" };

CP1610::CP1610(MemoryBus* m, UINT16 resetAddress,
        UINT16 interruptAddress)
    : Processor("CP1610"),
      memoryBus(m),
      resetAddress(resetAddress),
      interruptAddress(interruptAddress)
{
}

INT32 CP1610::getClockSpeed() {
    return 3579545;
}

void CP1610::resetProcessor()
{
    //the four external lines
    ext = 0;

    pinOut[CP1610_PIN_OUT_BUSAK]->isHigh = TRUE;
    interruptible = FALSE;
    S = C = O = Z = I = D = FALSE;
    for (INT32 i = 0; i < 7; i++)
        r[i] = 0;
    r[7] = resetAddress;
}

/**
 * This method ticks the CPU and returns the number of cycles that were
 * used up, indicating to the main emulation loop when the CPU will need
 * to be ticked again.
 */
INT32 CP1610::tick(INT32 minimum)
{
    int usedCycles = 0;
    do {
        if (interruptible) {
            if (!pinIn[CP1610_PIN_IN_BUSRQ]->isHigh) {
                pinOut[CP1610_PIN_OUT_BUSAK]->isHigh = pinIn[CP1610_PIN_IN_BUSRQ]->isHigh;
                return MAX((usedCycles<<2), minimum);
            }

            if (I && !pinIn[CP1610_PIN_IN_INTRM]->isHigh) {
                pinIn[CP1610_PIN_IN_INTRM]->isHigh = TRUE;
                interruptible = false;
                memoryBus->poke(r[6], r[7]);
                r[6]++;
                r[7] = interruptAddress;
                usedCycles += 7;
                if ((usedCycles << 2) >= minimum)
                    return (usedCycles<<2);
            }
        }

        //do the next instruction
        usedCycles += decode(memoryBus->peek(r[7]));
    } while ((usedCycles<<2) < minimum);

    return (usedCycles<<2);
}

#ifdef DEVELOPER_VERSION

#define FORMAT_IMPLIED            "%04X:%04X  %-4s           "
#define FORMAT_REGISTER           "%04X:%04X  %-4s R%i        "
#define FORMAT_REGISTER_DOUBLE    "%04X:%04X  %-4s R%i, 2     "
#define FORMAT_REGISTER_REGISTER  "%04X:%04X  %-4s R%i, R%i    "
#define FORMAT_ADDRESS            "%04X:%04X  %-4s $%04X     "
#define FORMAT_ADDRESS_REGISTER   "%04X:%04X  %-4s $%04X, R%i "
#define FORMAT_IMMEDIATE          "%04X:%04X  %-4s #$%04X    "
#define FORMAT_IMMEDIATE_REGISTER "%04X:%04X  %-4s #$%04X, R%i"

const UINT32 debugItemCount = 17;
const CHAR* debugItemNames[debugItemCount] = {
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7/PC",
    "S",
    "C",
    "O",
    "Z",
    "I",
    "D",
    "INTRM",
    "BUSRQ",
    "BUSAK",
};

UINT32 CP1610::getDebugItemCount()
{
    return debugItemCount;
}

const CHAR* CP1610::getDebugItemName(UINT32 i)
{
    return debugItemNames[i];
}

const UINT32 CP1610::getDebugItemValue(UINT32 i)
{
    switch (i) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            return r[i];
        case 8:
            return S;
        case 9:
            return C;
        case 10:
            return O;
        case 11:
            return Z;
        case 12:
            return I;
        case 13:
            return D;
        case 14:
            return pinIn[CP1610_PIN_IN_INTRM]->isHigh;
        case 15:
            return pinIn[CP1610_PIN_IN_BUSRQ]->isHigh;
        case 16:
            return pinOut[CP1610_PIN_OUT_BUSAK]->isHigh;
    }
    return 0;
}

UINT32 CP1610::getProgramCounter()
{
    if (I && interruptible && !pinIn[CP1610_PIN_IN_INTRM]->isHigh)
        return interruptAddress;
    else
        return r[7];
}

//text output
UINT32 CP1610::decode(CHAR description[256], UINT32 memoryLocation)
{
    UINT16 op = memoryBus->peek(memoryLocation);

    CHAR* opCode = "";
    CHAR* opDisplayFormat = FORMAT_IMPLIED;
    UINT16 read = 0;
    UINT16 param1 = 0;
    UINT16 param2 = 0;
    UINT8 opCodeSize = 1;

    switch (op & 0x03C0) {
        case 0x0000:
            switch (op & 0x0038) {
                case 0x0000:
                    switch (op & 0x0007) {
                        case 0x0000:
                            opCode = "HLT";
                        break;
                        case 0x0001:
                            opCode = "SDBD";
                        break;
                        case 0x0002:
                            opCode = "EIS";
                        break;
                        case 0x0003:
                            opCode = "DIS";
                        break;
                        case 0x0004:
                        {
                            read = memoryBus->peek(memoryLocation+1);
                            BOOL interrupt = (read & 0x0003);
                            param1 = ((read & 0x00FC) << 8);
                            param2 = ((read & 0x0300) >> 8)+4;
                            read = memoryBus->peek(memoryLocation+2);
                            param1 |= read & 0x03FF;
                            if (param2 == 7) {
                                opDisplayFormat = FORMAT_ADDRESS;
                                if (interrupt == 0)
                                    opCode = "J";
                                else if (interrupt == 1)
                                    opCode = "JE";
                                else if (interrupt == 2)
                                    opCode = "JD";
                                else {
                                    opCode = "HLT";
                                    opDisplayFormat = FORMAT_IMPLIED;
                                }
                            }
                            else {
                                opDisplayFormat = FORMAT_ADDRESS_REGISTER;
                                if (interrupt == 0)
                                    opCode = "JSR";
                                else if (interrupt == 1)
                                    opCode = "JSRE";
                                else if (interrupt == 2)
                                    opCode = "JSRD";
                                else {
                                    opCode = "HLT";
                                    opDisplayFormat = FORMAT_IMPLIED;
                                }
                            }
                            opCodeSize = 3;
                        }
                        break;
                        case 0x0005:
                            opCode = "TCI";
                        break;
                        case 0x0006:
                            opCode = "CLRC";
                        break;
                        case 0x0007:
                            opCode = "SETC";
                        break;
                    }
                break;
                case 0x0008:
                    opCode = "INCR";
                    opDisplayFormat = FORMAT_REGISTER;
                    param1 = (op & 0x0007);
                break;
                case 0x0010:
                    opCode = "DECR";
                    opDisplayFormat = FORMAT_REGISTER;
                    param1 = (op & 0x0007);
                break;
                case 0x0018:
                    opCode = "COMR";
                    opDisplayFormat = FORMAT_REGISTER;
                    param1 = (op & 0x0007);
                break;
                case 0x0020:
                    opCode = "NEGR";
                    opDisplayFormat = FORMAT_REGISTER;
                    param1 = (op & 0x0007);
                break;
                case 0x0028:
                    opCode = "ADCR";
                    opDisplayFormat = FORMAT_REGISTER;
                    param1 = (op & 0x0007);
                break;
                case 0x0030:
                    if (op < 34) {
                        opCode = "GSWD";
                        param1 = op & 0x03;
                        opDisplayFormat = FORMAT_REGISTER;
                    }
                    else if (op < 36) {
                        opCode = "NOP";
                    }
                    else {
                        opCode = "SIN";
                    }
                break;
                case 0x0038:
                    opDisplayFormat = FORMAT_REGISTER;
                    param1 = (op & 0x0007);
                    opCode = "RSWD";
                break;
            }
        break;
        case 0x0040:
            if (op & 0x0004)
                opDisplayFormat = FORMAT_REGISTER_DOUBLE;
            else
                opDisplayFormat = FORMAT_REGISTER;

            param1 = (op & 0x0003);
            switch (op & 0x0038) {
                case 0x0000:
                    opCode = "SWAP";
                break;
                case 0x0008:
                    opCode = "SLL";
                break;
                case 0x0010:
                    opCode = "RLC";
                break;
                case 0x0018:
                    opCode = "SLLC";
                break;
                case 0x0020:
                    opCode = "SLR";
                break;
                case 0x0028:
                    opCode = "SAR";
                break;
                case 0x0030:
                    opCode = "RRC";
                break;
                case 0x0038:
                    opCode = "SARC";
                break;
            }
        break;
        case 0x0080:
            opCode = "MOVR";
            opDisplayFormat = FORMAT_REGISTER_REGISTER;
            param1 = (op & 0x0038) >> 3;
            param2 = (op & 0x0007);
        break;
        case 0x00C0:
            opCode = "ADDR";
            opDisplayFormat = FORMAT_REGISTER_REGISTER;
            param1 = (op & 0x0038) >> 3;
            param2 = (op & 0x0007);
        break;
        case 0x0100:
            opCode = "SUBR";
            opDisplayFormat = FORMAT_REGISTER_REGISTER;
            param1 = (op & 0x0038) >> 3;
            param2 = (op & 0x0007);
        break;
        case 0x0140:
            opCode = "CMPR";
            opDisplayFormat = FORMAT_REGISTER_REGISTER;
            param1 = (op & 0x0038) >> 3;
            param2 = (op & 0x0007);
        break;
        case 0x0180:
            opCode = "ANDR";
            opDisplayFormat = FORMAT_REGISTER_REGISTER;
            param1 = (op & 0x0038) >> 3;
            param2 = (op & 0x0007);
        break;
        case 0x01C0:
            opCode = "XORR";
            opDisplayFormat = FORMAT_REGISTER_REGISTER;
            param1 = (op & 0x0038) >> 3;
            param2 = (op & 0x0007);
        break;
        case 0x0200:
            opDisplayFormat = FORMAT_ADDRESS;
            if (op & 0x0020)
                param1 = memoryLocation-memoryBus->peek(memoryLocation+1)+1;
            else
                param1 = memoryLocation+memoryBus->peek(memoryLocation+1)+3;
            
            if (op & 0x0010)
                opCode = "BEXT";
            else {
                switch (op & 0x000F) {
                    case 0x0000:
                        opCode = "B";
                    break;
                    case 0x0001:
                        opCode = "BC";
                    break;
                    case 0x0002:
                        opCode = "BOV";
                    break;
                    case 0x0003:
                        opCode = "BPL";
                    break;
                    case 0x0004:
                        opCode = "BZE";
                    break;
                    case 0x0005:
                        opCode = "BLT";
                    break;
                    case 0x0006:
                        opCode = "BLE";
                    break;
                    case 0x0007:
                        opCode = "BUSC";
                    break;
                    case 0x0008:
                        opCode = "NOPP";
                    break;
                    case 0x0009:
                        opCode = "BNC";
                    break;
                    case 0x000A:
                        opCode = "BNOV";
                    break;
                    case 0x000B:
                        opCode = "BMI";
                    break;
                    case 0x000C:
                        opCode = "BNEQ";
                    break;
                    case 0x000D:
                        opCode = "BGE";
                    break;
                    case 0x000E:
                        opCode = "BGT";
                    break;
                    case 0x000F:
                        opCode = "BESC";
                    break;
                }
            }
            opCodeSize = 2;
        break;
        default:
            if (!(op & 0x0038)) {
                //direct mode
                opDisplayFormat = FORMAT_ADDRESS;
                param1 = memoryBus->peek(memoryLocation+1);
                switch (op & 0x03C0) {
                    case 0x0240:
                        opCode = "MVO";
                    break;
                    case 0x0280:
                        opCode = "MVI";
                    break;
                    case 0x02C0:
                        opCode = "ADD";
                    break;
                    case 0x0300:
                        opCode = "SUB";
                    break;
                    case 0x0340:
                        opCode = "CMP";
                    break;
                    case 0x0380:
                        opCode = "AND";
                    break;
                    case 0x03C0:
                        opCode = "XOR";
                    break;
                }
                opCodeSize = 2;
            }
            else if ((op & 0x0038) == 0x0038) {
                //immediate mode
                opDisplayFormat = FORMAT_IMMEDIATE_REGISTER;
                if (D)
                    param1 = (memoryBus->peek(memoryLocation+1) & 0xFF) |
                            ((memoryBus->peek(memoryLocation+2) & 0xFF) << 8);
                else
                    param1 = memoryBus->peek(memoryLocation+1);
                param2 = (op & 0x0007);
                switch (op & 0x03C0) {
                    case 0x0240:
                        opCode = "MVOI";
                    break;
                    case 0x0280:
                        opCode = "MVII";
                    break;
                    case 0x02C0:
                        opCode = "ADDI";
                    break;
                    case 0x0300:
                        opCode = "SUBI";
                    break;
                    case 0x0340:
                        opCode = "CMPI";
                    break;
                    case 0x0380:
                        opCode = "ANDI";
                    break;
                    case 0x03C0:
                        opCode = "XORI";
                    break;
                }
                opCodeSize = 3;
            }
            else {
                //indirect mode
                opDisplayFormat = FORMAT_REGISTER_REGISTER;
                param1 = (op & 0x0038) >> 3;
                param2 = (op & 0x0007);
                switch (op & 0x03C0) {
                    case 0x0240:
                        opCode = "MVO@";
                    break;
                    case 0x0280:
                        opCode = "MVI@";
                    break;
                    case 0x02C0:
                        opCode = "ADD@";
                    break;
                    case 0x0300:
                        opCode = "SUB@";
                    break;
                    case 0x0340:
                        opCode = "CMP@";
                    break;
                    case 0x0380:
                        opCode = "AND@";
                    break;
                    case 0x03C0:
                        opCode = "XOR@";
                    break;
                }
            }
        break;
    }

    char tmp[256];
    sprintf(tmp, opDisplayFormat, memoryLocation, op, opCode, param1,
            param2);
    sprintf(description, "%s %04X %04X %04X %04X %04X %04X %04X %04X %c%c%c%c%c%c",
            tmp,
            r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7],
            (S ? 'S' : '-'), (C ? 'C' : '-'), (O ? 'O' : '-'), (Z ? 'Z' : '-'),
            (I ? 'I' : '-'), (D ? 'D' : '-'));

    return opCodeSize;
}
#endif

UINT16 CP1610::getIndirect(UINT16 registerNum)
{
    UINT16 value;
    if (registerNum == 6) {
        r[6]--;
        value = memoryBus->peek(r[6]);
        if (D)
            value = (value & 0xFF) | ((memoryBus->peek(r[6]) & 0xFF) << 8);
    }
    else {
        value = memoryBus->peek(r[registerNum]);
        if (registerNum & 0x04)
            r[registerNum]++;
        if (D) {
            value = (value & 0xFF) |
                    (memoryBus->peek(r[registerNum]) & 0xFF) << 8;
            if (registerNum & 0x04)
                r[registerNum]++;
        }
    }

    return value;
}

INT32 CP1610::HLT() {
    return 1;
}

INT32 CP1610::SDBD() {
    r[7]++;
    interruptible = FALSE;

    D = TRUE;

    return 4;
}

INT32 CP1610::EIS() {
    r[7]++;
    interruptible = FALSE;

    I = TRUE;

	D = FALSE;
    return 4;
}

INT32 CP1610::DIS() {
    r[7]++;
    interruptible = FALSE;

    I = FALSE;

	D = FALSE;
    return 4;
}

INT32 CP1610::TCI() {
    r[7]++;
    interruptible = FALSE;

    //What should this really do?

	D = FALSE;
    return 4;
}

INT32 CP1610::CLRC() {
    r[7]++;
    interruptible = FALSE;

    C = FALSE;

	D = FALSE;
    return 4;
}

INT32 CP1610::SETC() {
    r[7]++;
    interruptible = FALSE;

    C = TRUE;

	D = FALSE;
    return 4;
}

INT32 CP1610::J(UINT16 target) {
    r[7] = target;
    interruptible = TRUE;

	D = FALSE;
    return 12;
}

INT32 CP1610::JSR(UINT16 registerNum, UINT16 target) {
    r[registerNum] = r[7]+3;
    r[7] = target;
    interruptible = TRUE;

	D = FALSE;
    return 12;
}

INT32 CP1610::JE(UINT16 target) {
    I = TRUE;
    r[7] = target;
    interruptible = TRUE;

	D = FALSE;
    return 12;
}

INT32 CP1610::JSRE(UINT16 registerNum, UINT16 target) {
    I = TRUE;
    r[registerNum] = r[7]+3;
    r[7] = target;
    interruptible = TRUE;

	D = FALSE;
    return 12;
}

INT32 CP1610::JD(UINT16 target) {
    I = FALSE;
    r[7] = target;
    interruptible = TRUE;

	D = FALSE;
    return 12;
}

INT32 CP1610::JSRD(UINT16 registerNum, UINT16 target) {
    I = FALSE;
    r[registerNum] = r[7]+3;
    r[7] = target;
    interruptible = TRUE;

	D = FALSE;
    return 12;
}

INT32 CP1610::INCR(UINT16 registerNum) {
    r[7]++;
    interruptible = TRUE;

    UINT16 newValue = r[registerNum]+1;
    S = !!(newValue & 0x8000);
    Z = !newValue;
    r[registerNum] = newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::DECR(UINT16 registerNum) {
    r[7]++;
    interruptible = TRUE;

    UINT16 newValue = r[registerNum]-1;
    S = !!(newValue & 0x8000);
    Z = !newValue;
    r[registerNum] = newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::NEGR(UINT16 registerNum) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = r[registerNum];
    UINT32 newValue = (op1 ^ 0xFFFF) + 1;
    C = !!(newValue & 0x10000);
    O = !!(newValue & op1 & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[registerNum] = (UINT16)newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::ADCR(UINT16 registerNum) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = r[registerNum];
    UINT16 op2 = (C ? 1 : 0);
    UINT32 newValue = op1 + op2;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & ~(op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[registerNum] = (UINT16)newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::RSWD(UINT16 registerNum) {
    r[7]++;
    interruptible = TRUE;

    UINT16 value = r[registerNum];
    S = !!(value & 0x0080);
    Z = !!(value & 0x0040);
    O = !!(value & 0x0020);
    C = !!(value & 0x0010);

	D = FALSE;
    return 6;
}

INT32 CP1610::GSWD(UINT16 registerNum) {
    r[7]++;
    interruptible = TRUE;

    UINT16 value = ((S ? 1 : 0) << 7) | ((Z ? 1 : 0) << 6) | 
            ((O ? 1 : 0) << 5) | ((C ? 1 : 0) << 4);
    value |= (value << 8);
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::NOP(UINT16) {
    r[7]++;
    interruptible = TRUE;

	D = FALSE;
    return 6;
}

INT32 CP1610::SIN(UINT16) {
    r[7]++;
    interruptible = TRUE;

    //TODO: does SIN need to do anything?!

	D = FALSE;
    return 6;
}

INT32 CP1610::SWAP_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    value = ((value & 0xFF00) >> 8) | ((value & 0xFF) << 8);
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::SWAP_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum] & 0xFF;
    value |= (value << 8);
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::COMR(UINT16 registerNum) {
    r[7]++;
    interruptible = TRUE;

    UINT16 value = r[registerNum] ^ 0xFFFF;
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::SLL_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum] << 1;
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::SLL_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum] << 2;
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::RLC_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    UINT16 carry = (C ? 1 : 0);
    C = !!(value & 0x8000);
    value = (value << 1) | carry;
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::RLC_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    UINT16 carry = (C ? 1 : 0);
    UINT16 overflow = (O ? 1 : 0);
    C = !!(value & 0x8000);
    O = !!(value & 0x4000);
    value = (value << 2) | (carry << 1) | overflow;
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::SLLC_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    C = !!(value & 0x8000);
    value <<= 1;
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::SLLC_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    C = !!(value & 0x8000);
    O = !!(value & 0x4000);
    value <<= 2;
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::SLR_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum] >> 1;
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::SLR_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum] >> 2;
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::SAR_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    value = (value >> 1) | (value & 0x8000);
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::SAR_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    UINT16 s  = value & 0x8000;
    value = (value >> 2) | s | (s >> 1);
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::RRC_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    UINT16 carry = (C ? 1 : 0);
    C = !!(value & 0x0001);
    value = (value >> 1) | (carry << 15);
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::RRC_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    UINT16 carry = (C ? 1 : 0);
    UINT16 overflow = (O ? 1 : 0);
    C = !!(value & 0x0001);
    O = !!(value & 0x0002);
    value = (value >> 2) | (carry << 14) | (overflow << 15);
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::SARC_1(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    C = !!(value & 0x0001);
    value = (value >> 1) | (value & 0x8000);
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 6;
}

INT32 CP1610::SARC_2(UINT16 registerNum) {
    r[7]++;
    interruptible = FALSE;

    UINT16 value = r[registerNum];
    C = !!(value & 0x0001);
    O = !!(value & 0x0002);
    UINT16 s = value & 0x8000;
    value = (value >> 2) | s | (s >> 1);
    S = !!(value & 0x0080);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 8;
}

INT32 CP1610::MOVR(UINT16 sourceReg, UINT16 destReg) {
    r[7]++;
    interruptible = TRUE;

    UINT16 value = r[sourceReg];
    S = !!(value & 0x8000);
    Z = !value;
    r[destReg] = value;

	D = FALSE;
    return (destReg >= 6 ? 7 : 6);
}

INT32 CP1610::ADDR(UINT16 sourceReg, UINT16 destReg) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = r[sourceReg];
    UINT16 op2 = r[destReg];
    UINT32 newValue = op1 + op2;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & ~(op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[destReg] = (UINT16)newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::SUBR(UINT16 sourceReg, UINT16 destReg) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = r[sourceReg];
    UINT16 op2 = r[destReg];
    UINT32 newValue = op2 + (0xFFFF ^ op1) + 1;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & (op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[destReg] = (UINT16)newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::CMPR(UINT16 sourceReg, UINT16 destReg) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = r[sourceReg];
    UINT16 op2 = r[destReg];
    UINT32 newValue = op2 + (0xFFFF ^ op1) + 1;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & (op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);

	D = FALSE;
    return 6;
}

INT32 CP1610::ANDR(UINT16 sourceReg, UINT16 destReg) {
    r[7]++;
    interruptible = TRUE;

    UINT16 newValue = r[destReg] & r[sourceReg];
    S = !!(newValue & 0x8000);
    Z = !newValue;
    r[destReg] = newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::XORR(UINT16 sourceReg, UINT16 destReg) {
    r[7]++;
    interruptible = TRUE;

    UINT16 newValue = r[destReg] ^ r[sourceReg];
    S = !!(newValue & 0x8000);
    Z = !newValue;
    r[destReg] = newValue;

	D = FALSE;
    return 6;
}

INT32 CP1610::BEXT(UINT16 condition, INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (ext == condition) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::B(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    r[7] = (UINT16)(r[7] + displacement);

	D = FALSE;
    return 9;
}

INT32 CP1610::NOPP(INT16) {
    r[7] += 2;
    interruptible = TRUE;

	D = FALSE;
    return 7;
}

INT32 CP1610::BC(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (C) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BNC(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (!C) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BOV(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (O) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BNOV(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (!O) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BPL(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (!S) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BMI(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (S) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BEQ(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (Z) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BNEQ(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (!Z) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BLT(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (S != O) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BGE(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (S == O) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BLE(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (Z || (S != O)) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BGT(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (!(Z || (S != O))) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BUSC(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (C != S) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::BESC(INT16 displacement) {
    r[7] += 2;
    interruptible = TRUE;

    if (C == S) {
        r[7] = (UINT16)(r[7] + displacement);
		D = FALSE;
        return 9;
    }

	D = FALSE;
    return 7;
}

INT32 CP1610::MVO(UINT16 registerNum, UINT16 address) {
    r[7] += 2;
    interruptible = FALSE;

    memoryBus->poke(address, r[registerNum]);

	D = FALSE;
    return 11;
}

INT32 CP1610::MVO_ind(UINT16 registerWithAddress, UINT16 registerToMove) {
    r[7]++;
    interruptible = FALSE;

    memoryBus->poke(r[registerWithAddress], r[registerToMove]);

    //if the register number is 4-7, increment it
    if (registerWithAddress & 0x04)
        r[registerWithAddress]++;

	D = FALSE;
    return 9;
}

INT32 CP1610::MVI(UINT16 address, UINT16 registerNum) {
    r[7] += 2;
    interruptible = TRUE;

    r[registerNum] = memoryBus->peek(address);

	D = FALSE;
    return 10;
}

INT32 CP1610::MVI_ind(UINT16 registerWithAddress, UINT16 registerToReceive) {
    r[7]++;
    interruptible = TRUE;

    r[registerToReceive] = getIndirect(registerWithAddress);

	D = FALSE;
    return (D ? 10 : (registerWithAddress == 6 ? 11 : 8));
}

INT32 CP1610::ADD(UINT16 address, UINT16 registerNum) {
    r[7] += 2;
    interruptible = TRUE;

    UINT16 op1 = memoryBus->peek(address);
    UINT16 op2 = r[registerNum];
    UINT32 newValue = op1 + op2;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & ~(op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[registerNum] = (UINT16)newValue;

	D = FALSE;
    return 10;
}

INT32 CP1610::ADD_ind(UINT16 registerWithAddress, UINT16 registerToReceive) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = getIndirect(registerWithAddress);
    UINT16 op2 = r[registerToReceive];
    UINT32 newValue = op1 + op2;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & ~(op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[registerToReceive] = (UINT16)newValue;
    
	D = FALSE;
    return (D ? 10 : (registerWithAddress == 6 ? 11 : 8));
}

INT32 CP1610::SUB(UINT16 address, UINT16 registerNum) {
    r[7] += 2;
    interruptible = TRUE;

    UINT16 op1 = memoryBus->peek(address);
    UINT16 op2 = r[registerNum];
    UINT32 newValue = op2 + (0xFFFF ^ op1) + 1;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & (op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[registerNum] = (UINT16)newValue;

	D = FALSE;
    return 10;
}

INT32 CP1610::SUB_ind(UINT16 registerWithAddress, UINT16 registerToReceive) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = getIndirect(registerWithAddress);
    UINT16 op2 = r[registerToReceive];
    UINT32 newValue = op2 + (0xFFFF ^ op1) + 1;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & (op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);
    r[registerToReceive] = (UINT16)newValue;

	D = FALSE;
    return (D ? 10 : (registerWithAddress == 6 ? 11 : 8));
}

INT32 CP1610::CMP(UINT16 address, UINT16 registerNum) {
    r[7] += 2;
    interruptible = TRUE;

    UINT16 op1 = memoryBus->peek(address);
    UINT16 op2 = r[registerNum];
    UINT32 newValue = op2 + (0xFFFF ^ op1) + 1;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & (op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);

	D = FALSE;
    return 10;
}

INT32 CP1610::CMP_ind(UINT16 registerWithAddress, UINT16 registerToReceive) {
    r[7]++;
    interruptible = TRUE;

    UINT16 op1 = getIndirect(registerWithAddress);
    UINT16 op2 = r[registerToReceive];
    UINT32 newValue = op2 + (0xFFFF ^ op1) + 1;
    C = !!(newValue & 0x10000);
    O = !!((op2 ^ newValue) & (op1 ^ op2) & 0x8000);
    S = !!(newValue & 0x8000);
    Z = !(newValue & 0xFFFF);

	D = FALSE;
    return (D ? 10 : (registerWithAddress == 6 ? 11 : 8));
}

INT32 CP1610::AND(UINT16 address, UINT16 registerNum) {
    r[7] += 2;
    interruptible = TRUE;

    UINT16 value = memoryBus->peek(address) & r[registerNum];
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 10;
}

INT32 CP1610::AND_ind(UINT16 registerWithAddress, UINT16 registerToReceive) {
    r[7]++;
    interruptible = TRUE;

    UINT16 value = getIndirect(registerWithAddress) & r[registerToReceive];
    S = !!(value & 0x8000);
    Z = !value;
    r[registerToReceive] = value;
    
	D = FALSE;
    return (D ? 10 : (registerWithAddress == 6 ? 11 : 8));
}

INT32 CP1610::XOR(UINT16 address, UINT16 registerNum) {
    r[7] += 2;
    interruptible = TRUE;

    UINT16 value = memoryBus->peek(address) ^ r[registerNum];
    S = !!(value & 0x8000);
    Z = !value;
    r[registerNum] = value;

	D = FALSE;
    return 10;
}

INT32 CP1610::XOR_ind(UINT16 registerWithAddress, UINT16 registerToReceive) {
    r[7]++;
    interruptible = TRUE;

    UINT16 value = getIndirect(registerWithAddress) ^ r[registerToReceive];
    S = !!(value & 0x8000);
    Z = !value;
    r[registerToReceive] = value;
    
	D = FALSE;
    return (D ? 10 : (registerWithAddress == 6 ? 11 : 8));
}

INT32 CP1610::decode(UINT16 op)
{
    switch (op & 0x3FF) {
        case 0x0000:
            return HLT();
        case 0x0001:
            return SDBD();
        case 0x0002:
            return EIS();
        case 0x0003:
            return DIS();
        case 0x0004:
			{
            int read = memoryBus->peek((UINT16)(r[7] + 1));
            int reg = ((read & 0x0300) >> 8);
            int interrupt = (read & 0x0003);
            UINT16 target = (UINT16)((read & 0x00FC) << 8);
            read = memoryBus->peek((UINT16)(r[7] + 2));
            target |= (UINT16)(read & 0x03FF);
            if (reg == 3) {
                if (interrupt == 0)
                    return J(target);
                else if (interrupt == 1)
                    return JE(target);
                else if (interrupt == 2)
                    return JD(target);
                else
                    return HLT(); //invalid opcode
            }
            else {
                if (interrupt == 0)
                    return JSR((UINT16)(reg + 4), target);
                else if (interrupt == 1)
                    return JSRE((UINT16)(reg + 4), target);
                else if (interrupt == 2)
                    return JSRD((UINT16)(reg + 4), target);
                else
                    return HLT(); //invalid opcode
            }
			}
        case 0x0005:
            return TCI();
        case 0x0006:
            return CLRC();
        case 0x0007:
            return SETC();
        case 0x0008:
            return INCR(0);
        case 0x0009:
            return INCR(1);
        case 0x000A:
            return INCR(2);
        case 0x000B:
            return INCR(3);
        case 0x000C:
            return INCR(4);
        case 0x000D:
            return INCR(5);
        case 0x000E:
            return INCR(6);
        case 0x000F:
            return INCR(7);
        case 0x0010:
            return DECR(0);

        case 0x0011:
            return DECR(1);

        case 0x0012:
            return DECR(2);

        case 0x0013:
            return DECR(3);

        case 0x0014:
            return DECR(4);

        case 0x0015:
            return DECR(5);

        case 0x0016:
            return DECR(6);

        case 0x0017:
            return DECR(7);

        case 0x0018:
            return COMR(0);

        case 0x0019:
            return COMR(1);

        case 0x001A:
            return COMR(2);

        case 0x001B:
            return COMR(3);

        case 0x001C:
            return COMR(4);

        case 0x001D:
            return COMR(5);

        case 0x001E:
            return COMR(6);

        case 0x001F:
            return COMR(7);

        case 0x0020:
            return NEGR(0);

        case 0x0021:
            return NEGR(1);

        case 0x0022:
            return NEGR(2);

        case 0x0023:
            return NEGR(3);

        case 0x0024:
            return NEGR(4);

        case 0x0025:
            return NEGR(5);

        case 0x0026:
            return NEGR(6);

        case 0x0027:
            return NEGR(7);

        case 0x0028:
            return ADCR(0);

        case 0x0029:
            return ADCR(1);

        case 0x002A:
            return ADCR(2);

        case 0x002B:
            return ADCR(3);

        case 0x002C:
            return ADCR(4);

        case 0x002D:
            return ADCR(5);

        case 0x002E:
            return ADCR(6);

        case 0x002F:
            return ADCR(7);

        case 0x0030:
            return GSWD(0);

        case 0x0031:
            return GSWD(1);

        case 0x0032:
            return GSWD(2);

        case 0x0033:
            return GSWD(3);

        case 0x0034:
            return NOP(0);

        case 0x0035:
            return NOP(1);

        case 0x0036:
            return SIN(0);

        case 0x0037:
            return SIN(1);

        case 0x0038:
            return RSWD(0);

        case 0x0039:
            return RSWD(1);

        case 0x003A:
            return RSWD(2);

        case 0x003B:
            return RSWD(3);

        case 0x003C:
            return RSWD(4);

        case 0x003D:
            return RSWD(5);

        case 0x003E:
            return RSWD(6);

        case 0x003F:
            return RSWD(7);

        case 0x0040:
            return SWAP_1(0);

        case 0x0041:
            return SWAP_1(1);

        case 0x0042:
            return SWAP_1(2);

        case 0x0043:
            return SWAP_1(3);

        case 0x0044:
            return SWAP_2(0);

        case 0x0045:
            return SWAP_2(1);

        case 0x0046:
            return SWAP_2(2);

        case 0x0047:
            return SWAP_2(3);

        case 0x0048:
            return SLL_1(0);

        case 0x0049:
            return SLL_1(1);

        case 0x004A:
            return SLL_1(2);

        case 0x004B:
            return SLL_1(3);

        case 0x004C:
            return SLL_2(0);

        case 0x004D:
            return SLL_2(1);

        case 0x004E:
            return SLL_2(2);

        case 0x004F:
            return SLL_2(3);

        case 0x0050:
            return RLC_1(0);

        case 0x0051:
            return RLC_1(1);

        case 0x0052:
            return RLC_1(2);

        case 0x0053:
            return RLC_1(3);

        case 0x0054:
            return RLC_2(0);

        case 0x0055:
            return RLC_2(1);

        case 0x0056:
            return RLC_2(2);

        case 0x0057:
            return RLC_2(3);

        case 0x0058:
            return SLLC_1(0);

        case 0x0059:
            return SLLC_1(1);

        case 0x005A:
            return SLLC_1(2);

        case 0x005B:
            return SLLC_1(3);

        case 0x005C:
            return SLLC_2(0);

        case 0x005D:
            return SLLC_2(1);

        case 0x005E:
            return SLLC_2(2);

        case 0x005F:
            return SLLC_2(3);

        case 0x0060:
            return SLR_1(0);

        case 0x0061:
            return SLR_1(1);

        case 0x0062:
            return SLR_1(2);

        case 0x0063:
            return SLR_1(3);

        case 0x0064:
            return SLR_2(0);

        case 0x0065:
            return SLR_2(1);

        case 0x0066:
            return SLR_2(2);

        case 0x0067:
            return SLR_2(3);

        case 0x0068:
            return SAR_1(0);

        case 0x0069:
            return SAR_1(1);

        case 0x006A:
            return SAR_1(2);

        case 0x006B:
            return SAR_1(3);

        case 0x006C:
            return SAR_2(0);

        case 0x006D:
            return SAR_2(1);

        case 0x006E:
            return SAR_2(2);

        case 0x006F:
            return SAR_2(3);

        case 0x0070:
            return RRC_1(0);

        case 0x0071:
            return RRC_1(1);

        case 0x0072:
            return RRC_1(2);

        case 0x0073:
            return RRC_1(3);

        case 0x0074:
            return RRC_2(0);

        case 0x0075:
            return RRC_2(1);

        case 0x0076:
            return RRC_2(2);

        case 0x0077:
            return RRC_2(3);

        case 0x0078:
            return SARC_1(0);

        case 0x0079:
            return SARC_1(1);

        case 0x007A:
            return SARC_1(2);

        case 0x007B:
            return SARC_1(3);

        case 0x007C:
            return SARC_2(0);

        case 0x007D:
            return SARC_2(1);

        case 0x007E:
            return SARC_2(2);

        case 0x007F:
            return SARC_2(3);

        case 0x0080:
            return MOVR(0, 0);

        case 0x0081:
            return MOVR(0, 1);

        case 0x0082:
            return MOVR(0, 2);

        case 0x0083:
            return MOVR(0, 3);

        case 0x0084:
            return MOVR(0, 4);

        case 0x0085:
            return MOVR(0, 5);

        case 0x0086:
            return MOVR(0, 6);

        case 0x0087:
            return MOVR(0, 7);

        case 0x0088:
            return MOVR(1, 0);

        case 0x0089:
            return MOVR(1, 1);

        case 0x008A:
            return MOVR(1, 2);

        case 0x008B:
            return MOVR(1, 3);

        case 0x008C:
            return MOVR(1, 4);

        case 0x008D:
            return MOVR(1, 5);

        case 0x008E:
            return MOVR(1, 6);

        case 0x008F:
            return MOVR(1, 7);

        case 0x0090:
            return MOVR(2, 0);

        case 0x0091:
            return MOVR(2, 1);

        case 0x0092:
            return MOVR(2, 2);

        case 0x0093:
            return MOVR(2, 3);

        case 0x0094:
            return MOVR(2, 4);

        case 0x0095:
            return MOVR(2, 5);

        case 0x0096:
            return MOVR(2, 6);

        case 0x0097:
            return MOVR(2, 7);

        case 0x0098:
            return MOVR(3, 0);

        case 0x0099:
            return MOVR(3, 1);

        case 0x009A:
            return MOVR(3, 2);

        case 0x009B:
            return MOVR(3, 3);

        case 0x009C:
            return MOVR(3, 4);

        case 0x009D:
            return MOVR(3, 5);

        case 0x009E:
            return MOVR(3, 6);

        case 0x009F:
            return MOVR(3, 7);

        case 0x00A0:
            return MOVR(4, 0);

        case 0x00A1:
            return MOVR(4, 1);

        case 0x00A2:
            return MOVR(4, 2);

        case 0x00A3:
            return MOVR(4, 3);

        case 0x00A4:
            return MOVR(4, 4);

        case 0x00A5:
            return MOVR(4, 5);

        case 0x00A6:
            return MOVR(4, 6);

        case 0x00A7:
            return MOVR(4, 7);

        case 0x00A8:
            return MOVR(5, 0);

        case 0x00A9:
            return MOVR(5, 1);

        case 0x00AA:
            return MOVR(5, 2);

        case 0x00AB:
            return MOVR(5, 3);

        case 0x00AC:
            return MOVR(5, 4);

        case 0x00AD:
            return MOVR(5, 5);

        case 0x00AE:
            return MOVR(5, 6);

        case 0x00AF:
            return MOVR(5, 7);

        case 0x00B0:
            return MOVR(6, 0);

        case 0x00B1:
            return MOVR(6, 1);

        case 0x00B2:
            return MOVR(6, 2);

        case 0x00B3:
            return MOVR(6, 3);

        case 0x00B4:
            return MOVR(6, 4);

        case 0x00B5:
            return MOVR(6, 5);

        case 0x00B6:
            return MOVR(6, 6);

        case 0x00B7:
            return MOVR(6, 7);

        case 0x00B8:
            return MOVR(7, 0);

        case 0x00B9:
            return MOVR(7, 1);

        case 0x00BA:
            return MOVR(7, 2);

        case 0x00BB:
            return MOVR(7, 3);

        case 0x00BC:
            return MOVR(7, 4);

        case 0x00BD:
            return MOVR(7, 5);

        case 0x00BE:
            return MOVR(7, 6);

        case 0x00BF:
            return MOVR(7, 7);

        case 0x00C0:
            return ADDR(0, 0);

        case 0x00C1:
            return ADDR(0, 1);

        case 0x00C2:
            return ADDR(0, 2);

        case 0x00C3:
            return ADDR(0, 3);

        case 0x00C4:
            return ADDR(0, 4);

        case 0x00C5:
            return ADDR(0, 5);

        case 0x00C6:
            return ADDR(0, 6);

        case 0x00C7:
            return ADDR(0, 7);

        case 0x00C8:
            return ADDR(1, 0);

        case 0x00C9:
            return ADDR(1, 1);

        case 0x00CA:
            return ADDR(1, 2);

        case 0x00CB:
            return ADDR(1, 3);

        case 0x00CC:
            return ADDR(1, 4);

        case 0x00CD:
            return ADDR(1, 5);

        case 0x00CE:
            return ADDR(1, 6);

        case 0x00CF:
            return ADDR(1, 7);

        case 0x00D0:
            return ADDR(2, 0);

        case 0x00D1:
            return ADDR(2, 1);

        case 0x00D2:
            return ADDR(2, 2);

        case 0x00D3:
            return ADDR(2, 3);

        case 0x00D4:
            return ADDR(2, 4);

        case 0x00D5:
            return ADDR(2, 5);

        case 0x00D6:
            return ADDR(2, 6);

        case 0x00D7:
            return ADDR(2, 7);

        case 0x00D8:
            return ADDR(3, 0);

        case 0x00D9:
            return ADDR(3, 1);

        case 0x00DA:
            return ADDR(3, 2);

        case 0x00DB:
            return ADDR(3, 3);

        case 0x00DC:
            return ADDR(3, 4);

        case 0x00DD:
            return ADDR(3, 5);

        case 0x00DE:
            return ADDR(3, 6);

        case 0x00DF:
            return ADDR(3, 7);

        case 0x00E0:
            return ADDR(4, 0);

        case 0x00E1:
            return ADDR(4, 1);

        case 0x00E2:
            return ADDR(4, 2);

        case 0x00E3:
            return ADDR(4, 3);

        case 0x00E4:
            return ADDR(4, 4);

        case 0x00E5:
            return ADDR(4, 5);

        case 0x00E6:
            return ADDR(4, 6);

        case 0x00E7:
            return ADDR(4, 7);

        case 0x00E8:
            return ADDR(5, 0);

        case 0x00E9:
            return ADDR(5, 1);

        case 0x00EA:
            return ADDR(5, 2);

        case 0x00EB:
            return ADDR(5, 3);

        case 0x00EC:
            return ADDR(5, 4);

        case 0x00ED:
            return ADDR(5, 5);

        case 0x00EE:
            return ADDR(5, 6);

        case 0x00EF:
            return ADDR(5, 7);

        case 0x00F0:
            return ADDR(6, 0);

        case 0x00F1:
            return ADDR(6, 1);

        case 0x00F2:
            return ADDR(6, 2);

        case 0x00F3:
            return ADDR(6, 3);

        case 0x00F4:
            return ADDR(6, 4);

        case 0x00F5:
            return ADDR(6, 5);

        case 0x00F6:
            return ADDR(6, 6);

        case 0x00F7:
            return ADDR(6, 7);

        case 0x00F8:
            return ADDR(7, 0);

        case 0x00F9:
            return ADDR(7, 1);

        case 0x00FA:
            return ADDR(7, 2);

        case 0x00FB:
            return ADDR(7, 3);

        case 0x00FC:
            return ADDR(7, 4);

        case 0x00FD:
            return ADDR(7, 5);

        case 0x00FE:
            return ADDR(7, 6);

        case 0x00FF:
            return ADDR(7, 7);

        case 0x0100:
            return SUBR(0, 0);

        case 0x0101:
            return SUBR(0, 1);

        case 0x0102:
            return SUBR(0, 2);

        case 0x0103:
            return SUBR(0, 3);

        case 0x0104:
            return SUBR(0, 4);

        case 0x0105:
            return SUBR(0, 5);

        case 0x0106:
            return SUBR(0, 6);

        case 0x0107:
            return SUBR(0, 7);

        case 0x0108:
            return SUBR(1, 0);

        case 0x0109:
            return SUBR(1, 1);

        case 0x010A:
            return SUBR(1, 2);

        case 0x010B:
            return SUBR(1, 3);

        case 0x010C:
            return SUBR(1, 4);

        case 0x010D:
            return SUBR(1, 5);

        case 0x010E:
            return SUBR(1, 6);

        case 0x010F:
            return SUBR(1, 7);

        case 0x0110:
            return SUBR(2, 0);

        case 0x0111:
            return SUBR(2, 1);

        case 0x0112:
            return SUBR(2, 2);

        case 0x0113:
            return SUBR(2, 3);

        case 0x0114:
            return SUBR(2, 4);

        case 0x0115:
            return SUBR(2, 5);

        case 0x0116:
            return SUBR(2, 6);

        case 0x0117:
            return SUBR(2, 7);

        case 0x0118:
            return SUBR(3, 0);

        case 0x0119:
            return SUBR(3, 1);

        case 0x011A:
            return SUBR(3, 2);

        case 0x011B:
            return SUBR(3, 3);

        case 0x011C:
            return SUBR(3, 4);

        case 0x011D:
            return SUBR(3, 5);

        case 0x011E:
            return SUBR(3, 6);

        case 0x011F:
            return SUBR(3, 7);

        case 0x0120:
            return SUBR(4, 0);

        case 0x0121:
            return SUBR(4, 1);

        case 0x0122:
            return SUBR(4, 2);

        case 0x0123:
            return SUBR(4, 3);

        case 0x0124:
            return SUBR(4, 4);

        case 0x0125:
            return SUBR(4, 5);

        case 0x0126:
            return SUBR(4, 6);

        case 0x0127:
            return SUBR(4, 7);

        case 0x0128:
            return SUBR(5, 0);

        case 0x0129:
            return SUBR(5, 1);

        case 0x012A:
            return SUBR(5, 2);

        case 0x012B:
            return SUBR(5, 3);

        case 0x012C:
            return SUBR(5, 4);

        case 0x012D:
            return SUBR(5, 5);

        case 0x012E:
            return SUBR(5, 6);

        case 0x012F:
            return SUBR(5, 7);

        case 0x0130:
            return SUBR(6, 0);

        case 0x0131:
            return SUBR(6, 1);

        case 0x0132:
            return SUBR(6, 2);

        case 0x0133:
            return SUBR(6, 3);

        case 0x0134:
            return SUBR(6, 4);

        case 0x0135:
            return SUBR(6, 5);

        case 0x0136:
            return SUBR(6, 6);

        case 0x0137:
            return SUBR(6, 7);

        case 0x0138:
            return SUBR(7, 0);

        case 0x0139:
            return SUBR(7, 1);

        case 0x013A:
            return SUBR(7, 2);

        case 0x013B:
            return SUBR(7, 3);

        case 0x013C:
            return SUBR(7, 4);

        case 0x013D:
            return SUBR(7, 5);

        case 0x013E:
            return SUBR(7, 6);

        case 0x013F:
            return SUBR(7, 7);

        case 0x0140:
            return CMPR(0, 0);

        case 0x0141:
            return CMPR(0, 1);

        case 0x0142:
            return CMPR(0, 2);

        case 0x0143:
            return CMPR(0, 3);

        case 0x0144:
            return CMPR(0, 4);

        case 0x0145:
            return CMPR(0, 5);

        case 0x0146:
            return CMPR(0, 6);

        case 0x0147:
            return CMPR(0, 7);

        case 0x0148:
            return CMPR(1, 0);

        case 0x0149:
            return CMPR(1, 1);

        case 0x014A:
            return CMPR(1, 2);

        case 0x014B:
            return CMPR(1, 3);

        case 0x014C:
            return CMPR(1, 4);

        case 0x014D:
            return CMPR(1, 5);

        case 0x014E:
            return CMPR(1, 6);

        case 0x014F:
            return CMPR(1, 7);

        case 0x0150:
            return CMPR(2, 0);

        case 0x0151:
            return CMPR(2, 1);

        case 0x0152:
            return CMPR(2, 2);

        case 0x0153:
            return CMPR(2, 3);

        case 0x0154:
            return CMPR(2, 4);

        case 0x0155:
            return CMPR(2, 5);

        case 0x0156:
            return CMPR(2, 6);

        case 0x0157:
            return CMPR(2, 7);

        case 0x0158:
            return CMPR(3, 0);

        case 0x0159:
            return CMPR(3, 1);

        case 0x015A:
            return CMPR(3, 2);

        case 0x015B:
            return CMPR(3, 3);

        case 0x015C:
            return CMPR(3, 4);

        case 0x015D:
            return CMPR(3, 5);

        case 0x015E:
            return CMPR(3, 6);

        case 0x015F:
            return CMPR(3, 7);

        case 0x0160:
            return CMPR(4, 0);

        case 0x0161:
            return CMPR(4, 1);

        case 0x0162:
            return CMPR(4, 2);

        case 0x0163:
            return CMPR(4, 3);

        case 0x0164:
            return CMPR(4, 4);

        case 0x0165:
            return CMPR(4, 5);

        case 0x0166:
            return CMPR(4, 6);

        case 0x0167:
            return CMPR(4, 7);

        case 0x0168:
            return CMPR(5, 0);

        case 0x0169:
            return CMPR(5, 1);

        case 0x016A:
            return CMPR(5, 2);

        case 0x016B:
            return CMPR(5, 3);

        case 0x016C:
            return CMPR(5, 4);

        case 0x016D:
            return CMPR(5, 5);

        case 0x016E:
            return CMPR(5, 6);

        case 0x016F:
            return CMPR(5, 7);

        case 0x0170:
            return CMPR(6, 0);

        case 0x0171:
            return CMPR(6, 1);

        case 0x0172:
            return CMPR(6, 2);

        case 0x0173:
            return CMPR(6, 3);

        case 0x0174:
            return CMPR(6, 4);

        case 0x0175:
            return CMPR(6, 5);

        case 0x0176:
            return CMPR(6, 6);

        case 0x0177:
            return CMPR(6, 7);

        case 0x0178:
            return CMPR(7, 0);

        case 0x0179:
            return CMPR(7, 1);

        case 0x017A:
            return CMPR(7, 2);

        case 0x017B:
            return CMPR(7, 3);

        case 0x017C:
            return CMPR(7, 4);

        case 0x017D:
            return CMPR(7, 5);

        case 0x017E:
            return CMPR(7, 6);

        case 0x017F:
            return CMPR(7, 7);

        case 0x0180:
            return ANDR(0, 0);

        case 0x0181:
            return ANDR(0, 1);

        case 0x0182:
            return ANDR(0, 2);

        case 0x0183:
            return ANDR(0, 3);

        case 0x0184:
            return ANDR(0, 4);

        case 0x0185:
            return ANDR(0, 5);

        case 0x0186:
            return ANDR(0, 6);

        case 0x0187:
            return ANDR(0, 7);

        case 0x0188:
            return ANDR(1, 0);

        case 0x0189:
            return ANDR(1, 1);

        case 0x018A:
            return ANDR(1, 2);

        case 0x018B:
            return ANDR(1, 3);

        case 0x018C:
            return ANDR(1, 4);

        case 0x018D:
            return ANDR(1, 5);

        case 0x018E:
            return ANDR(1, 6);

        case 0x018F:
            return ANDR(1, 7);

        case 0x0190:
            return ANDR(2, 0);

        case 0x0191:
            return ANDR(2, 1);

        case 0x0192:
            return ANDR(2, 2);

        case 0x0193:
            return ANDR(2, 3);

        case 0x0194:
            return ANDR(2, 4);

        case 0x0195:
            return ANDR(2, 5);

        case 0x0196:
            return ANDR(2, 6);

        case 0x0197:
            return ANDR(2, 7);

        case 0x0198:
            return ANDR(3, 0);

        case 0x0199:
            return ANDR(3, 1);

        case 0x019A:
            return ANDR(3, 2);

        case 0x019B:
            return ANDR(3, 3);

        case 0x019C:
            return ANDR(3, 4);

        case 0x019D:
            return ANDR(3, 5);

        case 0x019E:
            return ANDR(3, 6);

        case 0x019F:
            return ANDR(3, 7);

        case 0x01A0:
            return ANDR(4, 0);

        case 0x01A1:
            return ANDR(4, 1);

        case 0x01A2:
            return ANDR(4, 2);

        case 0x01A3:
            return ANDR(4, 3);

        case 0x01A4:
            return ANDR(4, 4);

        case 0x01A5:
            return ANDR(4, 5);

        case 0x01A6:
            return ANDR(4, 6);

        case 0x01A7:
            return ANDR(4, 7);

        case 0x01A8:
            return ANDR(5, 0);

        case 0x01A9:
            return ANDR(5, 1);

        case 0x01AA:
            return ANDR(5, 2);

        case 0x01AB:
            return ANDR(5, 3);

        case 0x01AC:
            return ANDR(5, 4);

        case 0x01AD:
            return ANDR(5, 5);

        case 0x01AE:
            return ANDR(5, 6);

        case 0x01AF:
            return ANDR(5, 7);

        case 0x01B0:
            return ANDR(6, 0);

        case 0x01B1:
            return ANDR(6, 1);

        case 0x01B2:
            return ANDR(6, 2);

        case 0x01B3:
            return ANDR(6, 3);

        case 0x01B4:
            return ANDR(6, 4);

        case 0x01B5:
            return ANDR(6, 5);

        case 0x01B6:
            return ANDR(6, 6);

        case 0x01B7:
            return ANDR(6, 7);

        case 0x01B8:
            return ANDR(7, 0);

        case 0x01B9:
            return ANDR(7, 1);

        case 0x01BA:
            return ANDR(7, 2);

        case 0x01BB:
            return ANDR(7, 3);

        case 0x01BC:
            return ANDR(7, 4);

        case 0x01BD:
            return ANDR(7, 5);

        case 0x01BE:
            return ANDR(7, 6);

        case 0x01BF:
            return ANDR(7, 7);

        case 0x01C0:
            return XORR(0, 0);

        case 0x01C1:
            return XORR(0, 1);

        case 0x01C2:
            return XORR(0, 2);

        case 0x01C3:
            return XORR(0, 3);

        case 0x01C4:
            return XORR(0, 4);

        case 0x01C5:
            return XORR(0, 5);

        case 0x01C6:
            return XORR(0, 6);

        case 0x01C7:
            return XORR(0, 7);

        case 0x01C8:
            return XORR(1, 0);

        case 0x01C9:
            return XORR(1, 1);

        case 0x01CA:
            return XORR(1, 2);

        case 0x01CB:
            return XORR(1, 3);

        case 0x01CC:
            return XORR(1, 4);

        case 0x01CD:
            return XORR(1, 5);

        case 0x01CE:
            return XORR(1, 6);

        case 0x01CF:
            return XORR(1, 7);

        case 0x01D0:
            return XORR(2, 0);

        case 0x01D1:
            return XORR(2, 1);

        case 0x01D2:
            return XORR(2, 2);

        case 0x01D3:
            return XORR(2, 3);

        case 0x01D4:
            return XORR(2, 4);

        case 0x01D5:
            return XORR(2, 5);

        case 0x01D6:
            return XORR(2, 6);

        case 0x01D7:
            return XORR(2, 7);

        case 0x01D8:
            return XORR(3, 0);

        case 0x01D9:
            return XORR(3, 1);

        case 0x01DA:
            return XORR(3, 2);

        case 0x01DB:
            return XORR(3, 3);

        case 0x01DC:
            return XORR(3, 4);

        case 0x01DD:
            return XORR(3, 5);

        case 0x01DE:
            return XORR(3, 6);

        case 0x01DF:
            return XORR(3, 7);

        case 0x01E0:
            return XORR(4, 0);

        case 0x01E1:
            return XORR(4, 1);

        case 0x01E2:
            return XORR(4, 2);

        case 0x01E3:
            return XORR(4, 3);

        case 0x01E4:
            return XORR(4, 4);

        case 0x01E5:
            return XORR(4, 5);

        case 0x01E6:
            return XORR(4, 6);

        case 0x01E7:
            return XORR(4, 7);

        case 0x01E8:
            return XORR(5, 0);

        case 0x01E9:
            return XORR(5, 1);

        case 0x01EA:
            return XORR(5, 2);

        case 0x01EB:
            return XORR(5, 3);

        case 0x01EC:
            return XORR(5, 4);

        case 0x01ED:
            return XORR(5, 5);

        case 0x01EE:
            return XORR(5, 6);

        case 0x01EF:
            return XORR(5, 7);

        case 0x01F0:
            return XORR(6, 0);

        case 0x01F1:
            return XORR(6, 1);

        case 0x01F2:
            return XORR(6, 2);

        case 0x01F3:
            return XORR(6, 3);

        case 0x01F4:
            return XORR(6, 4);

        case 0x01F5:
            return XORR(6, 5);

        case 0x01F6:
            return XORR(6, 6);

        case 0x01F7:
            return XORR(6, 7);

        case 0x01F8:
            return XORR(7, 0);

        case 0x01F9:
            return XORR(7, 1);

        case 0x01FA:
            return XORR(7, 2);

        case 0x01FB:
            return XORR(7, 3);

        case 0x01FC:
            return XORR(7, 4);

        case 0x01FD:
            return XORR(7, 5);

        case 0x01FE:
            return XORR(7, 6);

        case 0x01FF:
            return XORR(7, 7);

        case 0x0200:
            return B(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0201:
            return BC(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0202:
            return BOV(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0203:
            return BPL(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0204:
            return BEQ(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0205:
            return BLT(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0206:
            return BLE(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0207:
            return BUSC(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0208:
            return NOPP(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0209:
            return BNC(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x020A:
            return BNOV(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x020B:
            return BMI(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x020C:
            return BNEQ(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x020D:
            return BGE(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x020E:
            return BGT(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x020F:
            return BESC(memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0210:
            return BEXT(0, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0211:
            return BEXT(1, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0212:
            return BEXT(2, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0213:
            return BEXT(3, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0214:
            return BEXT(4, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0215:
            return BEXT(5, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0216:
            return BEXT(6, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0217:
            return BEXT(7, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0218:
            return BEXT(8, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0219:
            return BEXT(9, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x021A:
            return BEXT(10,
                memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x021B:
            return BEXT(11,
                memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x021C:
            return BEXT(12,
                memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x021D:
            return BEXT(13,
                memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x021E:
            return BEXT(14,
                memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x021F:
            return BEXT(15,
                memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0220:
            return B(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0221:
            return BC(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0222:
            return BOV(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0223:
            return BPL(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0224:
            return BEQ(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0225:
            return BLT(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0226:
            return BLE(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0227:
            return BUSC(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0228:
            return NOPP(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0229:
            return BNC(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x022A:
            return BNOV(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x022B:
            return BMI(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x022C:
            return BNEQ(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x022D:
            return BGE(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x022E:
            return BGT(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x022F:
            return BESC(-memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0230:
            return BEXT(0,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0231:
            return BEXT(1,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0232:
            return BEXT(2,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0233:
            return BEXT(3,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0234:
            return BEXT(4,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0235:
            return BEXT(5,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0236:
            return BEXT(6,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0237:
            return BEXT(7,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0238:
            return BEXT(8,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0239:
            return BEXT(9,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x023A:
            return BEXT(10,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x023B:
            return BEXT(11,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x023C:
            return BEXT(12,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x023D:
            return BEXT(13,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x023E:
            return BEXT(14,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x023F:
            return BEXT(15,
                -memoryBus->peek((UINT16)(r[7] + 1)) - 1);

        case 0x0240:
            return MVO(0, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0241:
            return MVO(1, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0242:
            return MVO(2, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0243:
            return MVO(3, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0244:
            return MVO(4, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0245:
            return MVO(5, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0246:
            return MVO(6, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0247:
            return MVO(7, memoryBus->peek((UINT16)(r[7] + 1)));

        case 0x0248:
            return MVO_ind(1, 0);

        case 0x0249:
            return MVO_ind(1, 1);

        case 0x024A:
            return MVO_ind(1, 2);

        case 0x024B:
            return MVO_ind(1, 3);

        case 0x024C:
            return MVO_ind(1, 4);

        case 0x024D:
            return MVO_ind(1, 5);

        case 0x024E:
            return MVO_ind(1, 6);

        case 0x024F:
            return MVO_ind(1, 7);

        case 0x0250:
            return MVO_ind(2, 0);

        case 0x0251:
            return MVO_ind(2, 1);

        case 0x0252:
            return MVO_ind(2, 2);

        case 0x0253:
            return MVO_ind(2, 3);

        case 0x0254:
            return MVO_ind(2, 4);

        case 0x0255:
            return MVO_ind(2, 5);

        case 0x0256:
            return MVO_ind(2, 6);

        case 0x0257:
            return MVO_ind(2, 7);

        case 0x0258:
            return MVO_ind(3, 0);

        case 0x0259:
            return MVO_ind(3, 1);

        case 0x025A:
            return MVO_ind(3, 2);

        case 0x025B:
            return MVO_ind(3, 3);

        case 0x025C:
            return MVO_ind(3, 4);

        case 0x025D:
            return MVO_ind(3, 5);

        case 0x025E:
            return MVO_ind(3, 6);

        case 0x025F:
            return MVO_ind(3, 7);

        case 0x0260:
            return MVO_ind(4, 0);

        case 0x0261:
            return MVO_ind(4, 1);

        case 0x0262:
            return MVO_ind(4, 2);

        case 0x0263:
            return MVO_ind(4, 3);

        case 0x0264:
            return MVO_ind(4, 4);

        case 0x0265:
            return MVO_ind(4, 5);

        case 0x0266:
            return MVO_ind(4, 6);

        case 0x0267:
            return MVO_ind(4, 7);

        case 0x0268:
            return MVO_ind(5, 0);

        case 0x0269:
            return MVO_ind(5, 1);

        case 0x026A:
            return MVO_ind(5, 2);

        case 0x026B:
            return MVO_ind(5, 3);

        case 0x026C:
            return MVO_ind(5, 4);

        case 0x026D:
            return MVO_ind(5, 5);

        case 0x026E:
            return MVO_ind(5, 6);

        case 0x026F:
            return MVO_ind(5, 7);

        case 0x0270:
            return MVO_ind(6, 0);

        case 0x0271:
            return MVO_ind(6, 1);

        case 0x0272:
            return MVO_ind(6, 2);

        case 0x0273:
            return MVO_ind(6, 3);

        case 0x0274:
            return MVO_ind(6, 4);

        case 0x0275:
            return MVO_ind(6, 5);

        case 0x0276:
            return MVO_ind(6, 6);

        case 0x0277:
            return MVO_ind(6, 7);

        case 0x0278:
            return MVO_ind(7, 0);

        case 0x0279:
            return MVO_ind(7, 1);

        case 0x027A:
            return MVO_ind(7, 2);

        case 0x027B:
            return MVO_ind(7, 3);

        case 0x027C:
            return MVO_ind(7, 4);

        case 0x027D:
            return MVO_ind(7, 5);

        case 0x027E:
            return MVO_ind(7, 6);

        case 0x027F:
            return MVO_ind(7, 7);

        case 0x0280:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 0);

        case 0x0281:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 1);

        case 0x0282:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 2);

        case 0x0283:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 3);

        case 0x0284:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 4);

        case 0x0285:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 5);

        case 0x0286:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 6);

        case 0x0287:
            return MVI(memoryBus->peek((UINT16)(r[7] + 1)), 7);

        case 0x0288:
            return MVI_ind(1, 0);

        case 0x0289:
            return MVI_ind(1, 1);

        case 0x028A:
            return MVI_ind(1, 2);

        case 0x028B:
            return MVI_ind(1, 3);

        case 0x028C:
            return MVI_ind(1, 4);

        case 0x028D:
            return MVI_ind(1, 5);

        case 0x028E:
            return MVI_ind(1, 6);

        case 0x028F:
            return MVI_ind(1, 7);

        case 0x0290:
            return MVI_ind(2, 0);

        case 0x0291:
            return MVI_ind(2, 1);

        case 0x0292:
            return MVI_ind(2, 2);

        case 0x0293:
            return MVI_ind(2, 3);

        case 0x0294:
            return MVI_ind(2, 4);

        case 0x0295:
            return MVI_ind(2, 5);

        case 0x0296:
            return MVI_ind(2, 6);

        case 0x0297:
            return MVI_ind(2, 7);

        case 0x0298:
            return MVI_ind(3, 0);

        case 0x0299:
            return MVI_ind(3, 1);

        case 0x029A:
            return MVI_ind(3, 2);

        case 0x029B:
            return MVI_ind(3, 3);

        case 0x029C:
            return MVI_ind(3, 4);

        case 0x029D:
            return MVI_ind(3, 5);

        case 0x029E:
            return MVI_ind(3, 6);

        case 0x029F:
            return MVI_ind(3, 7);

        case 0x02A0:
            return MVI_ind(4, 0);

        case 0x02A1:
            return MVI_ind(4, 1);

        case 0x02A2:
            return MVI_ind(4, 2);

        case 0x02A3:
            return MVI_ind(4, 3);

        case 0x02A4:
            return MVI_ind(4, 4);

        case 0x02A5:
            return MVI_ind(4, 5);

        case 0x02A6:
            return MVI_ind(4, 6);

        case 0x02A7:
            return MVI_ind(4, 7);

        case 0x02A8:
            return MVI_ind(5, 0);

        case 0x02A9:
            return MVI_ind(5, 1);

        case 0x02AA:
            return MVI_ind(5, 2);

        case 0x02AB:
            return MVI_ind(5, 3);

        case 0x02AC:
            return MVI_ind(5, 4);

        case 0x02AD:
            return MVI_ind(5, 5);

        case 0x02AE:
            return MVI_ind(5, 6);

        case 0x02AF:
            return MVI_ind(5, 7);

        case 0x02B0:
            return MVI_ind(6, 0);

        case 0x02B1:
            return MVI_ind(6, 1);

        case 0x02B2:
            return MVI_ind(6, 2);

        case 0x02B3:
            return MVI_ind(6, 3);

        case 0x02B4:
            return MVI_ind(6, 4);

        case 0x02B5:
            return MVI_ind(6, 5);

        case 0x02B6:
            return MVI_ind(6, 6);

        case 0x02B7:
            return MVI_ind(6, 7);

        case 0x02B8:
            return MVI_ind(7, 0);

        case 0x02B9:
            return MVI_ind(7, 1);

        case 0x02BA:
            return MVI_ind(7, 2);

        case 0x02BB:
            return MVI_ind(7, 3);

        case 0x02BC:
            return MVI_ind(7, 4);

        case 0x02BD:
            return MVI_ind(7, 5);

        case 0x02BE:
            return MVI_ind(7, 6);

        case 0x02BF:
            return MVI_ind(7, 7);

        case 0x02C0:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 0);

        case 0x02C1:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 1);

        case 0x02C2:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 2);

        case 0x02C3:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 3);

        case 0x02C4:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 4);

        case 0x02C5:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 5);

        case 0x02C6:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 6);

        case 0x02C7:
            return ADD(memoryBus->peek((UINT16)(r[7] + 1)), 7);

        case 0x02C8:
            return ADD_ind(1, 0);

        case 0x02C9:
            return ADD_ind(1, 1);

        case 0x02CA:
            return ADD_ind(1, 2);

        case 0x02CB:
            return ADD_ind(1, 3);

        case 0x02CC:
            return ADD_ind(1, 4);

        case 0x02CD:
            return ADD_ind(1, 5);

        case 0x02CE:
            return ADD_ind(1, 6);

        case 0x02CF:
            return ADD_ind(1, 7);

        case 0x02D0:
            return ADD_ind(2, 0);

        case 0x02D1:
            return ADD_ind(2, 1);

        case 0x02D2:
            return ADD_ind(2, 2);

        case 0x02D3:
            return ADD_ind(2, 3);

        case 0x02D4:
            return ADD_ind(2, 4);

        case 0x02D5:
            return ADD_ind(2, 5);

        case 0x02D6:
            return ADD_ind(2, 6);

        case 0x02D7:
            return ADD_ind(2, 7);

        case 0x02D8:
            return ADD_ind(3, 0);

        case 0x02D9:
            return ADD_ind(3, 1);

        case 0x02DA:
            return ADD_ind(3, 2);

        case 0x02DB:
            return ADD_ind(3, 3);

        case 0x02DC:
            return ADD_ind(3, 4);

        case 0x02DD:
            return ADD_ind(3, 5);

        case 0x02DE:
            return ADD_ind(3, 6);

        case 0x02DF:
            return ADD_ind(3, 7);

        case 0x02E0:
            return ADD_ind(4, 0);

        case 0x02E1:
            return ADD_ind(4, 1);

        case 0x02E2:
            return ADD_ind(4, 2);

        case 0x02E3:
            return ADD_ind(4, 3);

        case 0x02E4:
            return ADD_ind(4, 4);

        case 0x02E5:
            return ADD_ind(4, 5);

        case 0x02E6:
            return ADD_ind(4, 6);

        case 0x02E7:
            return ADD_ind(4, 7);

        case 0x02E8:
            return ADD_ind(5, 0);

        case 0x02E9:
            return ADD_ind(5, 1);

        case 0x02EA:
            return ADD_ind(5, 2);

        case 0x02EB:
            return ADD_ind(5, 3);

        case 0x02EC:
            return ADD_ind(5, 4);

        case 0x02ED:
            return ADD_ind(5, 5);

        case 0x02EE:
            return ADD_ind(5, 6);

        case 0x02EF:
            return ADD_ind(5, 7);

        case 0x02F0:
            return ADD_ind(6, 0);

        case 0x02F1:
            return ADD_ind(6, 1);

        case 0x02F2:
            return ADD_ind(6, 2);

        case 0x02F3:
            return ADD_ind(6, 3);

        case 0x02F4:
            return ADD_ind(6, 4);

        case 0x02F5:
            return ADD_ind(6, 5);

        case 0x02F6:
            return ADD_ind(6, 6);

        case 0x02F7:
            return ADD_ind(6, 7);

        case 0x02F8:
            return ADD_ind(7, 0);

        case 0x02F9:
            return ADD_ind(7, 1);

        case 0x02FA:
            return ADD_ind(7, 2);

        case 0x02FB:
            return ADD_ind(7, 3);

        case 0x02FC:
            return ADD_ind(7, 4);

        case 0x02FD:
            return ADD_ind(7, 5);

        case 0x02FE:
            return ADD_ind(7, 6);

        case 0x02FF:
            return ADD_ind(7, 7);

        case 0x0300:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 0);

        case 0x0301:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 1);

        case 0x0302:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 2);

        case 0x0303:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 3);

        case 0x0304:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 4);

        case 0x0305:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 5);

        case 0x0306:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 6);

        case 0x0307:
            return SUB(memoryBus->peek((UINT16)(r[7] + 1)), 7);

        case 0x0308:
            return SUB_ind(1, 0);

        case 0x0309:
            return SUB_ind(1, 1);

        case 0x030A:
            return SUB_ind(1, 2);

        case 0x030B:
            return SUB_ind(1, 3);

        case 0x030C:
            return SUB_ind(1, 4);

        case 0x030D:
            return SUB_ind(1, 5);

        case 0x030E:
            return SUB_ind(1, 6);

        case 0x030F:
            return SUB_ind(1, 7);

        case 0x0310:
            return SUB_ind(2, 0);

        case 0x0311:
            return SUB_ind(2, 1);

        case 0x0312:
            return SUB_ind(2, 2);

        case 0x0313:
            return SUB_ind(2, 3);

        case 0x0314:
            return SUB_ind(2, 4);

        case 0x0315:
            return SUB_ind(2, 5);

        case 0x0316:
            return SUB_ind(2, 6);

        case 0x0317:
            return SUB_ind(2, 7);

        case 0x0318:
            return SUB_ind(3, 0);

        case 0x0319:
            return SUB_ind(3, 1);

        case 0x031A:
            return SUB_ind(3, 2);

        case 0x031B:
            return SUB_ind(3, 3);

        case 0x031C:
            return SUB_ind(3, 4);

        case 0x031D:
            return SUB_ind(3, 5);

        case 0x031E:
            return SUB_ind(3, 6);

        case 0x031F:
            return SUB_ind(3, 7);

        case 0x0320:
            return SUB_ind(4, 0);

        case 0x0321:
            return SUB_ind(4, 1);

        case 0x0322:
            return SUB_ind(4, 2);

        case 0x0323:
            return SUB_ind(4, 3);

        case 0x0324:
            return SUB_ind(4, 4);

        case 0x0325:
            return SUB_ind(4, 5);

        case 0x0326:
            return SUB_ind(4, 6);

        case 0x0327:
            return SUB_ind(4, 7);

        case 0x0328:
            return SUB_ind(5, 0);

        case 0x0329:
            return SUB_ind(5, 1);

        case 0x032A:
            return SUB_ind(5, 2);

        case 0x032B:
            return SUB_ind(5, 3);

        case 0x032C:
            return SUB_ind(5, 4);

        case 0x032D:
            return SUB_ind(5, 5);

        case 0x032E:
            return SUB_ind(5, 6);

        case 0x032F:
            return SUB_ind(5, 7);

        case 0x0330:
            return SUB_ind(6, 0);

        case 0x0331:
            return SUB_ind(6, 1);

        case 0x0332:
            return SUB_ind(6, 2);

        case 0x0333:
            return SUB_ind(6, 3);

        case 0x0334:
            return SUB_ind(6, 4);

        case 0x0335:
            return SUB_ind(6, 5);

        case 0x0336:
            return SUB_ind(6, 6);

        case 0x0337:
            return SUB_ind(6, 7);

        case 0x0338:
            return SUB_ind(7, 0);

        case 0x0339:
            return SUB_ind(7, 1);

        case 0x033A:
            return SUB_ind(7, 2);

        case 0x033B:
            return SUB_ind(7, 3);

        case 0x033C:
            return SUB_ind(7, 4);

        case 0x033D:
            return SUB_ind(7, 5);

        case 0x033E:
            return SUB_ind(7, 6);

        case 0x033F:
            return SUB_ind(7, 7);

        case 0x0340:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 0);

        case 0x0341:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 1);

        case 0x0342:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 2);

        case 0x0343:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 3);

        case 0x0344:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 4);

        case 0x0345:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 5);

        case 0x0346:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 6);

        case 0x0347:
            return CMP(memoryBus->peek((UINT16)(r[7] + 1)), 7);

        case 0x0348:
            return CMP_ind(1, 0);

        case 0x0349:
            return CMP_ind(1, 1);

        case 0x034A:
            return CMP_ind(1, 2);

        case 0x034B:
            return CMP_ind(1, 3);

        case 0x034C:
            return CMP_ind(1, 4);

        case 0x034D:
            return CMP_ind(1, 5);

        case 0x034E:
            return CMP_ind(1, 6);

        case 0x034F:
            return CMP_ind(1, 7);

        case 0x0350:
            return CMP_ind(2, 0);

        case 0x0351:
            return CMP_ind(2, 1);

        case 0x0352:
            return CMP_ind(2, 2);

        case 0x0353:
            return CMP_ind(2, 3);

        case 0x0354:
            return CMP_ind(2, 4);

        case 0x0355:
            return CMP_ind(2, 5);

        case 0x0356:
            return CMP_ind(2, 6);

        case 0x0357:
            return CMP_ind(2, 7);

        case 0x0358:
            return CMP_ind(3, 0);

        case 0x0359:
            return CMP_ind(3, 1);

        case 0x035A:
            return CMP_ind(3, 2);

        case 0x035B:
            return CMP_ind(3, 3);

        case 0x035C:
            return CMP_ind(3, 4);

        case 0x035D:
            return CMP_ind(3, 5);

        case 0x035E:
            return CMP_ind(3, 6);

        case 0x035F:
            return CMP_ind(3, 7);

        case 0x0360:
            return CMP_ind(4, 0);

        case 0x0361:
            return CMP_ind(4, 1);

        case 0x0362:
            return CMP_ind(4, 2);

        case 0x0363:
            return CMP_ind(4, 3);

        case 0x0364:
            return CMP_ind(4, 4);

        case 0x0365:
            return CMP_ind(4, 5);

        case 0x0366:
            return CMP_ind(4, 6);

        case 0x0367:
            return CMP_ind(4, 7);

        case 0x0368:
            return CMP_ind(5, 0);

        case 0x0369:
            return CMP_ind(5, 1);

        case 0x036A:
            return CMP_ind(5, 2);

        case 0x036B:
            return CMP_ind(5, 3);

        case 0x036C:
            return CMP_ind(5, 4);

        case 0x036D:
            return CMP_ind(5, 5);

        case 0x036E:
            return CMP_ind(5, 6);

        case 0x036F:
            return CMP_ind(5, 7);

        case 0x0370:
            return CMP_ind(6, 0);

        case 0x0371:
            return CMP_ind(6, 1);

        case 0x0372:
            return CMP_ind(6, 2);

        case 0x0373:
            return CMP_ind(6, 3);

        case 0x0374:
            return CMP_ind(6, 4);

        case 0x0375:
            return CMP_ind(6, 5);

        case 0x0376:
            return CMP_ind(6, 6);

        case 0x0377:
            return CMP_ind(6, 7);

        case 0x0378:
            return CMP_ind(7, 0);

        case 0x0379:
            return CMP_ind(7, 1);

        case 0x037A:
            return CMP_ind(7, 2);

        case 0x037B:
            return CMP_ind(7, 3);

        case 0x037C:
            return CMP_ind(7, 4);

        case 0x037D:
            return CMP_ind(7, 5);

        case 0x037E:
            return CMP_ind(7, 6);

        case 0x037F:
            return CMP_ind(7, 7);

        case 0x0380:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 0);

        case 0x0381:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 1);

        case 0x0382:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 2);

        case 0x0383:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 3);

        case 0x0384:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 4);

        case 0x0385:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 5);

        case 0x0386:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 6);

        case 0x0387:
            return AND(memoryBus->peek((UINT16)(r[7] + 1)), 7);

        case 0x0388:
            return AND_ind(1, 0);

        case 0x0389:
            return AND_ind(1, 1);

        case 0x038A:
            return AND_ind(1, 2);

        case 0x038B:
            return AND_ind(1, 3);

        case 0x038C:
            return AND_ind(1, 4);

        case 0x038D:
            return AND_ind(1, 5);

        case 0x038E:
            return AND_ind(1, 6);

        case 0x038F:
            return AND_ind(1, 7);

        case 0x0390:
            return AND_ind(2, 0);

        case 0x0391:
            return AND_ind(2, 1);

        case 0x0392:
            return AND_ind(2, 2);

        case 0x0393:
            return AND_ind(2, 3);

        case 0x0394:
            return AND_ind(2, 4);

        case 0x0395:
            return AND_ind(2, 5);

        case 0x0396:
            return AND_ind(2, 6);

        case 0x0397:
            return AND_ind(2, 7);

        case 0x0398:
            return AND_ind(3, 0);

        case 0x0399:
            return AND_ind(3, 1);

        case 0x039A:
            return AND_ind(3, 2);

        case 0x039B:
            return AND_ind(3, 3);

        case 0x039C:
            return AND_ind(3, 4);

        case 0x039D:
            return AND_ind(3, 5);

        case 0x039E:
            return AND_ind(3, 6);

        case 0x039F:
            return AND_ind(3, 7);

        case 0x03A0:
            return AND_ind(4, 0);

        case 0x03A1:
            return AND_ind(4, 1);

        case 0x03A2:
            return AND_ind(4, 2);

        case 0x03A3:
            return AND_ind(4, 3);

        case 0x03A4:
            return AND_ind(4, 4);

        case 0x03A5:
            return AND_ind(4, 5);

        case 0x03A6:
            return AND_ind(4, 6);

        case 0x03A7:
            return AND_ind(4, 7);

        case 0x03A8:
            return AND_ind(5, 0);

        case 0x03A9:
            return AND_ind(5, 1);

        case 0x03AA:
            return AND_ind(5, 2);

        case 0x03AB:
            return AND_ind(5, 3);

        case 0x03AC:
            return AND_ind(5, 4);

        case 0x03AD:
            return AND_ind(5, 5);

        case 0x03AE:
            return AND_ind(5, 6);

        case 0x03AF:
            return AND_ind(5, 7);

        case 0x03B0:
            return AND_ind(6, 0);

        case 0x03B1:
            return AND_ind(6, 1);

        case 0x03B2:
            return AND_ind(6, 2);

        case 0x03B3:
            return AND_ind(6, 3);

        case 0x03B4:
            return AND_ind(6, 4);

        case 0x03B5:
            return AND_ind(6, 5);

        case 0x03B6:
            return AND_ind(6, 6);

        case 0x03B7:
            return AND_ind(6, 7);

        case 0x03B8:
            return AND_ind(7, 0);

        case 0x03B9:
            return AND_ind(7, 1);

        case 0x03BA:
            return AND_ind(7, 2);

        case 0x03BB:
            return AND_ind(7, 3);

        case 0x03BC:
            return AND_ind(7, 4);

        case 0x03BD:
            return AND_ind(7, 5);

        case 0x03BE:
            return AND_ind(7, 6);

        case 0x03BF:
            return AND_ind(7, 7);

        case 0x03C0:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 0);

        case 0x03C1:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 1);

        case 0x03C2:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 2);

        case 0x03C3:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 3);

        case 0x03C4:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 4);

        case 0x03C5:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 5);

        case 0x03C6:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 6);

        case 0x03C7:
            return XOR(memoryBus->peek((UINT16)(r[7] + 1)), 7);

        case 0x03C8:
            return XOR_ind(1, 0);

        case 0x03C9:
            return XOR_ind(1, 1);

        case 0x03CA:
            return XOR_ind(1, 2);

        case 0x03CB:
            return XOR_ind(1, 3);

        case 0x03CC:
            return XOR_ind(1, 4);

        case 0x03CD:
            return XOR_ind(1, 5);

        case 0x03CE:
            return XOR_ind(1, 6);

        case 0x03CF:
            return XOR_ind(1, 7);

        case 0x03D0:
            return XOR_ind(2, 0);

        case 0x03D1:
            return XOR_ind(2, 1);

        case 0x03D2:
            return XOR_ind(2, 2);

        case 0x03D3:
            return XOR_ind(2, 3);

        case 0x03D4:
            return XOR_ind(2, 4);

        case 0x03D5:
            return XOR_ind(2, 5);

        case 0x03D6:
            return XOR_ind(2, 6);

        case 0x03D7:
            return XOR_ind(2, 7);

        case 0x03D8:
            return XOR_ind(3, 0);

        case 0x03D9:
            return XOR_ind(3, 1);

        case 0x03DA:
            return XOR_ind(3, 2);

        case 0x03DB:
            return XOR_ind(3, 3);

        case 0x03DC:
            return XOR_ind(3, 4);

        case 0x03DD:
            return XOR_ind(3, 5);

        case 0x03DE:
            return XOR_ind(3, 6);

        case 0x03DF:
            return XOR_ind(3, 7);

        case 0x03E0:
            return XOR_ind(4, 0);

        case 0x03E1:
            return XOR_ind(4, 1);

        case 0x03E2:
            return XOR_ind(4, 2);

        case 0x03E3:
            return XOR_ind(4, 3);

        case 0x03E4:
            return XOR_ind(4, 4);

        case 0x03E5:
            return XOR_ind(4, 5);

        case 0x03E6:
            return XOR_ind(4, 6);

        case 0x03E7:
            return XOR_ind(4, 7);

        case 0x03E8:
            return XOR_ind(5, 0);

        case 0x03E9:
            return XOR_ind(5, 1);

        case 0x03EA:
            return XOR_ind(5, 2);

        case 0x03EB:
            return XOR_ind(5, 3);

        case 0x03EC:
            return XOR_ind(5, 4);

        case 0x03ED:
            return XOR_ind(5, 5);

        case 0x03EE:
            return XOR_ind(5, 6);

        case 0x03EF:
            return XOR_ind(5, 7);

        case 0x03F0:
            return XOR_ind(6, 0);

        case 0x03F1:
            return XOR_ind(6, 1);

        case 0x03F2:
            return XOR_ind(6, 2);

        case 0x03F3:
            return XOR_ind(6, 3);

        case 0x03F4:
            return XOR_ind(6, 4);

        case 0x03F5:
            return XOR_ind(6, 5);

        case 0x03F6:
            return XOR_ind(6, 6);

        case 0x03F7:
            return XOR_ind(6, 7);

        case 0x03F8:
            return XOR_ind(7, 0);

        case 0x03F9:
            return XOR_ind(7, 1);

        case 0x03FA:
            return XOR_ind(7, 2);

        case 0x03FB:
            return XOR_ind(7, 3);

        case 0x03FC:
            return XOR_ind(7, 4);

        case 0x03FD:
            return XOR_ind(7, 5);

        case 0x03FE:
            return XOR_ind(7, 6);

        case 0x03FF:
        default :
            return XOR_ind(7, 7);

    }
}

CP1610State CP1610::getState()
{
	CP1610State state = {0};

	state.interruptAddress = this->interruptAddress;
	state.resetAddress = this->resetAddress;
	memcpy(state.r, this->r, sizeof(this->r));
	state.S = this->S;
	state.Z = this->Z;
	state.O = this->O;
	state.C = this->C;
	state.I = this->I;
	state.D = this->D;
	state.interruptible = this->interruptible;
	state.ext = this->ext;

	return state;
}

void CP1610::setState(CP1610State state)
{
	this->interruptAddress = state.interruptAddress;
	this->resetAddress = state.resetAddress;
	memcpy(this->r, state.r, sizeof(this->r));
	this->S = state.S;
	this->Z = state.Z;
	this->O = state.O;
	this->C = state.C;
	this->I = state.I;
	this->D = state.D;
	this->interruptible = state.interruptible;
	this->ext = state.ext;
}
