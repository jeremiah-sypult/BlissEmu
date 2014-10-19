
#ifndef JOYSTICKINPUTPRODUCER_H
#define JOYSTICKINPUTPRODUCER_H

#define JOYSTICK_OBJECT_COUNT 256

// TODO: jeremiah sypult cross-platform
#if defined( _WIN32 )
#include <windows.h>
#include <dinput.h>
#endif

#include "InputProducer.h"

class JoystickInputProducer : public InputProducer
{

public:
#if defined( _WIN32 )
    JoystickInputProducer(GUID g, IDirectInputDevice8* joystickDevice);
#endif
    const CHAR* getName() { return "Joystick"; }

    INT32 evaluateForAnyInput();

    void poll();

    const CHAR* getInputName(INT32);
    
    float getValue(INT32 enumeration);
#if defined( _WIN32 )
    IDirectInputDevice8* getDevice();
#endif
    INT32 getInputCount();
    INT32 getInput(int i);
    
    virtual BOOL isKeyboardDevice() { return FALSE; }

private:
#if defined( _WIN32 )
    IDirectInputDevice8* joystickDevice;
    DIJOYSTATE2 state;
#endif
};


#endif