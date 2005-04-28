
#ifndef POKEY_H
#define POKEY_H

#include "AudioProducer.h"
#include "core/cpu/Processor.h"

class Pokey : public Processor, public AudioProducer
{

    public:
        Pokey() : Processor("POKEY") {}
        INT32 getClockSpeed();
        INT32 tick();

    private:
        UINT8 AUDCTL;
        UINT8 AUDFX0;
        UINT8 AUDFX1;
        UINT8 AUDFX2;
        UINT8 AUDFX3;
        UINT8 AUDCX0;
        UINT8 AUDCX1;
        UINT8 AUDCX2;
        UINT8 AUDCX3;

};

#endif
