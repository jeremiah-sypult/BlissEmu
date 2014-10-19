
#include "InputProducerManager.h"
#include "KeyboardInputProducer.h"
#include "JoystickInputProducer.h"

#if defined( _WIN32 )
InputProducerManager::InputProducerManager(HWND w)
: wnd(w)
{
    DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL);
}
#endif

InputProducerManager::~InputProducerManager()
{
#if defined( _WIN32 )
    directInput->Release();
#endif
}

#if defined( _WIN32 )
IDirectInput8* InputProducerManager::getDevice()
{
    return directInput;
}
#endif

InputProducer* InputProducerManager::acquireInputProducer(GUID deviceGuid)
{
    InputProducer* ip = NULL;
#if defined( _WIN32 )
    DIDEVICEINSTANCE devInfo;
    devInfo.dwSize = sizeof(DIDEVICEINSTANCE);
    
    for (vector<InputProducer*>::iterator it = devices.begin(); it < devices.end(); it++) {
        InputProducer* nextIp = (*it);
        nextIp->getDevice()->GetDeviceInfo(&devInfo);
        if (devInfo.guidInstance == deviceGuid) {
            ip = nextIp;
            break;
        }
    }

    if (ip == NULL) {
        IDirectInputDevice8* nextDevice;
        directInput->CreateDevice(deviceGuid, &nextDevice, NULL);
        if (!nextDevice)
            return NULL;

        HRESULT res = nextDevice->SetCooperativeLevel(wnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
        if (res != DI_OK) {
            nextDevice->Release();
            return NULL;
        }

        nextDevice->GetDeviceInfo(&devInfo);
        if ((devInfo.dwDevType & 0xFF) == DI8DEVTYPE_KEYBOARD) {
            ip = new KeyboardInputProducer(deviceGuid, nextDevice);
            nextDevice->Acquire();
        }
        else if (((devInfo.dwDevType & 0xFF) == DI8DEVTYPE_JOYSTICK) ||
                ((devInfo.dwDevType & 0xFF) == DI8DEVTYPE_GAMEPAD))
        {
            ip = new JoystickInputProducer(deviceGuid, nextDevice);
            nextDevice->Acquire();
        }
        else {
            nextDevice->Release();
            return NULL;
        }

        if (ip != NULL)
            devices.push_back(ip);
    }
#else
	// TODO: jeremiah sypult cross-platform, hacking input
	for (vector<InputProducer*>::iterator it = devices.begin(); it < devices.end(); it++) {
        InputProducer* nextIp = (*it);
		GUID getGuid = nextIp->getGuid();
		int compare = memcmp((const void *)&getGuid, (const void *)&deviceGuid, sizeof(GUID));
        //nextIp->getDevice()->GetDeviceInfo(&devInfo);
        //if (devInfo.guidInstance == deviceGuid) {
		if ( compare ) {
            ip = nextIp;
            break;
        }
    }

	ip = new KeyboardInputProducer(deviceGuid);

	if (ip != NULL)
		devices.push_back(ip);
#endif
    return ip;
}

INT32 InputProducerManager::getAcquiredInputProducerCount()
{
    return (INT32)devices.size();
}

InputProducer* InputProducerManager::getAcquiredInputProducer(INT32 i)
{
    return devices[i];
}

void InputProducerManager::pollInputProducers()
{
    for (vector<InputProducer*>::iterator it = devices.begin(); it < devices.end(); it++)
        (*it)->poll();
}

void InputProducerManager::releaseInputProducers()
{
    for (vector<InputProducer*>::iterator it = devices.begin(); it < devices.end(); it++) {
#if defined( _WIN32 )
        IDirectInputDevice8* nextDevice = (*it)->getDevice();
        nextDevice->Unacquire();
        nextDevice->Release();
        delete (*it);
#endif
    }
    devices.clear();
}

