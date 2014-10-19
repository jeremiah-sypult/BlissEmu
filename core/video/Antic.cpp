
#include "Antic.h"
#include "core/cpu/ProcessorBus.h"
#include <time.h>
#include <stdlib.h> // jeremiah sypult - srand

#define IMAGE_BANK_LENGTH 76800
        
const UINT8 Antic::ANBK  = 0x0;
const UINT8 Antic::ANPF0 = 0x4;
const UINT8 Antic::ANPF1 = 0x5;
const UINT8 Antic::ANPF2 = 0x6;
const UINT8 Antic::ANPF3 = 0x7;
const UINT8 Antic::AN_SPECIAL = 0x1;

const UINT8 Antic::BLOCK_HEIGHTS[14] = { 8, 10, 8, 8, 8, 16, 8, 4, 4, 2, 1, 2, 1, 1 };

const UINT8 Antic::BYTE_WIDTHS[14][4] = {
        //text antic modes
        { 1, 32, 40, 48 },
        { 1, 32, 40, 48 },
        { 1, 32, 40, 48 },
        { 1, 32, 40, 48 },
        { 1, 16, 20, 24 },
        { 1, 16, 20, 24 },
        //graphics antic modes
        { 1,  8, 10, 12 },
        { 1,  8, 10, 12 },
        { 1, 16, 20, 24 },
        { 1, 16, 20, 24 },
        { 1, 16, 20, 24 },
        { 1, 32, 40, 48 },
        { 1, 32, 40, 48 },
        { 1, 32, 40, 48 } };

const UINT32 Antic::palette[256] = {
    0x323132, 0x3f3e3f, 0x4d4c4d, 0x5b5b5b, 0x6a696a, 0x797879, 0x888788, 0x979797,
    0xa1a0a1, 0xafafaf, 0xbebebe, 0xcecdce, 0xdbdbdb, 0xebeaeb, 0xfafafa, 0xffffff,
    0x612e00, 0x6c3b00, 0x7a4a00, 0x885800, 0x94670c, 0xa5761b, 0xb2842a, 0xc1943a,
    0xca9d43, 0xdaad53, 0xe8bb62, 0xf8cb72, 0xffd87f, 0xffe88f, 0xfff79f, 0xffffae,
    0x6c2400, 0x773000, 0x844003, 0x924e11, 0x9e5d22, 0xaf6c31, 0xbc7b41, 0xcc8a50,
    0xd5935b, 0xe4a369, 0xf2b179, 0xffc289, 0xffcf97, 0xffdfa6, 0xffedb5, 0xfffdc4,
    0x751618, 0x812324, 0x8f3134, 0x9d4043, 0xaa4e50, 0xb85e60, 0xc66d6f, 0xd57d7f,
    0xde8787, 0xed9596, 0xfca4a5, 0xffb4b5, 0xffc2c4, 0xffd1d3, 0xffe0e1, 0xffeff0,
    0x620e71, 0x6e1b7c, 0x7b2a8a, 0x8a3998, 0x9647a5, 0xa557b5, 0xb365c3, 0xc375d1,
    0xcd7eda, 0xdc8de9, 0xea97f7, 0xf9acff, 0xffbaff, 0xffc9ff, 0xffd9ff, 0xffe8ff,
    0x560f87, 0x611d90, 0x712c9e, 0x7f3aac, 0x8d48ba, 0x9b58c7, 0xa967d5, 0xb877e5,
    0xc280ed, 0xd090fc, 0xdf9fff, 0xeeafff, 0xfcbdff, 0xffccff, 0xffdbff, 0xffeaff,
    0x461695, 0x5122a0, 0x6032ac, 0x6e41bb, 0x7c4fc8, 0x8a5ed6, 0x996de3, 0xa87cf2,
    0xb185fb, 0xc095ff, 0xcfa3ff, 0xdfb3ff, 0xeec1ff, 0xfcd0ff, 0xffdfff, 0xffefff,
    0x212994, 0x2d359f, 0x3d44ad, 0x4b53ba, 0x5961c7, 0x686fd5, 0x777ee2, 0x878ef2,
    0x9097fa, 0x96a6ff, 0xaeb5ff, 0xbfc4ff, 0xcdd2ff, 0xdae3ff, 0xeaf1ff, 0xfafeff,
    0x0f3584, 0x1c418d, 0x2c509b, 0x3a5eaa, 0x486cb7, 0x587bc5, 0x678ad2, 0x7699e2,
    0x80a2eb, 0x8fb2f9, 0x9ec0ff, 0xadd0ff, 0xbdddff, 0xcbecff, 0xdbfcff, 0xeaffff,
    0x043f70, 0x114b79, 0x215988, 0x2f6896, 0x3e75a4, 0x4d83b2, 0x5c92c1, 0x6ca1d2,
    0x74abd9, 0x83bae7, 0x93c9f6, 0xa2d8ff, 0xb1e6ff, 0xc0f5ff, 0xd0ffff, 0xdeffff,
    0x005918, 0x006526, 0x0f7235, 0x1d8144, 0x2c8e50, 0x3b9d60, 0x4aac6f, 0x59bb7e,
    0x63c487, 0x72d396, 0x82e2a5, 0x92f1b5, 0x9ffec3, 0xaeffd2, 0xbeffe2, 0xcefff1,
    0x075c00, 0x146800, 0x227500, 0x328300, 0x3f910b, 0x4fa01b, 0x5eae2a, 0x6ebd3b,
    0x77c644, 0x87d553, 0x96e363, 0xa7f373, 0xb3fe80, 0xc3ff8f, 0xd3ffa0, 0xe3ffb0,
    0x1a5600, 0x286200, 0x367000, 0x457e00, 0x538c00, 0x629b07, 0x70a916, 0x80b926,
    0x89c22f, 0x99d13e, 0xa8df4d, 0xb7ef5c, 0xc5fc6b, 0xd5ff7b, 0xe3ff8b, 0xf3ff99,
    0x334b00, 0x405700, 0x4d6500, 0x5d7300, 0x6a8200, 0x7a9100, 0x889e0f, 0x98ae1f,
    0xa1b728, 0xbac638, 0xbfd548, 0xcee458, 0xdcf266, 0xebff75, 0xfaff85, 0xffff95,
    0x4b3c00, 0x584900, 0x655700, 0x746500, 0x817400, 0x908307, 0x9f9116, 0xaea126,
    0xb7aa2e, 0xc7ba3e, 0xd5c74d, 0xe5d75d, 0xf2e56b, 0xfef47a, 0xffff8b, 0xffff9a,
    0x602e00, 0x6d3a00, 0x7a4900, 0x895800, 0x95670a, 0xa4761b, 0xb2832a, 0xc2943a,
    0xcb9d44, 0xdaac53, 0xe8ba62, 0xf8cb73, 0xffd77f, 0xffe791, 0xfff69f, 0xffffaf,
    };
      
Antic::Antic(MemoryBus* mb, GTIA* gt)
: Processor("ANTIC"),
  memoryBus(mb),
  gtia(gt),
  anticMode(START_HSYNC)
{
    registers.init(this);
    this->imageBank = gtia->imageBank;
}
           
void Antic::resetProcessor()
{
    pinOut[ANTIC_PIN_OUT_NMI]->isHigh = TRUE;
    pinOut[ANTIC_PIN_OUT_HALT]->isHigh = TRUE;
    pinOut[ANTIC_PIN_OUT_READY]->isHigh = TRUE;

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
    
    anticMode = VBLANK;
    cyclesToSteal = 0;

    srand((unsigned int)time(NULL));
}

void Antic::setPixelBuffer(UINT32* pixelBuffer, UINT32 rowSize)
{
	Antic::pixelBuffer = pixelBuffer;
	Antic::pixelBufferRowSize = rowSize;
}

void Antic::render()
{
	// the video bus handles rendering.
}

INT32 Antic::tick(INT32 minimum)
{
    INT32 usedCycles = 0;
    do {

    switch (anticMode) {

        case START_HSYNC:  //hclock == 0
            HCOUNT = 16;
            anticMode = END_HSYNC;
            usedCycles += 16;
            break;

        case END_HSYNC:  //hclock == 16
            HCOUNT = 34;
            anticMode = START_DISPLAY;
            usedCycles += 18;
            break;

        case START_DISPLAY:  //hclock == 34; vclock >= 8 && vclock < 248
            //render the left border, wide=10, regular=14, narrow=30
            switch (DMACTL & 0x03) {
                case 0x00:
                    //no instruction DMA; blank line
                    renderLine();
                    anticMode = END_WSYNC;
                    HCOUNT = 208;
                    usedCycles += 174;
                    break;
                case 0x01:
                    //narrow playfield
                    anticMode = START_NARROW_PLAYFIELD;
                    HCOUNT = 64;
                    usedCycles += 30;
                    break;
                case 0x02:
                    //regular playfield
                    anticMode = START_REGULAR_PLAYFIELD;
                    HCOUNT = 48;
                    usedCycles += 14;
                    break;
                case 0x03:
                    //wide playfield
                    anticMode = START_WIDE_PLAYFIELD;
                    HCOUNT = 44;    
                    usedCycles += 10;
                    break;
            }
            break;
            
        case START_NARROW_PLAYFIELD:  //hclock == 64
            //render the playfield area, 128 clocks
            renderLine();
            anticMode = END_NARROW_PLAYFIELD;
            HCOUNT = 192;
            usedCycles += 128;
            break;
            
        case END_NARROW_PLAYFIELD:  //hclock == 192
            //render partial right border, 16 clocks
            anticMode = END_WSYNC;
            HCOUNT = 208;
            usedCycles += 16;
            break;
            
        case START_REGULAR_PLAYFIELD:  //hclock == 48
            //render the playfield area, 160 clocks
            renderLine();
            HCOUNT = 208;
            anticMode = END_REGULAR_PLAYFIELD;
            usedCycles += 160;
            break;

        case END_REGULAR_PLAYFIELD:  //hclock == 208
            //end WSYNC and render the right border, 14 clocks
            pinOut[ANTIC_PIN_OUT_READY]->isHigh = TRUE;
            anticMode = START_HBLANK;
            HCOUNT = 222;
            usedCycles += 14;
            break;
            
        case START_WIDE_PLAYFIELD: //hclock == 44
            //render part of the playfield area, 164 clocks
            renderLine();
            anticMode = END_WSYNC_DURING_WIDE;
            HCOUNT = 208;
            usedCycles += 164;
            break;
            
        case END_WSYNC_DURING_WIDE:  //hclock = 208
            //assert READY, in case someone did a WSYNC
            pinOut[ANTIC_PIN_OUT_READY]->isHigh = TRUE;
            anticMode = END_WIDE_PLAYFIELD;
            HCOUNT = 220;
            usedCycles += 12;
            break;
            
        case END_WIDE_PLAYFIELD:  //hclock == 220
            //render the right border, 2 clocks
            anticMode = START_HBLANK;
            HCOUNT = 222;
            usedCycles += 2;
            break;
            
        case END_WSYNC:  //hclock = 208
            //assert READY, in case someone did a WSYNC
            pinOut[ANTIC_PIN_OUT_READY]->isHigh = TRUE;
            
            anticMode = START_HBLANK;
            HCOUNT = 222;
            usedCycles += 14;
            break;
        
        case START_HBLANK: //hclock = 222
            //prefetch the instruction to execute during the next line
            if (LCOUNT == BLOCKLENGTH) {
                //done with this instruction
                LCOUNT = 0;
                //generate an NMI if requested
                if ((INST & NMIEN & 0x80) != 0) {
                    NMIST |= 0x80;
                    pinOut[ANTIC_PIN_OUT_NMI]->isHigh = FALSE;
                }
                
                //if instruction DMA is disabled, just blank the next line
                if ((DMACTL & 0x20) == 0) {
                    MODE = 0;
                    BLOCKLENGTH = 1;
                    /*
                    VCOUNT++;
                    HCOUNT = 208;
                    anticMode = END_WSYNC;
                    usedCycles += 214;
                    */
                }
                else {
                    //fetch and decode the next instruction
                    fetchAndDecode();
                
                    //load the SHIFT register; should steal cycles for this, but oh well
                    for (UINT8 i = 0; i < BYTEWIDTH; i++) {
                        SHIFT[i] = (UINT8)memoryBus->peek(MEMSCAN);
                        MEMSCAN = (UINT16)((MEMSCAN & 0xF000) | ((MEMSCAN+1) & 0x0FFF));
                    }

                    /*
                    VCOUNT++;
                    HCOUNT = (UINT8)(HCOUNT+cyclesToSteal);
                    pinOut[ANTIC_PIN_OUT_HALT].isHigh = true;
                    anticMode = END_CYCLE_STEALING;
                    afterCycleStealingMode = START_HSYNC;
                    usedCycles += cyclesToSteal;
                    */
                }
            }
            
            //do player-missile DMA to the GTIA player-missile graphics registers
            if ((DMACTL & 0x08) && (gtia->GRACTL & 0x02)) {
                if (DMACTL & 0x10) {
                    for (INT32 i = 0; i < 4; i++)
                        gtia->GRAFP[i] = (UINT8)memoryBus->peek((UINT16)(((PMBASE&0xF8)<<8)|0x0400|(i<<8)|VCOUNT));
                    gtia->GRAFM = (UINT8)memoryBus->peek((UINT16)(((PMBASE&0xF8)<<8)|0x0300|VCOUNT));
                }
                else {
                    for (INT32 i = 0; i < 4; i++)
                        gtia->GRAFP[i] = (UINT8)memoryBus->peek((UINT16)(((PMBASE&0xFC)<<8)|0x0200|(i<<7)|(VCOUNT>>1)));
                    gtia->GRAFM = (UINT8)memoryBus->peek((UINT16)(((PMBASE&0xFC)<<8)|0x0180|VCOUNT));
                }
            }
            
            //now move along to the next line
            HCOUNT = 0;
            VCOUNT++;
            if (VCOUNT == 248)
                anticMode = START_VBLANK;
            else
                anticMode = START_HSYNC;
            usedCycles += 6;
            break;
            
        case START_VBLANK:
            processorBus->stop();
            //kick the nmi line
            if (NMIEN & 0x40) {
                NMIST |= 0x40;
                pinOut[ANTIC_PIN_OUT_NMI]->isHigh = FALSE;
            }
            VCOUNT++;
            anticMode = VBLANK;
            usedCycles += 228;
            break;

        case VBLANK:
            VCOUNT++;
            if (VCOUNT == 262)
                VCOUNT = 0;
            else if (VCOUNT == 8)
                anticMode = START_HSYNC;
            usedCycles += 228;
            break;

        //this is a generic mode use to end a session of cycle stealing from the CPU
        case END_CYCLE_STEALING:  //hclock == ???
#if 0
            pinOut[ANTIC_PIN_OUT_HALT]->isHigh = TRUE;
            HCOUNT = (UINT8)(HCOUNT+cyclesToSteal);
            cyclesToSteal = 0;
            //anticMode = afterCycleStealingMode;
            usedCycles += cyclesToSteal;
#endif
            break;
    }
    
    } while (usedCycles < minimum);
    return usedCycles;
}

void Antic::renderLine()
{
    switch (MODE) 
    {
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
        case 0x05:
            render_4_5();
            break;
        case 0x06:
        case 0x07:
            render_6_7();
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
    
    //render the four players
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 4; i++) {
        UINT8 nextPos = gtia->HPOSP[i];
        if (nextPos < 0x30 || nextPos > 0xC8)
            continue;

        UINT32 byteStart = (lineStart + (2*(nextPos-0x30)));
        renderPlayerGraphic(imageBank+byteStart, gtia->GRAFP[i], gtia->COLPM[i]);
    }

    //render the four missiles or the fifth player
    UINT8 nextPos = gtia->HPOSM[0];
    if (nextPos >= 0x30 && nextPos <= 0xC8) {
        UINT32 byteStart = (lineStart + (2*(nextPos-0x30)));
        renderPlayerGraphic(imageBank+byteStart, gtia->GRAFM, gtia->COLPF[3]);
    }
    
    LCOUNT++;
}

void Antic::renderPlayerGraphic(UINT8* ptr, UINT8 grafx, UINT8 color)
{
    if (grafx & 0x80) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
    ptr += 2;
    if (grafx & 0x40) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
    ptr += 2;
    if (grafx & 0x20) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
    ptr += 2;
    if (grafx & 0x10) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
    ptr += 2;
    if (grafx & 0x08) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
    ptr += 2;
    if (grafx & 0x04) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
    ptr += 2;
    if (grafx & 0x02) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
    ptr += 2;
    if (grafx & 0x01) {
        *(ptr) = color;
        *(ptr+1) = color;
    }
}

void Antic::fetchAndDecode()
{
    //fetch the next instruction, and steal one cpu cycle to do it
    INST = (UINT8)memoryBus->peek(DLIST);
    DLIST = (UINT16)((DLIST & 0xFC00) | ((DLIST+1) & 0x03FF));
    cyclesToSteal = 2;

    //prepare to execute a regular render instruction, steal cycles
    MODE = (UINT8)(INST & 0x0F);
    if (MODE == 0x00) {
        //blank the requested number of lines
        BYTEWIDTH = 0;
        BLOCKLENGTH = (UINT8)(((INST & 0x70) >> 4)+1);
    }
    else if (MODE == 0x01) {
        //execute a display list jump instruction, steal total of 3 cpu cycles
        UINT8 lo = (UINT8)memoryBus->peek(DLIST);
        DLIST = (UINT16)((DLIST & 0xFC00) | ((DLIST+1) & 0x03FF));
        UINT8 hi = (UINT8)memoryBus->peek(DLIST);
        DLIST = (UINT16)((hi << 8) | lo);
        
        //steal two more cpu cycles
        cyclesToSteal += 4;

        MODE = 0;
        BYTEWIDTH = 0;
        if ((INST & 0x40) != 0)
            BLOCKLENGTH = (UINT8)(248-VCOUNT);
        else
            BLOCKLENGTH = 1;
    }
    else {
        //reload memscan if necessary
        if ((INST & 0x40) != 0) 
        {
            UINT8 lo = (UINT8)memoryBus->peek(DLIST);
            DLIST = (UINT16)((DLIST & 0xFC00) | ((DLIST+1) & 0x03FF));
            UINT8 hi = (UINT8)memoryBus->peek(DLIST);
            DLIST = (UINT16)((DLIST & 0xFC00) | ((DLIST+1) & 0x03FF));
            MEMSCAN = (UINT16)((hi << 8) | lo);
            //steal two cpu cycles
            cyclesToSteal += 4;
        }

        //determine the size of the shift register
        BYTEWIDTH = BYTE_WIDTHS[(MODE-2)][(DMACTL & 0x03)];
        BLOCKLENGTH = BLOCK_HEIGHTS[(INST & 0x0F)-2];
    }
}

void Antic::render_blank()
{
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 20; i++) 
    {
        UINT32 byteStart = (lineStart + (i << 4));
        memset(imageBank+byteStart, gtia->COLBK, 16);
    }
}

void Antic::render_2()
{
    UINT8 mixedColor = (gtia->COLPF[2] & 0xF0) | (gtia->COLPF[1] & 0x0F);
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 40; i++) {
        UINT32 byteStart = (lineStart + (i << 3));
        UINT16 charAddr = (UINT16)(((CHBASE & 0xFC) << 8) |
                ((SHIFT[i] & 0x7F) << 3) | (LCOUNT & 0x07));
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        imageBank[byteStart++] = dataByte & 0x80 ? mixedColor : gtia->COLPF[2];
        imageBank[byteStart++] = dataByte & 0x40 ? mixedColor : gtia->COLPF[2];
        imageBank[byteStart++] = dataByte & 0x20 ? mixedColor : gtia->COLPF[2];
        imageBank[byteStart++] = dataByte & 0x10 ? mixedColor : gtia->COLPF[2];
        imageBank[byteStart++] = dataByte & 0x08 ? mixedColor : gtia->COLPF[2];
        imageBank[byteStart++] = dataByte & 0x04 ? mixedColor : gtia->COLPF[2];
        imageBank[byteStart++] = dataByte & 0x02 ? mixedColor : gtia->COLPF[2];
        imageBank[byteStart++] = dataByte & 0x01 ? mixedColor : gtia->COLPF[2];
    }
}

void Antic::render_3()
{
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() % 256);
}

void Antic::render_4_5()
{
    UINT8 anPalette[4] = { ANBK, ANPF0, ANPF2, ANPF3 };
    UINT8 colorPalette[4] = { gtia->COLBK, gtia->COLPF[0], gtia->COLPF[2], gtia->COLPF[3] };
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 40; i++) {
        UINT32 byteStart = (lineStart + (i << 3));
        anPalette[2] = (UINT8)((SHIFT[i] & 0x80) == 0 ? ANPF1 : ANPF2);
        colorPalette[2] = (UINT8)((SHIFT[i] & 0x80) == 0 ? gtia->COLPF[1] : gtia->COLPF[2]);
        UINT16 charAddr = (UINT16)(((CHBASE & 0xFC) << 8) | ((SHIFT[i] & 0x7F) << 3));
        charAddr |= (MODE == 4 ? LCOUNT & 0x07 : (LCOUNT & 0x0E) >> 1);
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        UINT8 color = colorPalette[(dataByte & 0xC0) >> 6];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(dataByte & 0x30) >> 4];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(dataByte & 0x0C) >> 2];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[dataByte & 0x03];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
    }
}

void Antic::render_6_7()
{
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 20; i++) 
    {
        UINT32 byteStart = (lineStart + (i << 4));
        UINT16 charAddr = (UINT16)(((CHBASE & 0xFE) << 8) |
            ((SHIFT[i] & 0x3F) << 3));
        charAddr |= (MODE == 6 ? LCOUNT & 0x07 : (LCOUNT & 0x0E) >> 1);
        UINT8 dataByte = (UINT8)memoryBus->peek(charAddr);
        UINT8 fgcolor = gtia->COLPF[(SHIFT[i] & 0xC0) >> 6];
        UINT8 color = (dataByte & 0x80 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (dataByte & 0x40 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (dataByte & 0x20 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (dataByte & 0x10 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (dataByte & 0x08 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (dataByte & 0x04 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (dataByte & 0x02 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (dataByte & 0x01 ? fgcolor : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
    }
}

void Antic::render_8()
{
    UINT8 colorPalette[4] = { gtia->COLBK, gtia->COLPF[0], gtia->COLPF[1], gtia->COLPF[2] };
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < BYTEWIDTH; i++) 
    {
        UINT32 byteStart = (lineStart + (i << 3));
        UINT8 dataByte = SHIFT[i];
        memset(imageBank+byteStart, colorPalette[(dataByte & 0xC0) >> 6], 8);
        memset(imageBank+byteStart+8, colorPalette[(dataByte & 0x30) >> 4], 8);
        memset(imageBank+byteStart+16, colorPalette[(dataByte & 0x0C) >> 2], 8);
        memset(imageBank+byteStart+24, colorPalette[dataByte & 0x03], 8);
    }
}

void Antic::render_9()
{
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() % 256);
}

void Antic::render_A()
{
    UINT8 colorPalette[4] = { gtia->COLBK, gtia->COLPF[0], gtia->COLPF[1], gtia->COLPF[2] };
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < BYTEWIDTH; i++) 
    {
        UINT32 byteStart = (lineStart + (i << 4));
        UINT8 dataByte = SHIFT[i];
        UINT8 color = colorPalette[(dataByte & 0xC0)>>6];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(dataByte & 0x30)>>4];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(dataByte & 0x0C)>>2];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[dataByte & 0x03];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
    }
}

void Antic::render_B()
{
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < BYTEWIDTH; i++) {
        UINT32 byteStart = (lineStart + (i << 4));
        UINT8 color = (SHIFT[i] & 0x80 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (SHIFT[i] & 0x40 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (SHIFT[i] & 0x20 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (SHIFT[i] & 0x10 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (SHIFT[i] & 0x08 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (SHIFT[i] & 0x04 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (SHIFT[i] & 0x02 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = (SHIFT[i] & 0x01 ? gtia->COLPF[0] : gtia->COLBK);
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
    }
}

void Antic::render_C()
{
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < 320; i++)
        imageBank[lineStart+i] = (UINT8)(rand() % 256);
}

void Antic::render_D()
{
    //UINT8 anPalette[4] = { ANBK, ANPF0, ANPF1, ANPF2 };
    UINT8 colorPalette[4] = { gtia->COLBK, gtia->COLPF[0], gtia->COLPF[1], gtia->COLPF[2] };
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < BYTEWIDTH; i++) {
        UINT32 byteStart = (lineStart + (i << 3));
        UINT8 color = colorPalette[(SHIFT[i] & 0xC0) >> 6];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(SHIFT[i] & 0x30) >> 4];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(SHIFT[i] & 0x0C) >> 2];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[SHIFT[i] & 0x03];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
    }
}

void Antic::render_E()
{
    UINT8 colorPalette[4] = { gtia->COLBK, gtia->COLPF[0], gtia->COLPF[1], gtia->COLPF[2] };
    UINT32 lineStart = ((VCOUNT-8)*320);
    for (INT32 i = 0; i < BYTEWIDTH; i++) 
    {
        UINT32 byteStart = (lineStart + (i << 3));
        UINT8 color = colorPalette[(SHIFT[i] & 0xC0) >> 6];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(SHIFT[i] & 0x30) >> 4];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[(SHIFT[i] & 0x0C) >> 2];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
        color = colorPalette[SHIFT[i] & 0x03];
        imageBank[byteStart++] = color;
        imageBank[byteStart++] = color;
    }
}

void Antic::render_F()
{
    UINT32 lineStart = ((VCOUNT-8)*320);
    UINT8 mixedColor = (gtia->COLPF[2] & 0xF0) | (gtia->COLPF[1] & 0x0F);
    for (INT32 i = 0; i < BYTEWIDTH; i++) 
    {
        UINT32 byteStart = (lineStart + (i << 3));
        UINT8 dataByte = SHIFT[i];
        imageBank[byteStart++] = (dataByte & 0x80 ? mixedColor : gtia->COLPF[2]);
        imageBank[byteStart++] = (dataByte & 0x40 ? mixedColor : gtia->COLPF[2]);
        imageBank[byteStart++] = (dataByte & 0x20 ? mixedColor : gtia->COLPF[2]);
        imageBank[byteStart++] = (dataByte & 0x10 ? mixedColor : gtia->COLPF[2]);
        imageBank[byteStart++] = (dataByte & 0x08 ? mixedColor : gtia->COLPF[2]);
        imageBank[byteStart++] = (dataByte & 0x04 ? mixedColor : gtia->COLPF[2]);
        imageBank[byteStart++] = (dataByte & 0x02 ? mixedColor : gtia->COLPF[2]);
        imageBank[byteStart++] = (dataByte & 0x01 ? mixedColor : gtia->COLPF[2]);
    }
}
