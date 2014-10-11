#pragma once

#include "stdafx.h"

#include "BlissApp.h"
#include "core/types.h"
#include "core/input/InputConsumerObject.h"

typedef struct _InputConfiguration InputConfiguration;

class BlissConfiguration
{
public:
	BlissConfiguration();

    InputConfiguration* LoadInputConfiguration(InputConsumer* inputConsumer, InputConsumerObject*);
    void SaveInputConfiguration(InputConsumer* inputConsumer, InputConsumerObject*, InputConfiguration*);
    void ReleaseInputConfiguration(InputConfiguration*);

private:
    void ParseInputConfiguration(InputConfiguration* inputConfig, UINT8* buffer, INT32 size);

};

typedef struct _InputConfiguration
{
    InputConsumerObject* inputObject;
    GUID*   producerIDs[MAX_BINDINGS];
    INT32*  objectIDs[MAX_BINDINGS];
    INT32   subBindingCounts[MAX_BINDINGS];
    INT32   bindingCount;

} InputConfiguration;

extern BlissConfiguration configuration;
