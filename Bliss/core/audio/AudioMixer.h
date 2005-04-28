
#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <Windows.h>
#include <d3d9.h>
#include <mmreg.h>
#include <dsound.h>

#include "AudioProducer.h"
#include "core/types.h"
#include "core/cpu/Processor.h"

#define MAX_AUDIO_PRODUCERS 10

template<typename A, typename W> class EmulatorTmpl;

class AudioMixer : public Processor
{

    friend class ProcessorBus;
    friend class AudioOutputLine;

    public:
        AudioMixer();
        virtual ~AudioMixer();

        void resetProcessor();
        INT32 getClockSpeed();
        INT32 tick(INT32 minimum);
        void flushAudio();

        //only to be called by the Emulator
        void init(IDirectSoundBuffer8* aod);
        void release();

        void addAudioProducer(AudioProducer*);
        void removeAudioProducer(AudioProducer*);
        void removeAll();

    private:
        //output info
        IDirectSoundBuffer8* outputBuffer;
        UINT32 outputBufferWritePosition;
        UINT32 outputBufferSize;

        AudioProducer*     audioProducers[MAX_AUDIO_PRODUCERS];
        UINT32             audioProducerCount;

        INT64 commonClocksPerTick;
        UINT8* sampleBuffer;
		UINT32 sampleBufferLength;
        UINT32 sampleCount;

};

#endif
