
#include "BlissConfiguration.h"
#include "core/input/InputConsumer.h"
#include "core/input/InputConsumerObject.h"

BlissConfiguration configuration;

BlissConfiguration::BlissConfiguration()
{
}

InputConfiguration* BlissConfiguration::LoadInputConfiguration(InputConsumer* inputConsumer, InputConsumerObject* inputObject)
{
    InputConfiguration* inputConfig = new InputConfiguration();
    memset(inputConfig, 0, sizeof(InputConfiguration));
    inputConfig->inputObject = inputObject;

    UINT8* buffer = NULL;
    UINT32 size = 0;
    CHAR* key = new CHAR[strlen(inputConsumer->getName()) + strlen(inputObject->getName()) + 2];
    strcpy(key, inputConsumer->getName());
    strcat(key, ".");
    strcat(key, inputObject->getName());
    theApp.GetProfileBinary("Input", key, &buffer, &size);
    delete[] key;
    if (buffer != NULL) {
        ParseInputConfiguration(inputConfig, buffer, size);
        delete[] buffer;
    }
    else {
        //DEPRECATED: try getting the configuration the old way
        GUID producerGuid = inputObject->getDefaultDeviceGuid();
        key = new CHAR[strlen(inputConsumer->getName()) + strlen(inputObject->getName()) + 13];
        strcpy(key, inputConsumer->getName());
        strcat(key, ".");
        strcat(key, inputObject->getName());
        strcat(key, ".DeviceGUID");
        theApp.GetProfileBinary("Input", key, &buffer, &size);
        delete[] key;
        if (buffer != NULL) {
            if (size == sizeof(GUID))
                memcpy(&producerGuid, buffer, sizeof(GUID));
            delete[] buffer;
        }
        INT32 inputEnum = inputObject->getDefaultEnum();
        key = new CHAR[strlen(inputConsumer->getName()) + strlen(inputObject->getName()) + 12];
        strcpy(key, inputConsumer->getName());
        strcat(key, ".");
        strcat(key, inputObject->getName());
        strcat(key, ".InputEnum");
        inputEnum = theApp.GetProfileInt("Input", key, inputEnum);
        delete[] key;

        if (inputEnum != -1) {
            inputConfig->producerIDs[0] = new GUID[1];
            inputConfig->producerIDs[0][0] = producerGuid;
            inputConfig->objectIDs[0] = new INT32[1];
            inputConfig->objectIDs[0][0] = inputEnum;
            inputConfig->subBindingCounts[0] = 1;
            inputConfig->bindingCount = 1;
        }
    }
    return inputConfig;
}

void BlissConfiguration::ParseInputConfiguration(InputConfiguration* inputConfig, UINT8* buffer, INT32 size)
{
    UINT8* endMarker = buffer+size;
    UINT8* currentReadMarker = buffer;
    if (currentReadMarker+sizeof(INT32) > endMarker)
        //ERROR!
        return;

    inputConfig->bindingCount = *((INT32*)currentReadMarker);
    currentReadMarker += sizeof(INT32);

    if (inputConfig->bindingCount > MAX_BINDINGS)
        inputConfig->bindingCount = MAX_BINDINGS;

    //parse each binding
    for (INT32 i = 0; i < inputConfig->bindingCount; i++) {
        if (currentReadMarker+sizeof(INT32) > endMarker) {
            //ERROR! truncate the bindings here
            inputConfig->bindingCount = i;
            return;
        }

        inputConfig->subBindingCounts[i] = *((INT32*)currentReadMarker);
        currentReadMarker += sizeof(INT32);

        INT32 guidArraySize = inputConfig->subBindingCounts[i]*sizeof(GUID);
        INT32 objectIDArraySize = inputConfig->subBindingCounts[i]*sizeof(INT32);
        if (currentReadMarker+guidArraySize+objectIDArraySize > endMarker) {
            //ERROR! truncate the bindings here
            inputConfig->subBindingCounts[i] = 0;
            inputConfig->bindingCount = i;
            return;
        }

        inputConfig->producerIDs[i] = new GUID[inputConfig->subBindingCounts[i]];
        memcpy(inputConfig->producerIDs[i], currentReadMarker, guidArraySize);
        currentReadMarker += guidArraySize;

        inputConfig->objectIDs[i] = new INT32[inputConfig->subBindingCounts[i]];
        memcpy(inputConfig->objectIDs[i], currentReadMarker, objectIDArraySize);
        currentReadMarker += objectIDArraySize;
    }
}

void BlissConfiguration::SaveInputConfiguration(InputConsumer* inputConsumer, InputConsumerObject* inputObject, InputConfiguration* inputConfig)
{
    CHAR* key = new CHAR[strlen(inputConsumer->getName()) + strlen(inputObject->getName()) + 2];
    strcpy(key, inputConsumer->getName());
    strcat(key, ".");
    strcat(key, inputObject->getName());

    if (inputConfig == NULL) {
        theApp.WriteProfileBinary("Input", key, NULL, 0);
        delete[] key;
        return;
    }

    INT32 bufferSize = sizeof(INT32);
    for (INT32 i = 0; i < inputConfig->bindingCount; i++)
        bufferSize += sizeof(INT32) + (inputConfig->subBindingCounts[i]*(sizeof(GUID)+sizeof(INT32)));

    UINT8* buffer = new UINT8[bufferSize];
    UINT8* currentWriteMarker = buffer;
    *((INT32*)currentWriteMarker) = inputConfig->bindingCount;
    currentWriteMarker += sizeof(INT32);
    for (INT32 i = 0; i < inputConfig->bindingCount; i++) {
        *((INT32*)currentWriteMarker) = inputConfig->subBindingCounts[i];
        currentWriteMarker += sizeof(INT32);
        INT32 guidArraySize = inputConfig->subBindingCounts[i]*sizeof(GUID);
        memcpy(currentWriteMarker, inputConfig->producerIDs[i], guidArraySize);
        currentWriteMarker += guidArraySize;
        INT32 objectIDArraySize = inputConfig->subBindingCounts[i]*sizeof(INT32);
        memcpy(currentWriteMarker, inputConfig->objectIDs[i], objectIDArraySize);
        currentWriteMarker += objectIDArraySize;
    }

    theApp.WriteProfileBinary("Input", key, buffer, bufferSize);
    delete[] key;
    delete[] buffer;

}

void BlissConfiguration::ReleaseInputConfiguration(InputConfiguration* inputConfig)
{
    if (inputConfig == NULL)
        return;

    for (INT32 i = 0; i < inputConfig->bindingCount; i++) {
        delete[] inputConfig->producerIDs[i];
        delete[] inputConfig->objectIDs[i];
    }
    delete inputConfig;
}



    /*
    for (int k = 0; k < objCount; k++) {
        InputConsumerObject* nextObj = nextIc->getInputConsumerObject(k);

        //get the input producer (joystick, keyboard, etc.) bound to this object
        InputProducer* producer = bindings[id][k].inputProducer;
        //save the binding to the registry
        CHAR* key = new CHAR[strlen(nextIc->getName()) + strlen(nextObj->getName()) + 13];
        strcpy(key, nextIc->getName());
        strcat(key, ".");
        strcat(key, nextObj->getName());
        strcat(key, ".DeviceGUID");
        GUID g;
        memset(&g, 0, sizeof(GUID));
        if (producer != NULL)
            g = producer->getGuid();
        theApp.WriteProfileBinary("Input", key, (LPBYTE)&g, sizeof(GUID));
        delete[] key;

        //get the input enum (joystick axes, keyboard buttons, etc.) bound to this object
        INT32 e = bindings[id][k].inputEnum;
        //save the binding to the registry
        key = new CHAR[strlen(nextIc->getName()) + strlen(nextObj->getName()) + 12];
        strcpy(key, nextIc->getName());
        strcat(key, ".");
        strcat(key, nextObj->getName());
        strcat(key, ".InputEnum");
        theApp.WriteProfileInt("Input", key, e);
        delete[] key;
    }
    */

/*
        InputConsumerObject* nextObject = inputConsumer->getInputConsumerObject(i);

        //get the producer associated with this object
        InputProducer* producer = nextObject->getInputProducer();
        if (producer == NULL) {
            GUID g = nextObject->getDefaultDeviceGuid();
            CHAR* key = new CHAR[strlen(inputConsumer->getName()) + strlen(nextObject->getName()) + 13];
            sprintf(key, "%s.%s.DeviceGUID", inputConsumer->getName(), nextObject->getName());
            UINT8* buffer = NULL;
            UINT32 size;
            theApp.GetProfileBinary("Input", key, &buffer, &size);
            delete[] key;
            if (buffer != NULL) {
                if (size == sizeof(GUID))
                    memcpy(&g, buffer, sizeof(GUID));
                delete[] buffer;
            }
            producer = manager->acquireInputProducer(g);
            if (producer == NULL)
                continue;
        }

        //now get the input enum associated with this object
        INT32 e = nextObject->getEnum();
        if (e < 0 || e >= producer->getInputCount()) { 
            e = nextObject->getDefaultEnum();
            CHAR* key = new CHAR[strlen(inputConsumer->getName()) + strlen(nextObject->getName()) + 12];
            strcpy(key, inputConsumer->getName());
            strcat(key, ".");
            strcat(key, nextObject->getName());
            strcat(key, ".InputEnum");
            e = theApp.GetProfileInt("Input", key, e);
            delete[] key;
            if (e < 0 || e >= producer->getInputCount())
                continue;
        }

        //bind the current settings to the input object
        bindings[inputConsumer->getId()][i].inputProducer = producer;
        bindings[inputConsumer->getId()][i].inputEnum = e;
        */
        /*
            //find the producer to bind with this object
            GUID g = nextObject->getDefaultDeviceGuid();
            CHAR* key = new CHAR[strlen(nextInputConsumer->getName()) + strlen(nextObject->getName()) + 13];
            strcpy(key, nextInputConsumer->getName());
            strcat(key, ".");
            strcat(key, nextObject->getName());
            strcat(key, ".DeviceGUID");
            UINT8* buffer;
            UINT32 size;
            theApp.GetProfileBinary("Input", key, &buffer, &size);
            delete[] key;
            if (buffer != NULL) {
                if (size == sizeof(GUID))
                    memcpy(&g, buffer, sizeof(GUID));
                delete[] buffer;
            }
            InputProducer* producer = manager->acquireInputProducer(g);
            if (producer == NULL) {
                nextObject->setDeviceInput(NULL, -1);
                continue;
            }

            //now get the input enum associated with this object
            INT32 e = nextObject->getEnum();
            if (e < 0 || e >= producer->getInputCount()) { 
                e = nextObject->getDefaultEnum();
                CHAR* key = new CHAR[strlen(nextInputConsumer->getName()) + strlen(nextObject->getName()) + 12];
                strcpy(key, nextInputConsumer->getName());
                strcat(key, ".");
                strcat(key, nextObject->getName());
                strcat(key, ".InputEnum");
                e = theApp.GetProfileInt("Input", key, e);
                delete[] key;
                if (e < 0 || e >= producer->getInputCount()) {
                    nextObject->setDeviceInput(NULL, -1);
                    continue;
                }
            }

            //attach the desired producer enum to the consumer object
            nextObject->setDeviceInput(producer, e);
        */

