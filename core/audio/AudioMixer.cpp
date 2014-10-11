
#include "gui/stdafx.h"
#include "gui/BlissApp.h"

#include <stdio.h>
#include <string.h>
#include "AudioMixer.h"
#include "AudioOutputLine.h"

extern UINT64 lcm(UINT64, UINT64);

AudioMixer::AudioMixer()
    : Processor("Audio Mixer"),
      outputBuffer(NULL),
      audioProducerCount(0),
      commonClocksPerTick(0),
      sampleBuffer(NULL),
	  sampleBufferLength(0),
      sampleCount(0),
      outputBufferWritePosition(0),
      outputBufferSize(0)
{
	memset(&audioProducers, 0, sizeof(audioProducers));
}

AudioMixer::~AudioMixer()
{
    if (sampleBuffer)
        delete[] sampleBuffer;
    for (UINT32 i = 0; i < audioProducerCount; i++)
        delete audioProducers[i]->audioOutputLine;
}

void AudioMixer::addAudioProducer(AudioProducer* p)
{
    audioProducers[audioProducerCount] = p;
    audioProducers[audioProducerCount]->audioOutputLine =
            new AudioOutputLine();
    audioProducerCount++;
}

void AudioMixer::removeAudioProducer(AudioProducer* p)
{
    for (UINT32 i = 0; i < audioProducerCount; i++) {
        if (audioProducers[i] == p) {
            delete p->audioOutputLine;
            for (UINT32 j = i; j < (audioProducerCount-1); j++)
                audioProducers[j] = audioProducers[j+1];
            audioProducerCount--;
            return;
        }
    }
}

void AudioMixer::removeAll()
{
    while (audioProducerCount)
        removeAudioProducer(audioProducers[0]);
}

void AudioMixer::resetProcessor()
{
    //reset instance data
    outputBufferWritePosition = 0;
    commonClocksPerTick = 0;
    sampleCount = 0;
    if (sampleBuffer)
        memset(sampleBuffer, 0, sampleBufferLength);

    //iterate through my audio output lines to determine the common output clock
    UINT64 totalClockSpeed = getClockSpeed();
    for (UINT32 i = 0; i < audioProducerCount; i++) {
        audioProducers[i]->audioOutputLine->reset();
        totalClockSpeed = lcm(totalClockSpeed, ((UINT64)audioProducers[i]->getClockSpeed()));
    }

    //iterate again to determine the clock factor of each
    commonClocksPerTick = totalClockSpeed / getClockSpeed();
    for (UINT32 i = 0; i < audioProducerCount; i++) {
        audioProducers[i]->audioOutputLine->commonClocksPerSample = (totalClockSpeed / audioProducers[i]->getClockSpeed())
				* audioProducers[i]->getClocksPerSample();
    }
}

void AudioMixer::init(IDirectSoundBuffer8* buffer)
{
    this->outputBuffer = buffer;
    DSBCAPS caps;
	ZeroMemory(&caps, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	buffer->GetCaps(&caps);
    outputBufferSize = caps.dwBufferBytes;
    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
    buffer->GetFormat(&wfx, sizeof(WAVEFORMATEX), NULL);
    sampleBufferLength = wfx.nAvgBytesPerSec/10;
    sampleBuffer = new UINT8[sampleBufferLength];
	memset(sampleBuffer, 0, sampleBufferLength);
    sampleCount = 0;
}

void AudioMixer::release()
{
    if (outputBuffer) {
        this->outputBuffer = NULL;
    }
    if (sampleBuffer) {
        delete[] sampleBuffer;
        sampleBuffer = NULL;
        sampleBufferLength = 0;
        sampleCount = 0;
    }
}

INT32 AudioMixer::getClockSpeed()
{
	DWORD freq;
	outputBuffer->GetFrequency(&freq);
	return freq;
}

INT32 AudioMixer::tick(INT32 minimum)
{
    for (int totalTicks = 0; totalTicks < minimum; totalTicks++) {
        //mix and flush the sample buffers
        INT64 totalSample = 0;
        for (UINT32 i = 0; i < audioProducerCount; i++) {
            AudioOutputLine* nextLine = audioProducers[i]->audioOutputLine;

            INT64 missingClocks = (this->commonClocksPerTick - nextLine->commonClockCounter);
            INT64 sampleToUse = (missingClocks < 0 ? nextLine->previousSample : nextLine->currentSample);

            //account for when audio producers idle by adding enough samples to each producer's buffer
			//to fill the time since last sample calculation
            INT64 missingSampleCount = (missingClocks / nextLine->commonClocksPerSample);
            if (missingSampleCount != 0) {
			    nextLine->sampleBuffer += missingSampleCount * sampleToUse * nextLine->commonClocksPerSample;
                nextLine->commonClockCounter += missingSampleCount * nextLine->commonClocksPerSample;
                missingClocks -= missingSampleCount * nextLine->commonClocksPerSample;
            }
			INT64 partialSample = sampleToUse * missingClocks;

            //calculate the sample for this line
            totalSample += (INT16)((nextLine->sampleBuffer + partialSample) / commonClocksPerTick);

            //clear the sample buffer for this line
            nextLine->sampleBuffer = -partialSample;
            nextLine->commonClockCounter = -missingClocks;
        }

        if (audioProducerCount > 0)
            totalSample = totalSample / audioProducerCount;
        sampleBuffer[sampleCount++] = (UINT8)(totalSample & 0xFF);
        sampleBuffer[sampleCount++] = (UINT8)((totalSample & 0xFF00)>>8);
    	
        if (sampleCount == sampleBufferLength)
            flushAudio();
    }

    return minimum;
}

void AudioMixer::flushAudio()
{
    DWORD currentPos;
    UINT32 end = (outputBufferWritePosition + sampleCount) % outputBufferSize;
    if (end < outputBufferWritePosition) {
        do {
            outputBuffer->GetCurrentPosition(&currentPos, NULL);
        } while (currentPos >= outputBufferWritePosition || currentPos < end);
    }
    else {
        do {
            outputBuffer->GetCurrentPosition(&currentPos, NULL);
        } while (currentPos >= outputBufferWritePosition && currentPos < end);
    }
    DWORD bufSize1, bufSize2;
    void* buf1; void* buf2;
    outputBuffer->Lock(outputBufferWritePosition, sampleCount, &buf1, &bufSize1, &buf2, &bufSize2, 0);
    memcpy(buf1, sampleBuffer, bufSize1);
    memcpy(buf2, sampleBuffer+bufSize1, bufSize2);
    outputBuffer->Unlock(buf1, bufSize1, buf2, bufSize2);
    outputBufferWritePosition = end;

    sampleCount = 0;
}
