
#ifndef KEYBOARDINPUTPRODUCER_H
#define KEYBOARDINPUTPRODUCER_H

#define KEYBOARD_OBJECT_COUNT 256

// TODO: jeremiah sypult cross-platform
#if defined( _WIN32 )
#include <windows.h>
#include <dinput.h>
#endif

#include "InputProducer.h"

class KeyboardInputProducer : public InputProducer
{

public:
#if defined( _WIN32 )
    KeyboardInputProducer(GUID g, IDirectInputDevice8* keyboardDevice);
#else
	KeyboardInputProducer(GUID g);
#endif
    const CHAR* getName() { return "Keyboard"; }

    INT32 evaluateForAnyInput();

    void poll();

    const CHAR* getInputName(INT32);
    
    float getValue(INT32 enumeration);
#if defined( _WIN32 )
    IDirectInputDevice8* getDevice();
#endif
    INT32 getInputCount();
    INT32 getInput(int i);
    
    virtual BOOL isKeyboardDevice() { return TRUE; }

private:
#if defined( _WIN32 )
    IDirectInputDevice8* keyboardDevice;
#endif
	CHAR state[KEYBOARD_OBJECT_COUNT];

};


#endif