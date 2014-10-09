
#ifndef POKEY_INPUT_H
#define POKEY_INPUT_H

class Pokey_Input
{

public:
    //indicates the currently depressed key(s)
    virtual UINT8 getKeyPadCode() = 0;
	
	//a value of 0-228 indicating the potentiometer location in the X direction
    virtual UINT8 getPotX() = 0;

    //a value of 0-228 indicating the potentiometer location in the Y direction
    virtual UINT8 getPotY() = 0;

};

#endif
