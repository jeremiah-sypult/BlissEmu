
#include "Emulator.h"
#include "drivers/intv/Intellivision.h"
#include "drivers/a5200/Atari5200.h"

extern UINT32 systemIDs[NUM_EMULATORS];
extern Emulator* emus[NUM_EMULATORS];

UINT32 Emulator::GetEmulatorCount()
{
    return NUM_EMULATORS;
}

Emulator* Emulator::GetEmulator(UINT32 i)
{
    return emus[i];
}

Emulator* Emulator::GetEmulatorByID(UINT32 targetSystemID)
{
    for (int i = 0; i < NUM_EMULATORS; i++) {
        if (systemIDs[i] == targetSystemID)
            return emus[i];
    }

    return NULL;
}

Emulator::Emulator(const char* name)
    : Peripheral(name, name),
      currentRip(NULL),
      peripheralCount(0)
{
    memset(peripherals, 0, sizeof(peripherals));
    memset(usePeripheralIndicators, FALSE, sizeof(usePeripheralIndicators));
}

void Emulator::AddPeripheral(Peripheral* p)
{
    peripherals[peripheralCount] = p;
    peripheralCount++;
}

UINT32 Emulator::GetPeripheralCount()
{
    return peripheralCount;
}

Peripheral* Emulator::GetPeripheral(UINT32 i)
{
    return peripherals[i];
}

void Emulator::UsePeripheral(UINT32 i, BOOL b)
{
    usePeripheralIndicators[i] = b;
}

UINT32 Emulator::GetVideoWidth()
{
    return videoWidth;
}

UINT32 Emulator::GetVideoHeight()
{
    return videoHeight;
}

void Emulator::InitVideo(VideoBus* video, UINT32 width, UINT32 height)
{
    if ( video != NULL ) {
        videoBus = video;
    }

    videoBus->init(width, height);
}

void Emulator::ReleaseVideo()
{
    if (videoBus) {
        videoBus->release();
        videoBus = NULL;
    }
}

void Emulator::InitAudio(AudioMixer* audio, UINT32 sampleRate)
{
    if (audio != NULL) {
        audioMixer = audio;
    }

    // TODO: check for an existing audioMixer processor and release it
    for (UINT16 i = 0; i < GetProcessorCount(); i++) {
        Processor* p = GetProcessor(i);
        if (p == audio) {
            RemoveProcessor(audio);
        }
    }

    AddProcessor(audioMixer);
    audioMixer->init(sampleRate);
}

void Emulator::ReleaseAudio()
{
    if (audioMixer) {
        audioMixer->release();
        RemoveProcessor(audioMixer);
        audioMixer = NULL;
    }
}

void Emulator::Reset()
{
    processorBus.reset();
    memoryBus.reset();
}

void Emulator::SetRip(Rip* rip)
{
    if (this->currentRip != NULL) {
        processorBus.removeAll();
        memoryBus.removeAll();
        videoBus->removeAll();
        audioMixer->removeAll();
        inputConsumerBus.removeAll();
    }

    currentRip = rip;

    //TODO: use the desired peripheral configuration specified by the rip

    if (this->currentRip != NULL) {
        InsertPeripheral(this);

        //use the desired peripherals
        for (INT32 i = 0; i < peripheralCount; i++) {
            if (usePeripheralIndicators[i])
                InsertPeripheral(peripherals[i]);
        }

        InsertPeripheral(currentRip);
    }
}

void Emulator::InsertPeripheral(Peripheral* p)
{
    UINT16 i;

    //processors
    UINT16 count = p->GetProcessorCount();
    for (i = 0; i < count; i++)
        processorBus.addProcessor(p->GetProcessor(i));

    //RAM
    count = p->GetRAMCount();
    for (i = 0; i < count; i++)
        memoryBus.addMemory(p->GetRAM(i));

    //ROM
    count = p->GetROMCount();
    for (i = 0; i < count; i++) {
        ROM* nextRom = p->GetROM(i);
        if (!nextRom->isInternal())
            memoryBus.addMemory(nextRom);
    }

    //video producers
    count = p->GetVideoProducerCount();
    for (i = 0; i < count; i++)
        videoBus->addVideoProducer(p->GetVideoProducer(i));

    //audio producers
    count = p->GetAudioProducerCount();
    for (i = 0; i < count; i++)
        audioMixer->addAudioProducer(p->GetAudioProducer(i));

    //input consumers
    count = p->GetInputConsumerCount();
    for (i = 0; i < count; i++)
        inputConsumerBus.addInputConsumer(p->GetInputConsumer(i));
}

void Emulator::RemovePeripheral(Peripheral* p)
{
    UINT16 i;

    //processors
    UINT16 count = p->GetProcessorCount();
    for (i = 0; i < count; i++)
        processorBus.removeProcessor(p->GetProcessor(i));

    //RAM
    count = p->GetRAMCount();
    for (i = 0; i < count; i++)
        memoryBus.removeMemory(p->GetRAM(i));

    //ROM
    count = p->GetROMCount();
    for (i = 0; i < count; i++)
        memoryBus.removeMemory(p->GetROM(i));

    //video producers
    count = p->GetVideoProducerCount();
    for (i = 0; i < count; i++)
        videoBus->removeVideoProducer(p->GetVideoProducer(i));

    //audio producers
    count = p->GetAudioProducerCount();
    for (i = 0; i < count; i++)
        audioMixer->removeAudioProducer(p->GetAudioProducer(i));

    //input consumers
    count = p->GetInputConsumerCount();
    for (i = 0; i < count; i++)
        inputConsumerBus.removeInputConsumer(p->GetInputConsumer(i));
}

void Emulator::Run()
{
    inputConsumerBus.evaluateInputs();
    processorBus.run();
}

void Emulator::Render()
{
    videoBus->render();
}

void Emulator::FlushAudio()
{
    audioMixer->flushAudio();
}

UINT32 Emulator::systemIDs[NUM_EMULATORS] = {
        ID_SYSTEM_ATARI5200,
        ID_SYSTEM_INTELLIVISION,
    };

Intellivision Emulator::inty;
Atari5200 Emulator::atari5200;

Emulator* Emulator::emus[] = {
    &atari5200,
    &inty,
};
