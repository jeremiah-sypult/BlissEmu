
#ifndef ANTIC_H
#define ANTIC_H

#include "Antic_Registers.h"
#include "VideoProducer.h"
#include "GTIA.h"
#include "core/cpu/Processor.h"
#include "core/cpu/SignalLine.h"
#include "core/memory/MemoryBus.h"

#define ANTIC_PIN_OUT_NMI   0
#define ANTIC_PIN_OUT_HALT  1
#define ANTIC_PIN_OUT_READY 2

typedef enum _AnticMode
{
    START_HSYNC,
    END_HSYNC,
    START_DISPLAY,
    START_WIDE_PLAYFIELD,
    START_REGULAR_PLAYFIELD,
    START_NARROW_PLAYFIELD,
    END_WSYNC,
    END_WSYNC_DURING_WIDE,
    END_NARROW_PLAYFIELD,
    END_REGULAR_PLAYFIELD,
    END_WIDE_PLAYFIELD,
    START_HBLANK,
    START_VBLANK,
    VBLANK,
    END_CYCLE_STEALING,
} AnticMode;

class Antic : public Processor, public VideoProducer
{
    friend class Antic_Registers;

public:
    Antic_Registers registers;

    Antic(MemoryBus* mb, GTIA* gt);
        
    void resetProcessor();
	void setPixelBuffer(UINT32* pixelBuffer, UINT32 rowSize);

    void render();

    INT32 getClockSpeed() { return 3584160; }

    INT32 tick(INT32 minimum);
    

private:

    void renderPlayerGraphic(UINT8* ptr, UINT8 grafx, UINT8 color);
    
    const static UINT8 ANBK;
    const static UINT8 ANPF0;
    const static UINT8 ANPF1;
    const static UINT8 ANPF2;
    const static UINT8 ANPF3;
    const static UINT8 AN_SPECIAL;

    const static UINT8 BLOCK_HEIGHTS[14];
    const static UINT8 BYTE_WIDTHS[14][4];
            
    const static UINT32 palette[256];

    MemoryBus* memoryBus;
    GTIA* gtia;

    UINT8 INST;
    UINT8 LCOUNT;
    UINT8 HCOUNT;
    UINT16 MEMSCAN;
    UINT8 MODE;
    UINT8 BYTEWIDTH;
    UINT8 BLOCKLENGTH;
    UINT8* imageBank;

    //registers
    UINT8  SHIFT[48];
    UINT8  DMACTL;
    UINT8  CHACTL;
    UINT16 DLIST;
    UINT8  HSCROL;
    UINT8  VSCROL;
    UINT8  PMBASE;
    UINT8  CHBASE;
    UINT16 VCOUNT;
    UINT8  NMIEN;
    UINT8  NMIST;
    
    AnticMode anticMode;
    
    UINT16 cyclesToSteal;
    //private AnticMode afterCycleStealingMode;

	UINT32*					pixelBuffer;
	UINT32					pixelBufferRowSize;

private:
    void renderLine();
    
    void fetchAndDecode();
    
    void render_blank();
    void render_2();
    void render_3();
    void render_4_5();
    void render_6_7();
    void render_8();
    void render_9();
    void render_A();
    void render_B();
    void render_C();
    void render_D();
    void render_E();
    void render_F();

};

#endif

/*
class Antic : public Processor, public VideoProducer
{
    friend class Antic_Registers;

    public:
        Antic();
        Antic(MemoryBus* memoryBus, GTIA* gtia);
        BOOL inVerticalBlank() { return inVBlank; }
        Antic_Registers* getRegisters();
        void resetProcessor();
        INT32 getClockSpeed();
        INT32 tick(INT32);
		void setVideoOutputDevice(IDirect3DDevice9*);

        void getOutputImageSize(UINT16* width, UINT16* height);
        void getPalette(const UINT32** palette, UINT16* numEntries);
        void render();

    private:
        void render_blank();
        void render_2();
        void render_3();
        void render_4();
        void render_5();
        void render_6();
        void render_7();
        void render_8();
        void render_9();
        void render_A();
        void render_B();
        void render_C();
        void render_D();
        void render_E();
        void render_F();

        Antic_Registers registers;
        MemoryBus* memoryBus;
        GTIA* gtia;
        BOOL inVBlank;

        UINT8 INST;
        UINT8 LCOUNT;
        UINT8 HCOUNT;
        UINT16 MEMSCAN;
        UINT8 MODE;
        UINT8 BYTEWIDTH;
        UINT8 BLOCKLENGTH;
        UINT8* imageBank;

        //registers
        UINT8  SHIFT[48];
        UINT8  DMACTL;
        UINT8  CHACTL;
        UINT16 DLIST;
        UINT8  HSCROL;
        UINT8  VSCROL;
        UINT8  PMBASE;
        UINT8  CHBASE;
        UINT16 VCOUNT;
        UINT8  NMIEN;
        UINT8  NMIST;

        const static UINT8 BLOCK_LENGTHS[14];
        const static UINT8 BYTE_WIDTHS[14][3];
        const static UINT32 palette[256];

};

#endif
*/