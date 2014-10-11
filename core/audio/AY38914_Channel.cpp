
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
