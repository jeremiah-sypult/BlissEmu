
#include "JoystickInputProducer.h"

typedef enum
{
    NegX,   PosX,   NegY,   PosY,   NegZ,   PosZ,
    NegRx,  PosRx,  NegRy,  PosRy,  NegRz,  PosRz,
    NegAX,  PosAX,  NegAY,  PosAY,  NegAZ,  PosAZ,
    NegARx, PosARx, NegARy, PosARy, NegARz, PosARz,
    NegFX,  PosFX,  NegFY,  PosFY,  NegFZ,  PosFZ,
    NegFRx, PosFRx, NegFRy, PosFRy, NegFRz, PosFRz,
    NegVX,  PosVX,  NegVY,  PosVY,  NegVZ,  PosVZ,
    NegVRx, PosVRx, NegVRy, PosVRy, NegVRz, PosVRz,
    B0,  B1,  B2,
    B3,  B4,  B5,
    B6,  B7,  B8,
    B9,  B10, B11,
    B12, B13, B14,
    B15
} JoystickAxis;

const CHAR* enumNames[] = {
        "Negative-X", "Positive-X", "Negative-Y", "Positive-Y", "Negative-Z", "Positive-Z",
        "Negative-RX", "Positive-RX", "Negative-RY", "Positive-RY", "Negative-RZ", "Positive-RZ",
        "Negative-AX", "Positive-AX", "Negative-AY", "Positive-AY", "Negative-AZ", "Positive-AZ",
        "Negative-ARX", "Positive-ARX", "Negative-ARY", "Positive-ARY", "Negative-ARZ", "Positive-ARZ",
        "Negative-FX", "Positive-FX", "Negative-FY", "Positive-FY", "Negative-FZ", "Positive-FZ",
        "Negative-FRX", "Positive-FRX", "Negative-FRY", "Positive-FRY", "Negative-FRZ", "Positive-FRZ",
        "Negative-VX", "Positive-VX", "Negative-VY", "Positive-VY", "Negative-VZ", "Positive-VZ",
        "Negative-VRX", "Positive-VRX", "Negative-VRY", "Positive-VRY", "Negative-VRZ", "Positive-VRZ",
        "Button 0", "Button 1", "Button 2", "Button 3", "Button 4", "Button 5",
        "Button 6", "Button 7", "Button 8", "Button 9", "Button 10", "Button 11",
        "Button 12", "Button 13", "Button 14", "Button 15"
};

//BOOL CALLBACK EnumObjects(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
#if defined( _WIN32 )
JoystickInputProducer::JoystickInputProducer(GUID g, IDirectInputDevice8* jd)
: InputProducer(g),
  joystickDevice(jd)
{
    joystickDevice->SetDataFormat(&c_dfDIJoystick2);
    //set the range of all the axes
    //joystickDevice->EnumObjects(EnumObjects, joystickDevice, DIDFT_AXIS);
    DIPROPRANGE propRange = {
        {
            sizeof(DIPROPRANGE),
            sizeof(DIPROPHEADER),
            0,
            DIPH_DEVICE,
        },
        -5000,
        5000
    };
    HRESULT res;
    if ((res = joystickDevice->SetProperty(DIPROP_RANGE, &propRange.diph)) != DI_OK)
        //???
    {}

    DIPROPDWORD propValue = {
        {
            sizeof(DIPROPDWORD),
            sizeof(DIPROPHEADER),
            0,
            DIPH_DEVICE,
        },
        1500
    };
    if ((res = joystickDevice->SetProperty(DIPROP_DEADZONE, &propValue.diph)) != DI_OK)
        //???
    {}

    propValue.dwData = 9800;
    if ((res = joystickDevice->SetProperty(DIPROP_SATURATION, &propValue.diph)) != DI_OK)
         //???
    {}
}

IDirectInputDevice8* JoystickInputProducer::getDevice()
{
    return joystickDevice;
}
#endif
INT32 JoystickInputProducer::getInputCount()
{
    return JOYSTICK_OBJECT_COUNT;
}
        
INT32 JoystickInputProducer::getInput(INT32 i)
{
    return i;
}

const CHAR* JoystickInputProducer::getInputName(INT32 i)
{
    return enumNames[i];
}

INT32 JoystickInputProducer::evaluateForAnyInput()
{
    int count = getInputCount();
    for (INT32 i = 0; i < count; i++)  {
        INT32 e = getInput(i);
        if (getValue(e) != 0.0f)
            return e;
    }
    return -1;
}

float JoystickInputProducer::getValue(INT32 enumeration)
{
#if defined( _WIN32 )
    switch ((JoystickAxis)enumeration) 
    {
        case NegX:
            return state.lX < 0 ? ((float)-state.lX)/5000.0f : 0.0f;
        case PosX:
            return state.lX > 0 ? ((float)state.lX)/5000.0f : 0.0f;
        case NegY:
            return state.lY < 0 ? ((float)-state.lY)/5000.0f : 0.0f;
        case PosY:
            return state.lY > 0 ? ((float)state.lY)/5000.0f : 0.0f;
        case NegZ:
            return state.lZ < 0 ? ((float)-state.lZ)/5000.0f : 0.0f;
        case PosZ:
            return state.lZ > 0 ? ((float)state.lZ)/5000.0f : 0.0f;
        case NegRx:
            return state.lRx < 0 ? ((float)-state.lRx)/5000.0f : 0.0f;
        case PosRx:
            return state.lRx > 0 ? ((float)state.lRx)/5000.0f : 0.0f;
        case NegRy:
            return state.lRy < 0 ? ((float)-state.lRy)/5000.0f : 0.0f;
        case PosRy:
            return state.lRy > 0 ? ((float)state.lRy)/5000.0f : 0.0f;
        case NegRz:
            return state.lRz < 0 ? ((float)-state.lRz)/5000.0f : 0.0f;
        case PosRz:
            return state.lRz > 0 ? ((float)state.lRz)/5000.0f : 0.0f;
        case NegAX:
            return state.lAX < 0 ? ((float)-state.lAX)/5000.0f : 0.0f;
        case PosAX:
            return state.lAX > 0 ? ((float)state.lAX)/5000.0f : 0.0f;
        case NegAY:
            return state.lAY < 0 ? ((float)-state.lAY)/5000.0f : 0.0f;
        case PosAY:
            return state.lAY > 0 ? ((float)state.lAY)/5000.0f : 0.0f;
        case NegAZ:
            return state.lAZ < 0 ? ((float)-state.lAZ)/5000.0f : 0.0f;
        case PosAZ:
            return state.lAZ > 0 ? ((float)state.lAZ)/5000.0f : 0.0f;
        case NegARx:
            return state.lARx < 0 ? ((float)-state.lARx)/5000.0f : 0.0f;
        case PosARx:
            return state.lARx > 0 ? ((float)state.lARx)/5000.0f : 0.0f;
        case NegARy:
            return state.lARy < 0 ? ((float)-state.lARy)/5000.0f : 0.0f;
        case PosARy:
            return state.lARy > 0 ? ((float)state.lARy)/5000.0f : 0.0f;
        case NegARz:
            return state.lARz < 0 ? ((float)-state.lARz)/5000.0f : 0.0f;
        case PosARz:
            return state.lARz > 0 ? ((float)state.lARz)/5000.0f : 0.0f;
        case NegFX:
            return state.lFX < 0 ? ((float)-state.lFX)/5000.0f : 0.0f;
        case PosFX:
            return state.lFX > 0 ? ((float)state.lFX)/5000.0f : 0.0f;
        case NegFY:
            return state.lFY < 0 ? ((float)-state.lFY)/5000.0f : 0.0f;
        case PosFY:
            return state.lFY > 0 ? ((float)state.lFY)/5000.0f : 0.0f;
        case NegFZ:
            return state.lFZ < 0 ? ((float)-state.lFZ)/5000.0f : 0.0f;
        case PosFZ:
            return state.lFZ > 0 ? ((float)state.lFZ)/5000.0f : 0.0f;
        case NegFRx:
            return state.lFRx < 0 ? ((float)-state.lFRx)/5000.0f : 0.0f;
        case PosFRx:
            return state.lFRx > 0 ? ((float)state.lFRx)/5000.0f : 0.0f;
        case NegFRy:
            return state.lFRy < 0 ? ((float)-state.lFRy)/5000.0f : 0.0f;
        case PosFRy:
            return state.lFRy > 0 ? ((float)state.lFRy)/5000.0f : 0.0f;
        case NegFRz:
            return state.lFRz < 0 ? ((float)-state.lFRz)/5000.0f : 0.0f;
        case PosFRz:
            return state.lFRz > 0 ? ((float)state.lFRz)/5000.0f : 0.0f;
        case NegVX:
            return state.lVX < 0 ? ((float)-state.lVX)/5000.0f : 0.0f;
        case PosVX:
            return state.lVX > 0 ? ((float)state.lVX)/5000.0f : 0.0f;
        case NegVY:
            return state.lVY < 0 ? ((float)-state.lVY)/5000.0f : 0.0f;
        case PosVY:
            return state.lVY > 0 ? ((float)state.lVY)/5000.0f : 0.0f;
        case NegVZ:
            return state.lVZ < 0 ? ((float)-state.lVZ)/5000.0f : 0.0f;
        case PosVZ:
            return state.lVZ > 0 ? ((float)state.lVZ)/5000.0f : 0.0f;
        case NegVRx:
            return state.lVRx < 0 ? ((float)-state.lVRx)/5000.0f : 0.0f;
        case PosVRx:
            return state.lVRx > 0 ? ((float)state.lVRx)/5000.0f : 0.0f;
        case NegVRy:
            return state.lVRy < 0 ? ((float)-state.lVRy)/5000.0f : 0.0f;
        case PosVRy:
            return state.lVRy > 0 ? ((float)state.lVRy)/5000.0f : 0.0f;
        case NegVRz:
            return state.lVRz < 0 ? ((float)-state.lVRz)/5000.0f : 0.0f;
        case PosVRz:
            return state.lVRz > 0 ? ((float)state.lVRz)/5000.0f : 0.0f;
        case B0:
        case B1:
        case B2:
        case B3:
        case B4:
        case B5:
        case B6:
        case B7:
        case B8:
        case B9:
        case B10:
        case B11:
        case B12:
        case B13:
        case B14:
        case B15:
            return (state.rgbButtons[((JoystickAxis)enumeration) - B0] != 0 ? 1.0f : 0.0f);
    }
#endif
    return 0.0f;
}

void JoystickInputProducer::poll()
{
#if defined( _WIN32 )
    joystickDevice->GetDeviceState(sizeof(state), &state);
#endif
}

/*
BOOL CALLBACK EnumObjects(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    IDirectInputDevice8* device = (IDirectInputDevice8*)pvRef;
    DIPROPRANGE propRange = {
        {
            sizeof(DIPROPRANGE),
            sizeof(DIPROPHEADER),
            lpddoi->dwOfs,
            DIPH_BYOFFSET,
        },
        -5000,
        5000
    };
    HRESULT res;
    if ((res = device->SetProperty(DIPROP_RANGE, &propRange.diph)) != DI_OK)
        //???
    {
        res = device->GetProperty(DIPROP_RANGE, &propRange.diph);
    }

    DIPROPDWORD propValue = {
        {
            sizeof(DIPROPDWORD),
            sizeof(DIPROPHEADER),
            lpddoi->dwOfs,
            DIPH_BYOFFSET,
        },
        1500
    };
    if ((res = device->SetProperty(DIPROP_DEADZONE, &propValue.diph)) != DI_OK)
        //???
    {}

    propValue.dwData = 9800;
    if ((res = device->SetProperty(DIPROP_SATURATION, &propValue.diph)) != DI_OK)
         //???
    {}

    return DIENUM_CONTINUE;
}
*/