
#include <dinput.h>
#include "InputConsumerObject.h"

InputConsumerObject::InputConsumerObject(INT32 i, const CHAR* n, GUID ddg, INT32 doid)
: id(i),
  name(n),
  defaultDeviceGuid(ddg),
  defaultObjectID(doid),
  objectID(-1),
  producer(NULL)
{}

void InputConsumerObject::setDeviceInput(InputProducer* producer, INT32 objectid)
{
	this->producer = producer;
	this->objectID = objectid;
}

float InputConsumerObject::getInputValue()
{
	if (producer == NULL)
		return 0.0f;
		
	return producer->getValue(objectID);
}
