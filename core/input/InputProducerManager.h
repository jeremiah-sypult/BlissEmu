
#ifndef INPUTPRODUCERMANAGER_H
#define INPUTPRODUCERMANAGER_H

#include <windows.h>
#include <dinput.h>
#include "InputProducer.h"
#include <vector>
using namespace std;

class InputProducerManager
{

public:
    InputProducerManager(HWND wnd);
    virtual ~InputProducerManager();

    IDirectInput8* getDevice();
    
    InputProducer* acquireInputProducer(GUID deviceGuid);
    INT32 getAcquiredInputProducerCount();
    InputProducer* getAcquiredInputProducer(INT32 i);
	
	void pollInputProducers();
	
	void releaseInputProducers();

private:
    HWND wnd;
    IDirectInput8* directInput;
    vector<InputProducer*> devices;

};

#endif