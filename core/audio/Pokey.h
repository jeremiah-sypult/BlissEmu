
#ifndef POKEY_H
#define POKEY_H

#include "AudioProducer.h"
#include "Pokey_Registers.h"
#include "Pokey_Input.h"
#include "core/cpu/Processor.h"

#define POKEY_PIN_OUT_IRQ   0

class Pokey : public Processor, public AudioProducer
{
    friend class Pokey_Registers;

    public:
        Pokey_Registers registers;

        Pokey(Pokey_Input* left, Pokey_Input* right)
            : Processor("POKEY"),
              leftInput(left),
              rightInput(right)
        {
            registers.init(this);
        }

        void resetProcessor();
        INT32 getClockSpeed() { return 1792080; }
        INT32 getClocksPerSample() { return 1; }
        INT32 getSampleRate() { return getClockSpeed(); }

        INT32 tick(INT32 minimum);

    private:
        Pokey_Input* leftInput;
        Pokey_Input* rightInput;

        UINT8 AUDF1;
        UINT8 AUDC1;
        UINT8 AUDF2;
        UINT8 AUDC2;
        UINT8 AUDF3;
        UINT8 AUDC3;
        UINT8 AUDF4;
        UINT8 AUDC4;
        UINT8 AUDCTL;
        UINT8 STIMER;
        UINT8 SKRES;
        UINT8 POTGO;
        UINT8 SEROUT;
        UINT8 IRQEN;
        UINT8 IRQST;
        UINT8 SKCTLS;
        
        UINT8 KBCODE;
        UINT8 KBCODE_LATCH;
};

#endif
