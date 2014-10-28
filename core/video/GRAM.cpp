
#include "GRAM.h"

#define GRAM_ADDRESS    0x3800
#define GRAM_READ_MASK  0xF9FF
#define GRAM_WRITE_MASK 0x39FF

GRAM::GRAM()
: RAM(GRAM_SIZE, GRAM_ADDRESS, GRAM_READ_MASK, GRAM_WRITE_MASK)
{}

void GRAM::reset()
{
    UINT16 i;
    dirtyRAM = TRUE;
    for (i = 0; i < GRAM_SIZE; i++)
        image[i] = 0;

    for (i = 0; i < 0x40; i++)
        dirtyCards[i] = TRUE;
}

UINT16 GRAM::peek(UINT16 location) {
    return (enabled ? image[location & 0x01FF] : location);
}

void GRAM::poke(UINT16 location, UINT16 value)
{
    if (!enabled)
        return;

    location &= 0x01FF;
    value &= 0xFF;

    if (image[location] == value)
        return;

    image[location] = value;
    dirtyCards[location>>3] = TRUE;
    dirtyRAM = TRUE;
}

void GRAM::markClean() {
    if (!dirtyRAM)
        return;

    for (UINT16 i = 0; i < 0x40; i++)
        dirtyCards[i] = FALSE;
    dirtyRAM = FALSE;
}

BOOL GRAM::isDirty() {
    return dirtyRAM;
}

BOOL GRAM::isCardDirty(UINT16 cardLocation) {
    return dirtyCards[cardLocation>>3];
}

RAMState GRAM::getState(UINT16* image)
{
	RAMState state = {0};

	state = RAM::getState(NULL);

	if (image != NULL) {
		this->getImage(image, 0, this->getImageByteSize());
	}

	return state;
}

void GRAM::setState(RAMState state, UINT16* image)
{
	RAM::setState(state, NULL);

	if (image != NULL) {
		this->setImage(image, 0, this->getImageByteSize());
	}

	memset(this->dirtyCards, TRUE, sizeof(this->dirtyCards));
	this->dirtyRAM = TRUE;
}
