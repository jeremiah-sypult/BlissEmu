
#include "ECSKeyboard.h"

ECSKeyboard::ECSKeyboard(INT32 id)
: InputConsumer(id),
  rowsToScan(0)
{
    memset(rowInputValues, 0, sizeof(rowInputValues));
    int i = 0;
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Left", GUID_SysKeyboard, DIK_LEFT);
    inputConsumerObjects[i++] = new InputConsumerObject(i, ",", GUID_SysKeyboard, DIK_COMMA);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "N", GUID_SysKeyboard, DIK_N);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "V", GUID_SysKeyboard, DIK_V);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "X", GUID_SysKeyboard, DIK_X);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Space", GUID_SysKeyboard, DIK_SPACE);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Period", GUID_SysKeyboard, DIK_PERIOD);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "M", GUID_SysKeyboard, DIK_M);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "B", GUID_SysKeyboard, DIK_B);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "C", GUID_SysKeyboard, DIK_C);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Z", GUID_SysKeyboard, DIK_Z);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Down", GUID_SysKeyboard, DIK_DOWN);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Semicolon", GUID_SysKeyboard, DIK_SEMICOLON);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "K", GUID_SysKeyboard, DIK_K);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "H", GUID_SysKeyboard, DIK_H);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "F", GUID_SysKeyboard, DIK_F);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "S", GUID_SysKeyboard, DIK_S);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Up", GUID_SysKeyboard, DIK_UP);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "P", GUID_SysKeyboard, DIK_P);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "I", GUID_SysKeyboard, DIK_I);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Y", GUID_SysKeyboard, DIK_Y);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "R", GUID_SysKeyboard, DIK_R);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "W", GUID_SysKeyboard, DIK_W);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Q", GUID_SysKeyboard, DIK_Q);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Escape", GUID_SysKeyboard, DIK_ESCAPE);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "9", GUID_SysKeyboard, DIK_NUMPAD9);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "7", GUID_SysKeyboard, DIK_NUMPAD7);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "5", GUID_SysKeyboard, DIK_NUMPAD5);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "3", GUID_SysKeyboard, DIK_NUMPAD3);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "1", GUID_SysKeyboard, DIK_NUMPAD1);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "0", GUID_SysKeyboard, DIK_NUMPAD0);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "8", GUID_SysKeyboard, DIK_NUMPAD8);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "6", GUID_SysKeyboard, DIK_NUMPAD6);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "4", GUID_SysKeyboard, DIK_NUMPAD4);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "2", GUID_SysKeyboard, DIK_NUMPAD2);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Right", GUID_SysKeyboard, DIK_RIGHT);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Enter", GUID_SysKeyboard, DIK_RETURN);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "O", GUID_SysKeyboard, DIK_O);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "U", GUID_SysKeyboard, DIK_U);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "T", GUID_SysKeyboard, DIK_T);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "E", GUID_SysKeyboard, DIK_E);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Control", GUID_SysKeyboard, DIK_LCONTROL);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "L", GUID_SysKeyboard, DIK_L);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "J", GUID_SysKeyboard, DIK_J);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "G", GUID_SysKeyboard, DIK_G);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "D", GUID_SysKeyboard, DIK_D);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "A", GUID_SysKeyboard, DIK_A);
    inputConsumerObjects[i++] = new InputConsumerObject(i, "Shift", GUID_SysKeyboard, DIK_LSHIFT);
}

ECSKeyboard::~ECSKeyboard()
{
    for (int i = 0; i < NUM_ECS_OBJECTS; i++)
        delete inputConsumerObjects[i];
}

UINT32 ECSKeyboard::getInputConsumerObjectCount()
{
    return NUM_ECS_OBJECTS;
}

InputConsumerObject* ECSKeyboard::getInputConsumerObject(int i)
{
    return inputConsumerObjects[i];
}

void ECSKeyboard::resetInputConsumer()
{
    rowsToScan = 0;
    for (UINT16 i = 0; i < 8; i++)
        rowInputValues[i] = 0;
}

void ECSKeyboard::evaluateInputs()
{
    for (UINT16 row = 0; row < 6; row++) {
        rowInputValues[row] = 0;
        if (inputConsumerObjects[row]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x01;
        if (inputConsumerObjects[row+6]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x02;
        if (inputConsumerObjects[row+12]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x04;
        if (inputConsumerObjects[row+18]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x08;
        if (inputConsumerObjects[row+24]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x10;
        if (inputConsumerObjects[row+30]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x20;
        if (inputConsumerObjects[row+36]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x40;
        if (inputConsumerObjects[row+42]->getInputValue() == 1.0f)
            rowInputValues[row] |= 0x80;
    }
    rowInputValues[6] = (inputConsumerObjects[47]->getInputValue() == 1.0f ? 0x80 : 0);
}

UINT16 ECSKeyboard::getInputValue()
{
    UINT16 inputValue = 0;
    UINT16 rowMask = 1;
    for (UINT16 row = 0; row < 8; row++)  {
        if ((rowsToScan & rowMask) != 0) {
            rowMask = (UINT16)(rowMask << 1);
            continue;
        }
        inputValue |= rowInputValues[row];

        rowMask = (UINT16)(rowMask << 1);
    }

    return (UINT16)(0xFF ^ inputValue);
}

void ECSKeyboard::setOutputValue(UINT16 value) {
    this->rowsToScan = value;
}
