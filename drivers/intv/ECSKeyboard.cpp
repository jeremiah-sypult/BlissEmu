
#include "ECSKeyboard.h"

const INT32 ECSKeyboard::sortedObjectIndices[NUM_ECS_OBJECTS] = {
    24, 29, 34, 28, 33, 27, 32, 26, 31, 25, 30, // ESCAPE, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0
    23, 22, 40, 21, 39, 20, 38, 19, 37, 18,     // Q, W, E, R, T, Y, U, I, O, P
    46, 16, 45, 15, 44, 14, 43, 13, 42, 12, 36, // A, S, D, F, G, H, J, K, L, SEMICOLON, ENTER
    10,  4,  9,  3,  8,  2,  7,  1,  6,         // Z, X, C, V, B, N, M, COMMA, PERIOD
    41, 47,  5,                                 // CTRL, SHIFT, SPACE, 
    17, 11,  0,  35,                            // UP, DOWN, LEFT, RIGHT
};

ECSKeyboard::ECSKeyboard(INT32 id)
: InputConsumer(id),
  rowsToScan(0)
{
    memset(rowInputValues, 0, sizeof(rowInputValues));
    INT32 i = 0;
    inputConsumerObjects[0] = new InputConsumerObject(sortedObjectIndices[i++], "Left", GUID_SysKeyboard, DIK_LEFT);
    inputConsumerObjects[1] = new InputConsumerObject(sortedObjectIndices[i++], "Comma", GUID_SysKeyboard, DIK_COMMA);
    inputConsumerObjects[2] = new InputConsumerObject(sortedObjectIndices[i++], "N", GUID_SysKeyboard, DIK_N);
    inputConsumerObjects[3] = new InputConsumerObject(sortedObjectIndices[i++], "V", GUID_SysKeyboard, DIK_V);
    inputConsumerObjects[4] = new InputConsumerObject(sortedObjectIndices[i++], "X", GUID_SysKeyboard, DIK_X);
    inputConsumerObjects[5] = new InputConsumerObject(sortedObjectIndices[i++], "Space", GUID_SysKeyboard, DIK_SPACE);
    inputConsumerObjects[6] = new InputConsumerObject(sortedObjectIndices[i++], "Period", GUID_SysKeyboard, DIK_PERIOD);
    inputConsumerObjects[7] = new InputConsumerObject(sortedObjectIndices[i++], "M", GUID_SysKeyboard, DIK_M);
    inputConsumerObjects[8] = new InputConsumerObject(sortedObjectIndices[i++], "B", GUID_SysKeyboard, DIK_B);
    inputConsumerObjects[9] = new InputConsumerObject(sortedObjectIndices[i++], "C", GUID_SysKeyboard, DIK_C);
    inputConsumerObjects[10] = new InputConsumerObject(sortedObjectIndices[i++], "Z", GUID_SysKeyboard, DIK_Z);
    inputConsumerObjects[11] = new InputConsumerObject(sortedObjectIndices[i++], "Down", GUID_SysKeyboard, DIK_DOWN);
    inputConsumerObjects[12] = new InputConsumerObject(sortedObjectIndices[i++], "Semicolon", GUID_SysKeyboard, DIK_SEMICOLON);
    inputConsumerObjects[13] = new InputConsumerObject(sortedObjectIndices[i++], "K", GUID_SysKeyboard, DIK_K);
    inputConsumerObjects[14] = new InputConsumerObject(sortedObjectIndices[i++], "H", GUID_SysKeyboard, DIK_H);
    inputConsumerObjects[15] = new InputConsumerObject(sortedObjectIndices[i++], "F", GUID_SysKeyboard, DIK_F);
    inputConsumerObjects[16] = new InputConsumerObject(sortedObjectIndices[i++], "S", GUID_SysKeyboard, DIK_S);
    inputConsumerObjects[17] = new InputConsumerObject(sortedObjectIndices[i++], "Up", GUID_SysKeyboard, DIK_UP);
    inputConsumerObjects[18] = new InputConsumerObject(sortedObjectIndices[i++], "P", GUID_SysKeyboard, DIK_P);
    inputConsumerObjects[19] = new InputConsumerObject(sortedObjectIndices[i++], "I", GUID_SysKeyboard, DIK_I);
    inputConsumerObjects[20] = new InputConsumerObject(sortedObjectIndices[i++], "Y", GUID_SysKeyboard, DIK_Y);
    inputConsumerObjects[21] = new InputConsumerObject(sortedObjectIndices[i++], "R", GUID_SysKeyboard, DIK_R);
    inputConsumerObjects[22] = new InputConsumerObject(sortedObjectIndices[i++], "W", GUID_SysKeyboard, DIK_W);
    inputConsumerObjects[23] = new InputConsumerObject(sortedObjectIndices[i++], "Q", GUID_SysKeyboard, DIK_Q);
    inputConsumerObjects[24] = new InputConsumerObject(sortedObjectIndices[i++], "Escape", GUID_SysKeyboard, DIK_GRAVE);
    inputConsumerObjects[25] = new InputConsumerObject(sortedObjectIndices[i++], "9", GUID_SysKeyboard, DIK_NUMPAD9);
    inputConsumerObjects[26] = new InputConsumerObject(sortedObjectIndices[i++], "7", GUID_SysKeyboard, DIK_NUMPAD7);
    inputConsumerObjects[27] = new InputConsumerObject(sortedObjectIndices[i++], "5", GUID_SysKeyboard, DIK_NUMPAD5);
    inputConsumerObjects[28] = new InputConsumerObject(sortedObjectIndices[i++], "3", GUID_SysKeyboard, DIK_NUMPAD3);
    inputConsumerObjects[29] = new InputConsumerObject(sortedObjectIndices[i++], "1", GUID_SysKeyboard, DIK_NUMPAD1);
    inputConsumerObjects[30] = new InputConsumerObject(sortedObjectIndices[i++], "0", GUID_SysKeyboard, DIK_NUMPAD0);
    inputConsumerObjects[31] = new InputConsumerObject(sortedObjectIndices[i++], "8", GUID_SysKeyboard, DIK_NUMPAD8);
    inputConsumerObjects[32] = new InputConsumerObject(sortedObjectIndices[i++], "6", GUID_SysKeyboard, DIK_NUMPAD6);
    inputConsumerObjects[33] = new InputConsumerObject(sortedObjectIndices[i++], "4", GUID_SysKeyboard, DIK_NUMPAD4);
    inputConsumerObjects[34] = new InputConsumerObject(sortedObjectIndices[i++], "2", GUID_SysKeyboard, DIK_NUMPAD2);
    inputConsumerObjects[35] = new InputConsumerObject(sortedObjectIndices[i++], "Right", GUID_SysKeyboard, DIK_RIGHT);
    inputConsumerObjects[36] = new InputConsumerObject(sortedObjectIndices[i++], "Enter", GUID_SysKeyboard, DIK_RETURN);
    inputConsumerObjects[37] = new InputConsumerObject(sortedObjectIndices[i++], "O", GUID_SysKeyboard, DIK_O);
    inputConsumerObjects[38] = new InputConsumerObject(sortedObjectIndices[i++], "U", GUID_SysKeyboard, DIK_U);
    inputConsumerObjects[39] = new InputConsumerObject(sortedObjectIndices[i++], "T", GUID_SysKeyboard, DIK_T);
    inputConsumerObjects[40] = new InputConsumerObject(sortedObjectIndices[i++], "E", GUID_SysKeyboard, DIK_E);
    inputConsumerObjects[41] = new InputConsumerObject(sortedObjectIndices[i++], "Control", GUID_SysKeyboard, DIK_LCONTROL);
    inputConsumerObjects[42] = new InputConsumerObject(sortedObjectIndices[i++], "L", GUID_SysKeyboard, DIK_L);
    inputConsumerObjects[43] = new InputConsumerObject(sortedObjectIndices[i++], "J", GUID_SysKeyboard, DIK_J);
    inputConsumerObjects[44] = new InputConsumerObject(sortedObjectIndices[i++], "G", GUID_SysKeyboard, DIK_G);
    inputConsumerObjects[45] = new InputConsumerObject(sortedObjectIndices[i++], "D", GUID_SysKeyboard, DIK_D);
    inputConsumerObjects[46] = new InputConsumerObject(sortedObjectIndices[i++], "A", GUID_SysKeyboard, DIK_A);
    inputConsumerObjects[47] = new InputConsumerObject(sortedObjectIndices[i++], "Shift", GUID_SysKeyboard, DIK_LSHIFT);
}

ECSKeyboard::~ECSKeyboard()
{
    for (int i = 0; i < NUM_ECS_OBJECTS; i++)
        delete inputConsumerObjects[i];
}

INT32 ECSKeyboard::getInputConsumerObjectCount()
{
    return NUM_ECS_OBJECTS;
}

InputConsumerObject* ECSKeyboard::getInputConsumerObject(INT32 i)
{
    return inputConsumerObjects[sortedObjectIndices[i]];
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
