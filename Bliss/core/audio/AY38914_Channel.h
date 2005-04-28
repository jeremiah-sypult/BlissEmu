
#ifndef AY38914__CHANNEL_H
#define AY38914__CHANNEL_H

#include "core/types.h"

class AY38914;
class AY38914_Registers;

class AY38914_Channel
{

    friend class AY38914;
    friend class AY38914_Registers;

    private:
        void reset();

        INT32   period;
        INT32   periodValue;
        INT32   volume;
        INT32   toneCounter;
        BOOL    tone;
        BOOL    envelope;
        BOOL    toneDisabled;
        BOOL    noiseDisabled;
        BOOL    isDirty;
        INT32   cachedSample;

};

#endif

