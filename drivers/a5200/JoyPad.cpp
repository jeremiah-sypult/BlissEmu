
#include "JoyPad.h"

// TODO: jeremiah sypult cross-platform
#if defined( _WIN32 )
#include "dinput.h"
#endif

const UINT8 JoyPad::KBCODES[15] = {
        0x0C, //Start   1100
        0x08, //Pause   1000
        0x04, //Reset   0100
        0x0F, //1       1111
        0x0E, //2       1110
        0x0D, //3       1101
        0x0B, //4       1011
        0x0A, //5       1010
        0x09, //6       1001
        0x07, //7       0111
        0x06, //8       0110
        0x05, //9       0101
        0x03, //*       0011
        0x02, //0       0010
        0x01, //#       0001
};

JoyPad::JoyPad(INT32 id, const CHAR* name)
: InputConsumer(id)
{
	this->name = new CHAR[strlen(name)+1];
    strcpy(this->name, name);

    INT32 i = 0;
    inputObjects[0] = new InputConsumerObject(i++, "Start", GUID_SysKeyboard, DIK_RETURN);
    inputObjects[1] = new InputConsumerObject(i++, "Pause", GUID_SysKeyboard, DIK_TAB);
    inputObjects[2] = new InputConsumerObject(i++, "Reset", GUID_SysKeyboard, DIK_F2);
    inputObjects[3] = new InputConsumerObject(i++, "Keypad 1", GUID_SysKeyboard, DIK_NUMPAD7);
    inputObjects[4] = new InputConsumerObject(i++, "Keypad 2", GUID_SysKeyboard, DIK_NUMPAD8);
    inputObjects[5] = new InputConsumerObject(i++, "Keypad 3", GUID_SysKeyboard, DIK_NUMPAD9);
    inputObjects[6] = new InputConsumerObject(i++, "Keypad 4", GUID_SysKeyboard, DIK_NUMPAD4);
    inputObjects[7] = new InputConsumerObject(i++, "Keypad 5", GUID_SysKeyboard, DIK_NUMPAD5);
    inputObjects[8] = new InputConsumerObject(i++, "Keypad 6", GUID_SysKeyboard, DIK_NUMPAD6);
    inputObjects[9] = new InputConsumerObject(i++, "Keypad 7", GUID_SysKeyboard, DIK_NUMPAD1);
    inputObjects[10] = new InputConsumerObject(i++, "Keypad 8", GUID_SysKeyboard, DIK_NUMPAD2);
    inputObjects[11] = new InputConsumerObject(i++, "Keypad 9", GUID_SysKeyboard, DIK_NUMPAD3);
    inputObjects[12] = new InputConsumerObject(i++, "Keypad Star (*)", GUID_SysKeyboard, DIK_NUMPADPERIOD);
    inputObjects[13] = new InputConsumerObject(i++, "Keypad 0", GUID_SysKeyboard, DIK_NUMPAD0);
    inputObjects[14] = new InputConsumerObject(i++, "Keypad Pound (#)", GUID_SysKeyboard, DIK_NUMPADENTER);
    inputObjects[15] = new InputConsumerObject(i++, "Top Action Buttons", GUID_SysKeyboard, DIK_LCONTROL);
    inputObjects[16] = new InputConsumerObject(i++, "Bottom Action Buttons", GUID_SysKeyboard, DIK_LALT);
    inputObjects[17] = new InputConsumerObject(i++, "JoyStick Up", GUID_SysKeyboard, DIK_UP);
    inputObjects[18] = new InputConsumerObject(i++, "JoyStick Right", GUID_SysKeyboard, DIK_RIGHT);
    inputObjects[19] = new InputConsumerObject(i++, "JoyStick Down", GUID_SysKeyboard, DIK_DOWN);
    inputObjects[20] = new InputConsumerObject(i++, "JoyStick Left", GUID_SysKeyboard, DIK_LEFT);
}

JoyPad::~JoyPad()
{
    delete[] name;

    for (INT32 i = 0; i < 21; i++)
        delete inputObjects[i];
}

const CHAR* JoyPad::getName()
{
    return name;
}

void JoyPad::resetInputConsumer()
{
    kbcode = 0;
    potx = 0;
    poty = 0;
}

void JoyPad::evaluateInputs()
{
	//scan the keypad
	kbcode = 0;
	for (int i = 0; i < 15; i++) {
		if (inputObjects[i]->getInputValue() != 0.0f)
		    kbcode |= KBCODES[i];
	}
	
	//top fire buttons
    /*
	if (inputObjects[15]->getInputValue() != 0.0f)
		kbcode |= 0x20;
    */
	
    //x direction
    potx = (UINT8)(114.0f - (inputObjects[20]->getInputValue() * 114.0f) + (inputObjects[18]->getInputValue() * 114.0f));
    //y direction
    poty = (UINT8)(114.0f - (inputObjects[17]->getInputValue() * 114.0f) + (inputObjects[19]->getInputValue() * 114.0f));
}

