
#include "KeyboardInputProducer.h"

KeyboardInputProducer::KeyboardInputProducer(GUID g, IDirectInputDevice8* kd)
: InputProducer(g),
  keyboardDevice(kd)
{
    keyboardDevice->SetDataFormat(&c_dfDIKeyboard); 
    memset(state, 0, sizeof(state));
}

IDirectInputDevice8* KeyboardInputProducer::getDevice()
{
    return keyboardDevice;
}

INT32 KeyboardInputProducer::getInputCount()
{
    return KEYBOARD_OBJECT_COUNT;
}
        
INT32 KeyboardInputProducer::getInput(INT32 i)
{
    return i;
}

const CHAR* KeyboardInputProducer::getInputName(INT32 i)
{
    DIDEVICEOBJECTINSTANCE info;
    info.dwSize = sizeof(info);
    keyboardDevice->GetObjectInfo(&info, i, DIPH_BYOFFSET);
    return info.tszName;
}

INT32 KeyboardInputProducer::evaluateForAnyInput()
{
    int count = getInputCount();
    for (INT32 i = 0; i < count; i++)  {
        INT32 e = getInput(i);
        if (getValue(e) != 0.0f)
            return e;
    }
    return -1;
}

float KeyboardInputProducer::getValue(INT32 enumeration)
{
    return (state[enumeration] ? 1.0f : 0.0f);
}

void KeyboardInputProducer::poll()
{
    keyboardDevice->GetDeviceState(sizeof(state), &state);
}
