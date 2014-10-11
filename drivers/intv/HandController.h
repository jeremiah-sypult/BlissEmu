
#ifndef HANDCONTROLLER_H
#define HANDCONTROLLER_H

#define NUM_HAND_CONTROLLER_OBJECTS 23

#include <math.h>
#include "core/audio/AY38914_InputOutput.h"
#include "core/input/InputConsumer.h"

class HandController : public AY38914_InputOutput, public InputConsumer
{

    public:
        HandController(INT32 id, const CHAR* n);
        virtual ~HandController();

        const CHAR* getName() { return name; }
	    
        void resetInputConsumer();

        /**
         * Poll the controller.  This function is invoked by the
         * InputConsumerBus just after the Emulator indicates it has entered
         * vertical blank.
         */
        void evaluateInputs();

        //functions to get descriptive info about the input consumer
        INT32 getInputConsumerObjectCount();
        InputConsumerObject* getInputConsumerObject(INT32 i);

        void setOutputValue(UINT16 value);
        UINT16 getInputValue();

    private:
        const CHAR* name;

        InputConsumerObject* inputConsumerObjects[NUM_HAND_CONTROLLER_OBJECTS];
        UINT16            inputValue;

        static const float vectorParse;
        static const UINT16 BUTTON_OUTPUT_VALUES[15];
        static const UINT16 DIRECTION_OUTPUT_VALUES[16];

};

#endif
