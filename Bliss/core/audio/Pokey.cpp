
#include "Pokey.h"

void Pokey::resetProcessor()
{
    AUDF1 = 0;
    AUDC1 = 0;
    AUDF2 = 0;
    AUDC2 = 0;
    AUDF3 = 0;
    AUDC3 = 0;
    AUDF4 = 0;
    AUDC4 = 0;
    AUDCTL = 0;
    STIMER = 0;
    SKRES = 0;
    POTGO = 0;
    SEROUT = 0;
    IRQEN = 0;
    IRQST = 0;
    SKCTLS = 0;
    KBCODE = 0;
    KBCODE_LATCH = 0;

    pinOut[POKEY_PIN_OUT_IRQ]->isHigh = TRUE;
}

INT32 Pokey::tick(INT32 minimum)
{
    //check the keypad keys
    if (IRQEN & IRQST & 0x40) {
        UINT8 newCode = leftInput->getKeyPadCode();
        if (KBCODE == 0 && newCode != 0) {
            KBCODE_LATCH = newCode;
            IRQST &= 0xBF;
            pinOut[POKEY_PIN_OUT_IRQ]->isHigh = FALSE;
        }
        KBCODE = newCode;
    }

    audioOutputLine->playSample(0);
    return minimum;
}