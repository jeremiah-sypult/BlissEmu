
#ifndef AY38914_H
#define AY38914_H

#include "AudioProducer.h"
#include "AY38914_Registers.h"
#include "AY38914_Channel.h"
#include "AY38914_InputOutput.h"
#include "AudioOutputLine.h"
#include "core/cpu/Processor.h"
#include "core/types.h"

class Intellivision;

TYPEDEF_STRUCT_PACK( _AY38914State
{
    UINT16 registers[0x0E];
    AY38914_ChannelState channel0;
    AY38914_ChannelState channel1;
    AY38914_ChannelState channel2;
    INT32 clockDivisor;
    INT32 cachedTotalOutput;
    INT32 envelopePeriod;
    INT32 envelopePeriodValue;
    INT32 envelopeCounter;
    INT32 envelopeVolume;
    INT32 noisePeriod;
    INT32 noisePeriodValue;
    INT32 noiseCounter;
    INT32 random;
    INT8  cachedTotalOutputIsDirty;
    INT8  envelopeIdle;
    INT8  envelopeHold;
    INT8  envelopeAltr;
    INT8  envelopeAtak;
    INT8  envelopeCont;
    INT8  noiseIdle;
    INT8  noise;
} AY38914State; )

/**
 * The AY-3-8914 chip in the Intellivision, also known as the Programmable
 * Sound Generator (PSG).
 *
 * @author Kyle Davis
 */
class AY38914 : public Processor, public AudioProducer
{

    friend class AY38914_Registers;

    public:
        AY38914(UINT16 location, AY38914_InputOutput* io0,
                AY38914_InputOutput* io1);
        void resetProcessor();
        INT32 getClockSpeed();
        INT32 getClocksPerSample();
        INT32 getSampleRate() { return getClockSpeed(); }
        INT32 tick(INT32);

        void setClockDivisor(INT32 clockDivisor);
        INT32 getClockDivisor();

        AY38914State getState();
        void setState(AY38914State state);

        //registers
        AY38914_Registers      registers;

    private:
        AY38914_InputOutput*   psgIO0;
        AY38914_InputOutput*   psgIO1;

        //divisor for slowing down the clock speed for the AY38914
        INT32 clockDivisor;

        //channels
        AY38914_Channel        channel0;
        AY38914_Channel        channel1;
        AY38914_Channel        channel2;

        //cached total output sample
        BOOL  cachedTotalOutputIsDirty;
        INT32 cachedTotalOutput;

        //envelope data
        BOOL  envelopeIdle;
        INT32 envelopePeriod;
        INT32 envelopePeriodValue;
        INT32 envelopeCounter;
        INT32 envelopeVolume;
        BOOL  envelopeHold;
        BOOL  envelopeAltr;
        BOOL  envelopeAtak;
        BOOL  envelopeCont;

        //noise data
        BOOL  noiseIdle;
        INT32 noisePeriod;
        INT32 noisePeriodValue;
        INT32 noiseCounter;

        //data for random number generator, used for white noise accuracy
        INT32 random;
        BOOL  noise;

        //output amplitudes for a single channel
        static const INT32 amplitudes16Bit[16];
};

#endif
