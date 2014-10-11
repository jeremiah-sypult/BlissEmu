

#include "Peripheral.h"

void Peripheral::AddProcessor(Processor* p)
{
    processors[processorCount] = p;
    processorCount++;
}

UINT16 Peripheral::GetProcessorCount()
{
    return processorCount;
}

Processor* Peripheral::GetProcessor(UINT16 i)
{
    return processors[i];
}

void Peripheral::AddRAM(RAM* m)
{
    rams[ramCount] = m;
    ramCount++;
}

UINT16 Peripheral::GetRAMCount()
{
    return ramCount;
}

RAM* Peripheral::GetRAM(UINT16 i)
{
    return rams[i];
}

void Peripheral::AddVideoProducer(VideoProducer* vp)
{
    videoProducers[videoProducerCount] = vp;
    videoProducerCount++;
}

UINT16 Peripheral::GetVideoProducerCount()
{
    return videoProducerCount;
}

VideoProducer* Peripheral::GetVideoProducer(UINT16 i)
{
    return videoProducers[i];
}

void Peripheral::AddAudioProducer(AudioProducer* ap)
{
    audioProducers[audioProducerCount] = ap;
    audioProducerCount++;
}

UINT16 Peripheral::GetAudioProducerCount()
{
    return audioProducerCount;
}

AudioProducer* Peripheral::GetAudioProducer(UINT16 i)
{
    return audioProducers[i];
}

void Peripheral::AddInputConsumer(InputConsumer* ic)
{
    inputConsumers[inputConsumerCount] = ic;
    inputConsumerCount++;
}

UINT16 Peripheral::GetInputConsumerCount()
{
    return inputConsumerCount;
}

InputConsumer* Peripheral::GetInputConsumer(UINT16 i)
{
    return inputConsumers[i];
}

void Peripheral::AddROM(ROM* r)
{
    roms[romCount] = r;
    romCount++;
}

UINT16 Peripheral::GetROMCount()
{
    return romCount;
}

ROM* Peripheral::GetROM(UINT16 i)
{
    return roms[i];
}


