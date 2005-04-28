
#ifndef KEYBOARDINPUTPRODUCER_H
#define KEYBOARDINPUTPRODUCER_H

#define KEYBOARD_OBJECT_COUNT 256

#include <windows.h>
#include <dinput.h>
#include "InputProducer.h"

class KeyboardInputProducer : public InputProducer
{

public:
    KeyboardInputProducer(GUID g, IDirectInputDevice8* keyboardDevice);

    const CHAR* getName() { return "Keyboard"; }

    INT32 evaluateForAnyInput();

    void poll();

    const CHAR* getInputName(INT32);
    
    float getValue(INT32 enumeration);

    IDirectInputDevice8* getDevice();

    INT32 getInputCount();
    INT32 getInput(int i);
    
private:
    IDirectInputDevice8* keyboardDevice;
	CHAR state[KEYBOARD_OBJECT_COUNT];

};


#endif