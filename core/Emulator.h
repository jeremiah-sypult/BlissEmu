
#ifndef EMULATOR_H
#define EMULATOR_H

#include "Peripheral.h"
#include "core/types.h"
#include "core/rip/Rip.h"
#include "core/cpu/ProcessorBus.h"
#include "core/cpu/Processor.h"
#include "core/audio/AudioMixer.h"
#include "core/audio/AudioProducer.h"
#include "core/input/InputConsumerBus.h"
#include "core/input/InputConsumer.h"
#include "core/video/VideoBus.h"
#include "core/video/VideoProducer.h"
#include "core/memory/MemoryBus.h"
#include "core/memory/Memory.h"

class Intellivision;
class Atari5200;

#define MAX_PERIPHERALS    16
#define NUM_EMULATORS       2

/**
 *
 */
class Emulator : public Peripheral
{
    public:
        void AddPeripheral(Peripheral* p);
        UINT32 GetPeripheralCount();
        Peripheral* GetPeripheral(UINT32);

		UINT32 GetVideoWidth();
		UINT32 GetVideoHeight();

        void UsePeripheral(UINT32, BOOL);

        void SetRip(Rip* rip);

		void InitVideo(VideoBus* video, UINT32 width, UINT32 height);
		void ReleaseVideo();
		void InitAudio(AudioMixer* audio, UINT32 sampleRate);
		void ReleaseAudio();

        void Reset();
        void Run();
        void FlushAudio();
		void Render();

		static UINT32 GetEmulatorCount();
        static Emulator* GetEmulator(UINT32 i);
		static Emulator* GetEmulatorByID(UINT32 targetSystemID);
        
    protected:
        Emulator(const char* name);

        MemoryBus          memoryBus;

		UINT32 videoWidth;
		UINT32 videoHeight;

    private:
        ProcessorBus       processorBus;
        AudioMixer         *audioMixer;
        VideoBus           *videoBus;
        InputConsumerBus   inputConsumerBus;

        void InsertPeripheral(Peripheral* p);
        void RemovePeripheral(Peripheral* p);

        Rip*     currentRip;

        Peripheral*     peripherals[MAX_PERIPHERALS];
        BOOL            usePeripheralIndicators[MAX_PERIPHERALS];
        INT32           peripheralCount;

        static UINT32 systemIDs[NUM_EMULATORS];
        static Emulator* emus[NUM_EMULATORS];
        static Intellivision inty;
        static Atari5200     atari5200;

};

#endif

