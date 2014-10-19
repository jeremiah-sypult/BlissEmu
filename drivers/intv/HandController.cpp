
#include "HandController.h"
#include "Intellivision.h"

// TODO: jeremiah sypult cross-platform
// classic game controller support should be handled in the input implementation
#if defined( CLASSIC_GAME_CONTROLLER )
#include "cgc.h"
#endif /* CLASSIC_GAME_CONTROLLER */

const double PI = 3.14159265358979323846264338327950288;

const float HandController::vectorParse = (float)sin(PI/4.0);

const UINT16 HandController::BUTTON_OUTPUT_VALUES[15] = {
    0x81, //OUTPUT_KEYPAD_ONE
    0x41, //OUTPUT_KEYPAD_TWO
    0x21, //OUTPUT_KEYPAD_THREE
    0x82, //OUTPUT_KEYPAD_FOUR
    0x42, //OUTPUT_KEYPAD_FIVE
    0x22, //OUTPUT_KEYPAD_SIX
    0x84, //OUTPUT_KEYPAD_SEVEN
    0x44, //OUTPUT_KEYPAD_EIGHT
    0x24, //OUTPUT_KEYPAD_NINE
    0x88, //OUTPUT_KEYPAD_CLEAR
    0x48, //OUTPUT_KEYPAD_ZERO
    0x28, //OUTPUT_KEYPAD_ENTER
    0xA0, //OUTPUT_ACTION_BUTTON_TOP
    0x60, //OUTPUT_ACTION_BUTTON_BOTTOM_LEFT
    0xC0  //OUTPUT_ACTION_BUTTON_BOTTOM_RIGHT
};

const UINT16 HandController::DIRECTION_OUTPUT_VALUES[16] = {
    0x04, //OUTPUT_DISC_NORTH
    0x14, //OUTPUT_DISC_NORTH_NORTH_EAST
    0x16, //OUTPUT_DISC_NORTH_EAST
    0x06, //OUTPUT_DISC_EAST_NORTH_EAST
    0x02, //OUTPUT_DISC_EAST
    0x12, //OUTPUT_DISC_EAST_SOUTH_EAST
    0x13, //OUTPUT_DISC_SOUTH_EAST
    0x03, //OUTPUT_DISC_SOUTH_SOUTH_EAST
    0x01, //OUTPUT_DISC_SOUTH
    0x11, //OUTPUT_DISC_SOUTH_SOUTH_WEST
    0x19, //OUTPUT_DISC_SOUTH_WEST
    0x09, //OUTPUT_DISC_WEST_SOUTH_WEST
    0x08, //OUTPUT_DISC_WEST
    0x18, //OUTPUT_DISC_WEST_NORTH_WEST
    0x1C, //OUTPUT_DISC_NORTH_WEST
    0x0C  //OUTPUT_DISC_NORTH_NORTH_WEST
};

HandController::HandController(INT32 id, const CHAR* n)
: InputConsumer(id),
  name(n)
{
    int i = 0;
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 1", GUID_SysKeyboard, DIK_NUMPAD7);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 2", GUID_SysKeyboard, DIK_NUMPAD8);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 3", GUID_SysKeyboard, DIK_NUMPAD9);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 4", GUID_SysKeyboard, DIK_NUMPAD4);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 5", GUID_SysKeyboard, DIK_NUMPAD5);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 6", GUID_SysKeyboard, DIK_NUMPAD6);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 7", GUID_SysKeyboard, DIK_NUMPAD1);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 8", GUID_SysKeyboard, DIK_NUMPAD2);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 9", GUID_SysKeyboard, DIK_NUMPAD3);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad CLEAR", GUID_SysKeyboard, DIK_NUMPADPERIOD);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad 0", GUID_SysKeyboard, DIK_NUMPAD0);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Keypad ENTER", GUID_SysKeyboard, DIK_NUMPADENTER);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Top Action Buttons", GUID_SysKeyboard, DIK_SPACE);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Bottom Left Action Button", GUID_SysKeyboard, DIK_LCONTROL);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Bottom Right Action Button", GUID_SysKeyboard, DIK_RCONTROL);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad Up", GUID_SysKeyboard, DIK_UP);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad NorthEast", GUID_SysKeyboard, DIK_W);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad Right", GUID_SysKeyboard, DIK_RIGHT);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad SouthEast", GUID_SysKeyboard, DIK_S);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad Down", GUID_SysKeyboard, DIK_DOWN);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad SouthWest", GUID_SysKeyboard, DIK_A);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad Left", GUID_SysKeyboard, DIK_LEFT);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Disc Pad NorthWest", GUID_SysKeyboard, DIK_Q);
}

HandController::~HandController()
{
    for (int i = 0; i < NUM_HAND_CONTROLLER_OBJECTS; i++)
        delete inputConsumerObjects[i];
}

INT32 HandController::getInputConsumerObjectCount()
{
    return NUM_HAND_CONTROLLER_OBJECTS;
}

InputConsumerObject* HandController::getInputConsumerObject(INT32 i)
{
    return inputConsumerObjects[i];
}

void HandController::evaluateInputs()
{
	static const float offset = (2.0f * PI)/16.0f;

    inputValue = 0;

    for (UINT16 i = 0; i < 16; i++) {
        if (inputConsumerObjects[i]->getInputValue() == 1.0f)
            inputValue |= BUTTON_OUTPUT_VALUES[i];
    }

    //evaluate the disc
    float neswVector =
        (inputConsumerObjects[16]->getInputValue() -
        inputConsumerObjects[20]->getInputValue()) *
        vectorParse;
    float nwseVector =
        (inputConsumerObjects[22]->getInputValue() -
        inputConsumerObjects[18]->getInputValue()) *
        vectorParse;
    float yPos = inputConsumerObjects[15]->getInputValue() -
        inputConsumerObjects[19]->getInputValue() +
        nwseVector + neswVector;
    float xPos = inputConsumerObjects[17]->getInputValue() -
        inputConsumerObjects[21]->getInputValue() -
        nwseVector + neswVector;

	if (xPos != 0 || yPos != 0) {
		float positionInRadians = (atan2f(-xPos, -yPos)+PI);
		UINT16 directionIndex = (UINT16)((positionInRadians+(offset/2.0))/offset) & 0x0F;
		UINT16 directionValue = DIRECTION_OUTPUT_VALUES[directionIndex];
		inputValue |= directionValue;
	}

#if defined( CLASSIC_GAME_CONTROLLER )
    if (usingCGC) {
        //also support the CGC
        USHORT intyData;
        L_CGCGetCookedIntyData(getId() == 0 ? CONTROLLER_0 : CONTROLLER_1, &intyData);

        //decode the CGC data
        if (intyData & 0x000F)
            inputValue |= BUTTON_OUTPUT_VALUES[(intyData & 0x000F)-1];
        if (intyData & 0x0010)
            inputValue |= BUTTON_OUTPUT_VALUES[12];
        if (intyData & 0x0020)
            inputValue |= BUTTON_OUTPUT_VALUES[13];
        if (intyData & 0x0040)
            inputValue |= BUTTON_OUTPUT_VALUES[14];
        if (intyData & 0x0080)
            inputValue |= DIRECTION_OUTPUT_VALUES[(intyData & 0x0F00) >> 8];
    }
#endif /* CLASSIC_GAME_CONTROLLER */
    inputValue = (UINT16)(0xFF ^ inputValue);
}

void HandController::resetInputConsumer()
{
    inputValue = 0xFF;
}

void HandController::setOutputValue(UINT16)
{}

UINT16 HandController::getInputValue()
{
    return inputValue;
}

