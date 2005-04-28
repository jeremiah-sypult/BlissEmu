
#include <iostream>
#include <stdlib.h>
#include "Antic.h"

using namespace std;

#define TICK_LENGTH_VISIBLE_SCANLINE 192
#define TICK_LENGTH_HBLANK            36
#define ANBK  0x0
#define ANPF0 0x4
#define ANPF1 0x5
#define ANPF2 0x6
#define ANPF3 0x7

const UINT8 Antic::BLOCK_LENGTHS[14] = {
    8, 10, 8, 8, 8, 16, 8, 4, 4, 2, 1, 2, 1, 1
};

const UINT8 Antic::BYTE_WIDTHS[14][3] = {
    //text modes
    { 32, 40, 48 },
    { 32, 40, 48 },
    { 32, 40, 48 },
    { 32, 40, 48 },
    { 16, 20, 24 },
    { 16, 20, 24 },
    //graphics modes
    {  8, 10, 12 },
    {  8, 10, 12 },
    { 16, 20, 24 },
    { 16, 20, 24 },
    { 16, 20, 24 },
    { 32, 40, 48 },
    { 32, 40, 48 },
    { 32, 40, 48 },
};

const UINT32 Antic::palette[256] = {
    0x323132, 0x3f3e3f, 0x4d4c4d, 0x5b5b5b,
    0x6a696a, 0x797879, 0x888788, 0x979797,
    0xa1a0a1, 0xafafaf, 0xbebebe, 0xcecdce,
    0xdbdbdb, 0xebeaeb, 0xfafafa, 0xffffff,
    0x612e00, 0x6c3b00, 0x7a4a00, 0x885800,
    0x94670c, 0xa5761b, 0xb2842a, 0xc1943a,
    0xca9d43, 0xdaad53, 0xe8bb62, 0xf8cb72,
    0xffd87f, 0xffe88f, 0xfff79f, 0xffffae,
    0x6c2400, 0x773000, 0x844003, 0x924e11,
    0x9e5d22, 0xaf6c31, 0xbc7b41, 0xcc8a50,
    0xd5935b, 0xe4a369, 0xf2b179, 0xffc289,
    0xffcf97, 0xffdfa6, 0xffedb5, 0xfffdc4,
    0x751618, 0x812324, 0x8f3134, 0x9d4043,
    0xaa4e50, 0xb85e60, 0xc66d6f, 0xd57d7f,
    0xde8787, 0xed9596, 0xfca4a5, 0xffb4b5,
    0xffc2c4, 0xffd1d3, 0xffe0e1, 0xffeff0,
    0x620e71, 0x6e1b7c, 0x7b2a8a, 0x8a3998,
    0x9647a5, 0xa557b5, 0xb365c3, 0xc375d1,
    0xcd7eda, 0xdc8de9, 0xea97f7, 0xf9acff,
    0xffbaff, 0xffc9ff, 0xffd9ff, 0xffe8ff,
    0x560f87, 0x611d90, 0x712c9e, 0x7f3aac,
    0x8d48ba, 0x9b58c7, 0xa967d5, 0xb877e5,
    0xc280ed, 0xd090fc, 0xdf9fff, 0xeeafff,
    0xfcbdff, 0xffccff, 0xffdbff, 0xffeaff,
    0x461695, 0x5122a0, 0x6032ac, 0x6e41bb,
    0x7c4fc8, 0x8a5ed6, 0x996de3, 0xa87cf2,
    0xb185fb, 0xc095ff, 0xcfa3ff, 0xdfb3ff,
    0xeec1ff, 0xfcd0ff, 0xffdfff, 0xffefff,
    0x212994, 0x2d359f, 0x3d44ad, 0x4b53ba,
    0x5961c7, 0x686fd5, 0x777ee2, 0x878ef2,
    0x9097fa, 0x96a6ff, 0xaeb5ff, 0xbfc4ff,
    0xcdd2ff, 0xdae3ff, 0xeaf1ff, 0xfafeff,
    0x0f3584, 0x1c418d, 0x2c509b, 0x3a5eaa,
    0x486cb7, 0x587bc5, 0x678ad2, 0x7699e2,
    0x80a2eb, 0x8fb2f9, 0x9ec0ff, 0xadd0ff,
    0xbdddff, 0xcbecff, 0xdbfcff, 0xeaffff,
    0x043f70, 0x114b79, 0x215988, 0x2f6896,
    0x3e75a4, 0x4d83b2, 0x5c92c1, 0x6ca1d2,
    0x74abd9, 0x83bae7, 0x93c9f6, 0xa2d8ff,
    0xb1e6ff, 0xc0f5ff, 0xd0ffff, 0xdeffff,
    0x005918, 0x006526, 0x0f7235, 0x1d8144,
    0x2c8e50, 0x3b9d60, 0x4aac6f, 0x59bb7e,
    0x63c487, 0x72d396, 0x82e2a5, 0x92f1b5,
    0x9ffec3, 0xaeffd2, 0xbeffe2, 0xcefff1,
    0x075c00, 0x146800, 0x227500, 0x328300,
    0x3f910b, 0x4fa01b, 0x5eae2a, 0x6ebd3b,
    0x77c644, 0x87d553, 0x96e363, 0xa7f373,
    0xb3fe80, 0xc3ff8f, 0xd3ffa0, 0xe3ffb0,
    0x1a5600, 0x286200, 0x367000, 0x457e00,
    0x538c00, 0x629b07, 0x70a916, 0x80b926,
    0x89c22f, 0x99d13e, 0xa8df4d, 0xb7ef5c,
    0xc5fc6b, 0xd5ff7b, 0xe3ff8b, 0xf3ff99,
    0x334b00, 0x405700, 0x4d6500, 0x5d7300,
    0x6a8200, 0x7a9100, 0x889e0f, 0x98ae1f,
    0xa1b728, 0xbac638, 0xbfd548, 0xcee458,
    0xdcf266, 0xebff75, 0xfaff85, 0xffff95,
    0x4b3c00, 0x584900, 0x655700, 0x746500,
    0x817400, 0x908307, 0x9f9116, 0xaea126,
    0xb7aa2e, 0xc7ba3e, 0xd5c74d, 0xe5d75d,
    0xf2e56b, 0xfef47a, 0xffff8b, 0xffff9a,
    0x602e00, 0x6d3a00, 0x7a4900, 0x895800,
    0x95670a, 0xa4761b, 0xb2832a, 0xc2943a,
    0xcb9d44, 0xdaac53, 0xe8ba62, 0xf8cb73,
    0xffd77f, 0xffe791, 0xfff69f, 0xffffaf,
};

Antic::Antic()
    : Processor("ANTIC")
{
    inVBlank = FALSE;
    memoryBus = NULL;
    gtia = NULL;
    imageBank = NULL;

    INST = 0;
    LCOUNT = 0;
    HCOUNT = 0;
    MEMSCAN = 0;
    MODE = 0;
    BYTEWIDTH = 0;
    BLOCKLENGTH = 0;

    DMACTL = 0;
    CHACTL = 0;
    DLIST = 0;
    HSCROL = 0;
    VSCROL = 0;
    PMBASE = 0;
    CHBASE = 0;
    VCOUNT = 0;
    NMIEN = 0;
    NMIST = 0;
}

Antic::Antic(MemoryBus* mb, GTIA* gt)
    : Processor("ANTIC"),
      memoryBus(mb),
      gtia(gt)
{
    this->imageBank = gtia->imageBank;
    registers.init(this);
}

void Antic::getOutputImageSize(UINT16* width, UINT16* height)
{
    (*width) = 320;
    (*height) = 192;
}

void Antic::getPalette(const UINT32** palette, UINT16* numEntries)
{
    (*palette) = this->palette;
    (*numEntries) = 256;
}

void Antic::render()
{
    //videoOutputDevice->renderPalettedImage(gtia->imageBank);
}

Antic_Registers* Antic::getRegisters()
{
    return &registers;
}

void Antic::resetProcessor()
{
    pinOut[ANTIC_PIN_OUT_NMI]->isHigh = FALSE;
    pinOut[ANTIC_PIN_OUT_HALT]->isHigh = FALSE;
    pinOut[ANTIC_PIN_OUT_READY]->isHigh = FALSE;
    DMACTL = 0;
    PMBASE = 0;
    VCOUNT = 0;
}

void Antic::setVideoOutputDevice(IDirect3DDevice9*)
{
}

INT32 Antic::getClockSpeed()
{
    return 3584160;  //NTSC clock rate
}

INT32 Antic::tick(INT32)
{
#ifdef _TRACE_TICKS
    fprintf(stderr, "Antic::tick()\n");
#endif

//cout << "ticking antic line #" << (int)VCOUNT << "\n";
    //two modes are modeled here, display area and VBLANK
    if (VCOUNT < 240) {
        //if we're at the start of a new block, read and execute the next
        //instruction in the display list
        if (LCOUNT == 0 && HCOUNT == 0) {
            //if instruction DMA is disabled, just blank this line
            if (!(DMACTL & 0x20)) {
//cout << "1a\n";
                pinOut[ANTIC_PIN_OUT_HALT]->isHigh = FALSE;
                if (VCOUNT > 23 && VCOUNT < 216)
                    render_blank();
                HCOUNT = TICK_LENGTH_VISIBLE_SCANLINE;
                return TICK_LENGTH_VISIBLE_SCANLINE;
            }

            UINT8 cyclesToSteal = 0;
            //fetch the next instruction
            INST = (UINT8)memoryBus->peek(DLIST);
            DLIST = (DLIST & 0xFC00) | ((DLIST+1) & 0x03FF);
            //steal a cpu cycle
            cyclesToSteal += 2;

            //execute the instruction
            if ((INST & 0x0F) == 0x00) {
//cout << "executing a display list blank instruction\n";
                MODE = 0;
                BYTEWIDTH = 0;
                BLOCKLENGTH = ((INST & 0x70) >> 4)+1;
            }
            else if ((INST & 0x0F) == 0x01) {
//cout << "executing a display list jump instruction\n";
                UINT8 lo = (UINT8)memoryBus->peek(DLIST);
                DLIST = (DLIST & 0xFC00) | ((DLIST+1) & 0x03FF);
                UINT8 hi = (UINT8)memoryBus->peek(DLIST);
                DLIST = (hi << 8) | lo;
                //steal two cpu cycles
                cyclesToSteal += 4;
                
                MODE = 0;
                BYTEWIDTH = 0;
                if (INST & 0x40)
                    BLOCKLENGTH = (UINT8)(240-VCOUNT);
                else
                    BLOCKLENGTH = 1;
            }
            else {
//cout << "executing a display list instruction\n";
                MODE = (INST & 0x0F);
                //reload memscan if necessary
                if (INST & 0x40) {
                    UINT8 lo = (UINT8)memoryBus->peek(DLIST);
                    DLIST = (DLIST & 0xFC00) | ((DLIST+1) & 0x03FF);
                    UINT8 hi = (UINT8)memoryBus->peek(DLIST);
                    DLIST = (DLIST & 0xFC00) | ((DLIST+1) & 0x03FF);
                    MEMSCAN = (hi << 8) | lo;
                    //steal two cpu cycles
                    cyclesToSteal += 4;
                }

                //load the shift register
                UINT8 widthMode = (DMACTL & 0x03);
                if (widthMode > 0) {
                    if (widthMode < 3 && (INST & 0x10))
                        widthMode++;
                    BYTEWIDTH = BYTE_WIDTHS[(MODE-2)][widthMode-1];
//cout << "loading shift register with: " << (int)BYTEWIDTH << "  memscan: " << (int)MEMSCAN << "\n";
                    for (UINT8 i = 0; i < BYTEWIDTH; i++) {
                        SHIFT[i] = (UINT8)memoryBus->peek(MEMSCAN);
                        MEMSCAN = (MEMSCAN & 0xF000) | ((MEMSCAN+1) & 0x0FFF);
                    }
                    cyclesToSteal = (cyclesToSteal + BYTEWIDTH);
                }
                else
                    BYTEWIDTH = 0;
                BLOCKLENGTH = BLOCK_LENGTHS[(INST & 0x0F)-2];
            }
            HCOUNT = HCOUNT + cyclesToSteal;
            pinOut[ANTIC_PIN_OUT_HALT]->isHigh = TRUE;
            return cyclesToSteal;
        }
        else if (HCOUNT < TICK_LENGTH_VISIBLE_SCANLINE) {
            pinOut[ANTIC_PIN_OUT_HALT]->isHigh = FALSE;
            if (VCOUNT > 23 && VCOUNT < 216) {
//cout << "rendering line: " << VCOUNT << "   mode: " << (int)MODE << "\n";
                //render the line
                switch (MODE) {
                    case 0x00:
                       render_blank();
                       break;
                    case 0x02:
                       render_2();
                       break;
                    case 0x03:
                       render_3();
                       break;
                    case 0x04:
                       render_4();
                       break;
                    case 0x05:
                       render_5();
                       break;
                    case 0x06:
                       render_6();
                       break;
                    case 0x07:
                       render_7();
                       break;
                    case 0x08:
                       render_8();
                       break;
                    case 0x09:
                       render_9();
                       break;
                    case 0x0A:
                       render_A();
                       break;
                    case 0x0B:
                       render_B();
                       break;
                    case 0x0C:
                       render_C();
                       break;
                    case 0x0D:
                       render_D();
                       break;
                    case 0x0E:
                       render_E();
                       break;
                    case 0x0F:
                       render_F();
                       break;
                }
            }
    
            UINT8 oldh = HCOUNT;
            HCOUNT = TICK_LENGTH_VISIBLE_SCANLINE;
            return HCOUNT - oldh;
        }
        else {
            pinOut[ANTIC_PIN_OUT_HALT]->isHigh = FALSE;
            //assert READY, in case someone did a WSYNC
            pinOut[ANTIC_PIN_OUT_READY]->isHigh = TRUE;

            //increment line counter
            HCOUNT = 0;
            LCOUNT++;
            if (LCOUNT == BLOCKLENGTH) {
                LCOUNT = 0;
                //generate an NMI, if this instruction requested it
                if (INST & 0x80 && NMIEN & 0x80) {
                    NMIST |= 0x80;
                    pinOut[ANTIC_PIN_OUT_NMI]->isHigh = TRUE;
                }
            }
            VCOUNT++;
            return TICK_LENGTH_HBLANK;
        }
    }
    else {
        //VBLANK
        inVBlank = TRUE;
        if (HCOUNT == 0) {
            //kick the nmi line if we're starting vblank
            if (VCOUNT == 240) {
                if (NMIEN & 0x40) {
                    NMIST |= 0x40;
                    pinOut[ANTIC_PIN_OUT_NMI]->isHigh = TRUE;
                }
            }
            HCOUNT = TICK_LENGTH_VISIBLE_SCANLINE;
            return TICK_LENGTH_VISIBLE_SCANLINE;
        }
        else {
            HCOUNT = 0;

            //assert READY, in case someone did a WSYNC
            pinOut[ANTIC_PIN_OUT_READY]->isHigh = TRUE;
    
            //move to the next line
            VCOUNT++;
            if (VCOUNT == 262) {
                VCOUNT = 0;
                inVBlank = FALSE;
            }
            return TICK_LENGTH_HBLANK;
        }
    }
}

void Antic::render_blank()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (UINT16 i = 0; i < 20; i++) {
        UINT16 byteStart = lineStart + (i << 4);
        gtia->process(byteStart, ANBK, ANBK, ANBK, ANBK);
        gtia->process(byteStart+4, ANBK, ANBK, ANBK, ANBK);
        gtia->process(byteStart+8, ANBK, ANBK, ANBK, ANBK);
        gtia->process(byteStart+12, ANBK, ANBK, ANBK, ANBK);
    }
}

void Antic::render_2()
{
    UINT8 col0 = ANPF2;
    UINT8 col1 = (ANPF2 & 0xF0) | (ANPF1 & 0x0F);
    UINT16 lineStart = (VCOUNT-24)*320;
    for (UINT16 i = 0; i < 40; i++) {
        UINT16 byteStart = lineStart + (i << 3);
        UINT16 charAddr = ((CHBASE & 0xFC) << 8) |
                ((SHIFT[i] & 0x7F) << 3) | (LCOUNT & 0x07);
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        UINT8 an0 = ((dataByte & 0x80) ? col1 : col0);
        UINT8 an1 = ((dataByte & 0x40) ? col1 : col0);
        UINT8 an2 = ((dataByte & 0x20) ? col1 : col0);
        UINT8 an3 = ((dataByte & 0x10) ? col1 : col0);
        gtia->process(byteStart, an0, an1, an2, an3);
        an0 = ((dataByte & 0x08) ? col1 : col0);
        an1 = ((dataByte & 0x04) ? col1 : col0);
        an2 = ((dataByte & 0x02) ? col1 : col0);
        an3 = ((dataByte & 0x01) ? col1 : col0);
        gtia->process(byteStart+4, an0, an1, an2, an3);
    }
}

void Antic::render_3()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

void Antic::render_4()
{
    UINT8 colPalette[4] = { ANBK, ANPF0, ANPF1, ANPF2 };
    UINT16 lineStart = (VCOUNT-24)*320;
    for (UINT16 i = 0; i < 40; i++) {
        UINT16 byteStart = lineStart + (i << 3);
        colPalette[2] = ((SHIFT[i] & 0x80) ? ANPF2 : ANPF1);
        UINT16 charAddr = ((CHBASE & 0xFC) << 8) |
                ((SHIFT[i] & 0x7F) << 3) | (LCOUNT & 0x07);
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        UINT8 an0 = colPalette[(dataByte & 0xC0) >> 6];
        UINT8 an1 = colPalette[(dataByte & 0x30) >> 4];
        gtia->process(byteStart, an0, an0, an1, an1);
        an0 = colPalette[(dataByte & 0x0C) >> 2];
        an1 = colPalette[(dataByte & 0x03)];
        gtia->process(byteStart+4, an0, an0, an1, an1);
    }
}

void Antic::render_5()
{
    UINT8 colPalette[4] = { ANBK, ANPF0, ANPF1, ANPF2 };
    UINT16 lineStart = (VCOUNT-24)*320;
    for (UINT16 i = 0; i < 40; i++) {
        UINT16 byteStart = lineStart + (i << 3);
        colPalette[3] = (SHIFT[i] & 0x80 ? ANPF3 : ANPF2);
        UINT16 charAddr = ((CHBASE & 0xFC) << 8) |
                ((SHIFT[i] & 0x7F) << 3) | ((LCOUNT & 0x0E) >> 1);
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        UINT8 an0 = colPalette[(dataByte & 0xC0) >> 6];
        UINT8 an1 = colPalette[(dataByte & 0x30) >> 4];
        gtia->process(byteStart, an0, an0, an1, an1);
        an0 = colPalette[(dataByte & 0x0C) >> 2];
        an1 = colPalette[(dataByte & 0x03)];
        gtia->process(byteStart+4, an0, an0, an1, an1);
    }
}

void Antic::render_6()
{
    UINT8 colPalette[4] = { ANPF0, ANPF1, ANPF2, ANPF3 };
    UINT8 bgcolor = ANBK;
    UINT16 lineStart = (VCOUNT-24)*320;
    for (UINT16 i = 0; i < 20; i++) {
        UINT16 byteStart = lineStart + (i << 4);
        UINT16 charAddr = ((CHBASE & 0xFE) << 8) |
                ((SHIFT[i] & 0x3F) << 3) | (LCOUNT & 0x07);
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        UINT8 fgcolor = colPalette[(SHIFT[i] & 0xC0) >> 6];
        UINT8 an0 = ((dataByte & 0x80) ? fgcolor : bgcolor);
        UINT8 an1 = ((dataByte & 0x40) ? fgcolor : bgcolor);
        gtia->process(byteStart, an0, an0, an1, an1);
        an0 = ((dataByte & 0x20) ? fgcolor : bgcolor);
        an1 = ((dataByte & 0x10) ? fgcolor : bgcolor);
        gtia->process(byteStart+4, an0, an0, an1, an1);
        an0 = ((dataByte & 0x08) ? fgcolor : bgcolor);
        an1 = ((dataByte & 0x04) ? fgcolor : bgcolor);
        gtia->process(byteStart+8, an0, an0, an1, an1);
        an0 = ((dataByte & 0x02) ? fgcolor : bgcolor);
        an1 = ((dataByte & 0x01) ? fgcolor : bgcolor);
        gtia->process(byteStart+12, an0, an0, an1, an1);
    }
}

void Antic::render_7()
{
    UINT8 bgcolor = ANBK;
    UINT16 lineStart = (VCOUNT-24)*320;
    for (UINT16 i = 0; i < 20; i++) {
        UINT16 byteStart = lineStart + (i << 4);
        UINT8 col = (SHIFT[i] & 0xC0) >> 6;
        col = (col == 0 ? ANPF0 : (col == 1 ? ANPF1
                : (col == 2 ? ANPF2 : ANPF3)));
        UINT16 charAddr = ((CHBASE & 0xFE) << 8) |
                ((SHIFT[i] & 0x3F) << 3) | ((LCOUNT & 0x0E) >> 1);
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        UINT8 an0 = (dataByte & 0x80 ? col : bgcolor);
        UINT8 an1 = (dataByte & 0x40 ? col : bgcolor);
        gtia->process(byteStart, an0, an0, an1, an1);
        an0 = (dataByte & 0x20 ? col : bgcolor);
        an1 = (dataByte & 0x10 ? col : bgcolor);
        gtia->process(byteStart+4, an0, an0, an1, an1);
        an0 = (dataByte & 0x08 ? col : bgcolor);
        an1 = (dataByte & 0x04 ? col : bgcolor);
        gtia->process(byteStart+8, an0, an0, an1, an1);
        an0 = (dataByte & 0x02 ? col : bgcolor);
        an1 = (dataByte & 0x01 ? col : bgcolor);
        gtia->process(byteStart+12, an0, an0, an1, an1);
    }
}

void Antic::render_8()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

void Antic::render_9()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

void Antic::render_A()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

void Antic::render_B()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

void Antic::render_C()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

void Antic::render_D()
{
    UINT8 colPalette[4] = { ANBK, ANPF0, ANPF1, ANPF2 };
    UINT16 lineStart = (VCOUNT-24)*320;
    for (UINT16 i = 0; i < BYTEWIDTH; i++) {
        UINT16 byteStart = lineStart + (i << 3);
        UINT8 an0 = colPalette[(SHIFT[i] & 0xC0) >> 6];
        UINT8 an1 = colPalette[(SHIFT[i] & 0x30) >> 4];
        gtia->process(byteStart, an0, an0, an1, an1);
        an0 = colPalette[(SHIFT[i] & 0x0C) >> 2];
        an1 = colPalette[(SHIFT[i] & 0x03)];
        gtia->process(byteStart+4, an0, an0, an1, an1);
    }
}

void Antic::render_E()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

void Antic::render_F()
{
    UINT16 lineStart = (VCOUNT-24)*320;
    for (int i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() & 0xFF);
}

