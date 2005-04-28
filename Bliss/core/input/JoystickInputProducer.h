
#ifndef JOYSTICKINPUTPRODUCER_H
#define JOYSTICKINPUTPRODUCER_H

#define JOYSTICK_OBJECT_COUNT 256

#include <windows.h>
#include <dinput.h>
#include "InputProducer.h"

class JoystickInputProducer : public InputProducer
{

public:
    JoystickInputProducer(GUID g, IDirectInputDevice8* joystickDevice);

    const CHAR* getName() { return "Joystick"; }

    INT32 evaluateForAnyInput();

    void poll();

    const CHAR* getInputName(INT32);
    
    float getValue(INT32 enumeration);

    IDirectInputDevice8* getDevice();

    INT32 getInputCount();
    INT32 getInput(int i);
    
private:
    IDirectInputDevice8* joystickDevice;
    DIJOYSTATE2 state;

};


#endif