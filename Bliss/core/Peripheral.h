
#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include "core/cpu/Processor.h"
#include "core/memory/RAM.h"
#include "core/memory/ROM.h"
#include "core/audio/AudioProducer.h"
#include "core/video/VideoProducer.h"
#include "core/input/InputConsumer.h"

#define MAX_COMPONENTS 16

/**
 * A peripheral represents a device that contains emulated components, including
 * processors, memory units, video producers, audio producers, and input consumers.
 *
 * Each peripheral must to describe the components it contains and the system
 * ROMs it needs.
 */
class Peripheral
{
    public:
        const char* GetName() { return peripheralName; }
        const char* GetShortName() { return peripheralShortName; }

        void AddProcessor(Processor*);
        UINT16 GetProcessorCount();
        Processor* GetProcessor(UINT16 i);

        void AddRAM(RAM*);
        UINT16 GetRAMCount();
        RAM* GetRAM(UINT16 i);

        void AddROM(ROM*);
		UINT16 GetROMCount();
		ROM* GetROM(UINT16 i);

        void AddVideoProducer(VideoProducer*);
        UINT16 GetVideoProducerCount();
        VideoProducer* GetVideoProducer(UINT16 i);

        void AddAudioProducer(AudioProducer*);
        UINT16 GetAudioProducerCount();
        AudioProducer* GetAudioProducer(UINT16 i);

        void AddInputConsumer(InputConsumer*);
        UINT16 GetInputConsumerCount();
        InputConsumer* GetInputConsumer(UINT16 i);

    protected:
        Peripheral(const CHAR* name, const CHAR* shortName)
            : processorCount(0),
              videoProducerCount(0),
              audioProducerCount(0),
              inputConsumerCount(0),
              ramCount(0),
			  romCount(0),
              peripheralName(NULL),
              peripheralShortName(NULL)
            {
                if (name) {
                    peripheralName = new CHAR[strlen(name)+1];
                    strcpy(peripheralName, name);
                }
                if (shortName) {
                    peripheralShortName = new CHAR[strlen(shortName)+1];
                    strcpy(peripheralShortName, shortName);
                }
            }
        ~Peripheral()
        {
            delete[] peripheralShortName;
            delete[] peripheralName;
        }

        //rip exposes peripheral name as mutable, so we leave it protected here
        char*             peripheralName;

    private:
        char*             peripheralShortName;
        Processor*        processors[MAX_COMPONENTS];
        UINT16            processorCount;
        VideoProducer*    videoProducers[MAX_COMPONENTS];
        UINT16            videoProducerCount;
        AudioProducer*    audioProducers[MAX_COMPONENTS];
        UINT16            audioProducerCount;
        InputConsumer*    inputConsumers[MAX_COMPONENTS];
        UINT16            inputConsumerCount;
        RAM*			  rams[MAX_COMPONENTS];
        UINT16            ramCount;
        ROM*		      roms[MAX_COMPONENTS];
        UINT16            romCount;

};

#endif

