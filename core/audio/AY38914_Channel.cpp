
#include "AY38914_Channel.h"

void AY38914_Channel::reset() {
    volume = 0;
    period = 0;
    periodValue = 0x1000;
    toneCounter = periodValue;
    tone = FALSE;
    envelope = FALSE;
    toneDisabled = FALSE;
    noiseDisabled = FALSE;
    isDirty = TRUE;
    cachedSample = 0;
}

AY38914_ChannelState AY38914_Channel::getState()
{
	AY38914_ChannelState state = {0};

	state.period = this->period;
	state.periodValue = this->periodValue;
	state.volume = this->volume;
	state.toneCounter = this->toneCounter;
	state.tone = this->tone;
	state.envelope = this->envelope;
	state.toneDisabled = this->toneDisabled;
	state.noiseDisabled = this->noiseDisabled;

	return state;
}

void AY38914_Channel::setState(AY38914_ChannelState state)
{
	this->period = state.period;
	this->periodValue = state.periodValue;
	this->volume = state.volume;
	this->toneCounter = state.toneCounter;
	this->tone = state.tone;
	this->envelope = state.envelope;
	this->toneDisabled = state.toneDisabled;
	this->noiseDisabled = state.noiseDisabled;

	this->isDirty = TRUE;
}
