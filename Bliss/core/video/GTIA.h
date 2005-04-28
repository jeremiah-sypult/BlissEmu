
#ifndef GTIA_H
#define GTIA_H

#include "GTIA_Registers.h"
#include "core/cpu/Processor.h"

class GTIA : public Processor
{

    friend class Antic;
    friend class GTIA_Registers;

    public:
        GTIA();
        GTIA_Registers* getRegisters();

		void resetProcessor() {}
        INT32 getClockSpeed();
        INT32 tick(INT32);

    private:
        void process(UINT16 location, UINT8 an0, UINT8 an1, UINT8 an2,
                UINT8 an3);
        UINT8 imageBank[320*192];
        GTIA_Registers registers;

        UINT8 HPOSP0;
        UINT8 HPOSP1;
        UINT8 HPOSP2;
        UINT8 HPOSP3;
        UINT8 HPOSM0;
        UINT8 HPOSM1;
        UINT8 HPOSM2;
        UINT8 HPOSM3;
        UINT8 SIZEP0;
        UINT8 SIZEP1;
        UINT8 SIZEP2;
        UINT8 SIZEP3;
        UINT8 SIZEM;
        UINT8 GRAFP0;
        UINT8 GRAFP1;
        UINT8 GRAFP2;
        UINT8 GRAFP3;
        UINT8 GRAFM;
        UINT8 COLPM0;
        UINT8 COLPM1;
        UINT8 COLPM2;
        UINT8 COLPM3;
        UINT8 COLPF0;
        UINT8 COLPF1;
        UINT8 COLPF2;
        UINT8 COLPF3;
        UINT8 COLBK;
        UINT8 PRIOR;
        UINT8 VDELAY;
        UINT8 GRACTL;
        UINT8 CONSOL;

};

#endif
