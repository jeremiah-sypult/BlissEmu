
#include "Emulator.h"
#include "drivers/intv/Intellivision.h"
#include "drivers/a5200/Atari5200.h"

#define NUM_EMULATORS 1

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

    AddProcessor(&audioMixer);
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

void Emulator::InitVideo(IDirect3DDevice9* direct3DDevice)
{
    videoBus.init(direct3DDevice);
}

void Emulator::ReleaseVideo()
{
    videoBus.release();
}

void Emulator::InitAudio(IDirectSoundBuffer8* directSoundBuffer)
{
    audioMixer.init(directSoundBuffer);
}

void Emulator::ReleaseAudio()
{
    audioMixer.release();
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
        videoBus.removeAll();
        audioMixer.removeAll();
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
        videoBus.addVideoProducer(p->GetVideoProducer(i));

    //audio producers
    count = p->GetAudioProducerCount();
    for (i = 0; i < count; i++)
        audioMixer.addAudioProducer(p->GetAudioProducer(i));

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
        videoBus.removeVideoProducer(p->GetVideoProducer(i));

    //audio producers
    count = p->GetAudioProducerCount();
    for (i = 0; i < count; i++)
        audioMixer.removeAudioProducer(p->GetAudioProducer(i));

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
    videoBus.render();
}

void Emulator::FlushAudio()
{
    audioMixer.flushAudio();
}

UINT32 systemIDs[NUM_EMULATORS] = {
//        0xE4453A0B,
        0x4AC771F8,
    };

Intellivision inty;
//Atari5200 a5200;

Emulator* emus[] = {
//    &a5200,
    &inty,
};

