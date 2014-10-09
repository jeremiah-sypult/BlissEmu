
#ifndef JOYPAD_H
#define JOYPAD_H

#include "core/input/InputConsumer.h"
#include "core/audio/Pokey_Input.h"

class JoyPad : public InputConsumer, public Pokey_Input
{

public:
    JoyPad(INT32 id, const CHAR* name);
    virtual ~JoyPad();

    const CHAR* getName();
	
	void resetInputConsumer();
	
	void evaluateInputs();
	
    INT32 getInputConsumerObjectCount() { return 21; }
    InputConsumerObject* getInputConsumerObject(INT32 i) { return inputObjects[i]; }
	
    UINT8 getKeyPadCode() { return kbcode; }
    UINT8 getPotX() { return potx; }
    UINT8 getPotY() { return poty; }

private:
    InputConsumerObject* inputObjects[21];
    
    CHAR* name;
    UINT8 kbcode;
    UINT8 potx;
    UINT8 poty;

    const static UINT8 KBCODES[15];
        
};

#endif
