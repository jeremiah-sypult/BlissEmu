
#ifndef INPUTCONSUMEROBJECT_H
#define INPUTCONSUMEROBJECT_H

#include "InputProducer.h"
#include "core/types.h"

class InputConsumerObject
{
    friend class InputConsumer;

public:
	InputConsumerObject(INT32 id, const CHAR* name, GUID defaultDeviceGuid, INT32 defaultObjectID);

    INT32 getId() { return id; }

    const CHAR* getName() { return name; }
	
    GUID getDefaultDeviceGuid() { return defaultDeviceGuid; }
	
    INT32 getDefaultEnum() { return defaultObjectID; }
    
    InputProducer* getInputProducer() { return producer; }
	
    INT32 getEnum() { return objectID; }
	
	void setDeviceInput(InputProducer* producer, INT32 objectid);
	
	float getInputValue();

private:
    INT32 id;
  	const CHAR* name;
    GUID defaultDeviceGuid;
    INT32  defaultObjectID;
	
    InputProducer* producer;
    INT32  objectID;
	
};

#endif