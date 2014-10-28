
#include "AY38914.h"
#include "AudioMixer.h"

const INT32 AY38914::amplitudes16Bit[16] = {
    0x003C, 0x0055, 0x0079, 0x00AB, 0x00F1, 0x0155, 0x01E3, 0x02AA,
    0x03C5, 0x0555, 0x078B, 0x0AAB, 0x0F16, 0x1555, 0x1E2B, 0x2AAA
};

/**
 * The AY-3-8914 chip in the Intellivision, also know as the Programmable
 * Sound Generator (PSG).
 *
 * @author Kyle Davis
 */
AY38914::AY38914(UINT16 location, AY38914_InputOutput* io0,
        AY38914_InputOutput* io1)
    : Processor("AY-3-8914"),
      registers(location)
{
    this->psgIO0 = io0;
    this->psgIO1 = io1;
    clockDivisor = 1;
    registers.init(this);
}

INT32 AY38914::getClockSpeed() {
    return 3579545;
}

INT32 AY38914::getClocksPerSample() {
    return clockDivisor<<4;
}

void AY38914::resetProcessor()
{
    //reset the state variables
    noisePeriod = 0;
    noisePeriodValue = 0x0040;
    noiseCounter = noisePeriodValue;
    random = 1;
    noise = TRUE;
    noiseIdle = TRUE;
    channel0.reset();
    channel1.reset();
    channel2.reset();
    envelopeIdle = TRUE;
    envelopePeriod = 0;
    envelopePeriodValue = 0x20000;
    envelopeCounter = envelopePeriodValue;
    envelopeVolume = 0;
    envelopeHold = FALSE;
    envelopeAltr = FALSE;
    envelopeAtak = FALSE;
    envelopeCont = FALSE;
    cachedTotalOutput = 0;
    cachedTotalOutputIsDirty = TRUE;
}

void AY38914::setClockDivisor(INT32 clockDivisor) {
    this->clockDivisor = clockDivisor;
}

INT32 AY38914::getClockDivisor() {
    return clockDivisor;
}

/**
 * Tick the AY38914.  This method has been profiled over and over again
 * in an attempt to tweak it for optimal performance.  Please be careful
 * with any modifications that may adversely affect performance.
 *
 * @return the number of ticks used by the AY38914, always returns 4.
 */
INT32 AY38914::tick(INT32 minimum)
{
	INT32 totalTicks = 0;
	do {
    //iterate the envelope generator
    envelopeCounter -= clockDivisor;
    if (envelopeCounter <= 0) {
        do {
            envelopeCounter += envelopePeriodValue;
            if (!envelopeIdle) {
                envelopeVolume += (envelopeAtak ? 1 : -1);
                if (envelopeVolume > 15 || envelopeVolume < 0) {
                    if (!envelopeCont) {
                        envelopeVolume = 0;
                        envelopeIdle = TRUE;
                    }
                    else if (envelopeHold) {
                        envelopeVolume = (envelopeAtak == envelopeAltr ? 0 : 15);
                        envelopeIdle = TRUE;
                    }
                    else {
                        envelopeAtak = (envelopeAtak != envelopeAltr);
                        envelopeVolume = (envelopeAtak ? 0 : 15);
                    }
                }
            }
        }
        while (envelopeCounter <= 0);

        //the envelope volume has changed so the channel outputs
        //need to be updated if they are using the envelope
        channel0.isDirty = channel0.envelope;
        channel1.isDirty = channel1.envelope;
        channel2.isDirty = channel2.envelope;
    }

    //iterate the noise generator
    noiseCounter -= clockDivisor;
    if (noiseCounter <= 0) {
        BOOL oldNoise = noise;
        do {
            noiseCounter += noisePeriodValue;
            if (!noiseIdle) {
                random = (random >> 1) ^ (noise ? 0x14000 : 0);
                noise = (random & 1);
            }
        }
        while (noiseCounter <= 0);

        //if the noise bit changed, then our channel outputs need
        //to be updated if they are using the noise generator
        if (!noiseIdle && oldNoise != noise) {
            channel0.isDirty = (channel0.isDirty | !channel0.noiseDisabled);
            channel1.isDirty = (channel1.isDirty | !channel1.noiseDisabled);
            channel2.isDirty = (channel2.isDirty | !channel2.noiseDisabled);
        }
    }

    //iterate the tone generator for channel 0
    channel0.toneCounter -= clockDivisor;
    if (channel0.toneCounter <= 0) {
        do {
            channel0.toneCounter += channel0.periodValue;
            channel0.tone = !channel0.tone;
        }
        while (channel0.toneCounter <= 0);

        channel0.isDirty = !channel0.toneDisabled;
    }

    //iterate the tone generator for channel 1
    channel1.toneCounter -= clockDivisor;
    if (channel1.toneCounter <= 0) {
        do {
            channel1.toneCounter += channel1.periodValue;
            channel1.tone = !channel1.tone;
        }
        while (channel1.toneCounter <= 0);

        channel1.isDirty = !channel1.toneDisabled;
    }

    //iterate the tone generator for channel 2
    channel2.toneCounter -= clockDivisor;
    if (channel2.toneCounter <= 0) {
        do {
            channel2.toneCounter += channel2.periodValue;
            channel2.tone = !channel2.tone;
        }
        while (channel2.toneCounter <= 0);

        channel2.isDirty = !channel2.toneDisabled;
    }

    if (channel0.isDirty) {
        channel0.isDirty = FALSE;
        channel0.cachedSample = amplitudes16Bit[
                (((channel0.toneDisabled | channel0.tone) &
                (channel0.noiseDisabled | noise))
                    ? (channel0.envelope ? envelopeVolume
                        : channel0.volume) : 0)];
        cachedTotalOutputIsDirty = TRUE;
    }

    if (channel1.isDirty) {
        channel1.isDirty = FALSE;
        channel1.cachedSample = amplitudes16Bit[
                (((channel1.toneDisabled | channel1.tone) &
                (channel1.noiseDisabled | noise))
                    ? (channel1.envelope ? envelopeVolume
                        : channel1.volume) : 0)];
        cachedTotalOutputIsDirty = TRUE;
    }

    if (channel2.isDirty) {
        channel2.isDirty = FALSE;
        channel2.cachedSample = amplitudes16Bit[
                (((channel2.toneDisabled | channel2.tone) &
                (channel2.noiseDisabled | noise))
                    ? (channel2.envelope ? envelopeVolume
                        : channel2.volume) : 0)];
        cachedTotalOutputIsDirty = TRUE;
    }

    //mix all three channel samples together to generate the overall
    //output sample for the entire AY38914
    if (cachedTotalOutputIsDirty) {
        cachedTotalOutputIsDirty = FALSE;
        cachedTotalOutput = (channel0.cachedSample +
                channel1.cachedSample + channel2.cachedSample);

        //apply the saturation clipping to correctly model the
        //cross-channel modulation that occurs on a real Intellivision
        cachedTotalOutput <<= 1;
        if (cachedTotalOutput > 0x6000)
            cachedTotalOutput = 0x6000 + ((cachedTotalOutput - 0x6000)/6);
    }

    audioOutputLine->playSample((INT16)cachedTotalOutput);

	totalTicks += (clockDivisor<<4);

	} while (totalTicks < minimum);

    //every tick here always uses some multiple of 4 CPU cycles
    //or 16 NTSC cycles
	return totalTicks;
}

AY38914State AY38914::getState()
{
	AY38914State state = {0};

	this->registers.getMemory(state.registers, 0, this->registers.getMemoryByteSize());

	state.clockDivisor = this->clockDivisor;

	state.channel0 = this->channel0.getState();
	state.channel1 = this->channel1.getState();
	state.channel2 = this->channel2.getState();

	state.cachedTotalOutputIsDirty = this->cachedTotalOutputIsDirty;
	state.cachedTotalOutput = this->cachedTotalOutput;

	state.envelopeIdle = this->envelopeIdle;
	state.envelopePeriod = this->envelopePeriod;
	state.envelopePeriodValue = this->envelopePeriodValue;
	state.envelopeCounter = this->envelopeCounter;
	state.envelopeVolume = this->envelopeVolume;
	state.envelopeHold = this->envelopeHold;
	state.envelopeAltr = this->envelopeAltr;
	state.envelopeAtak = this->envelopeAtak;
	state.envelopeCont = this->envelopeCont;

	state.noiseIdle = this->noiseIdle;
	state.noisePeriod = this->noisePeriod;
	state.noisePeriodValue = this->noisePeriodValue;
	state.noiseCounter = this->noiseCounter;

	state.random = this->random;
	state.noise = this->noise;

	return state;
}

void AY38914::setState(AY38914State state)
{
	this->registers.setMemory(state.registers, 0, this->registers.getMemoryByteSize());

	this->clockDivisor = state.clockDivisor;

	this->channel0.setState(state.channel0);
	this->channel1.setState(state.channel1);
	this->channel2.setState(state.channel2);

	this->cachedTotalOutputIsDirty = state.cachedTotalOutputIsDirty;
	this->cachedTotalOutput = state.cachedTotalOutput;

	this->envelopeIdle = state.envelopeIdle;
	this->envelopePeriod = state.envelopePeriod;
	this->envelopePeriodValue = state.envelopePeriodValue;
	this->envelopeCounter = state.envelopeCounter;
	this->envelopeVolume = state.envelopeVolume;
	this->envelopeHold = state.envelopeHold;
	this->envelopeAltr = state.envelopeAltr;
	this->envelopeAtak = state.envelopeAtak;
	this->envelopeCont = state.envelopeCont;

	this->noiseIdle = state.noiseIdle;
	this->noisePeriod = state.noisePeriod;
	this->noisePeriodValue = state.noisePeriodValue;
	this->noiseCounter = state.noiseCounter;

	this->random = state.random;
	this->noise = state.noise;
}
