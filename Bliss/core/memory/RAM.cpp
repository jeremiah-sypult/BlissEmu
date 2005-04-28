
#include "RAM.h"

RAM::RAM(UINT16 size, UINT16 location)
{
    this->size = size;
    this->location = location;
    this->readAddressMask = 0xFFFF;
    this->writeAddressMask = 0xFFFF;
    this->bitWidth = sizeof(UINT16)<<3;
    this->trimmer = (UINT16)((1 << (sizeof(UINT16) << 3)) - 1);
    image = new UINT16[size];
}

RAM::RAM(UINT16 size, UINT16 location, UINT8 bitWidth)
{
    this->size = size;
    this->location = location;
    this->readAddressMask = 0xFFFF;
    this->writeAddressMask = 0xFFFF;
    this->bitWidth = bitWidth;
    this->trimmer = (UINT16)((1 << bitWidth) - 1);
    image = new UINT16[size];
}

RAM::RAM(UINT16 size, UINT16 location, UINT16 readAddressMask, UINT16 writeAddressMask)
{
    this->size = size;
    this->location = location;
    this->readAddressMask = readAddressMask;
    this->writeAddressMask = writeAddressMask;
    this->bitWidth = sizeof(UINT16)<<3;
    this->trimmer = (UINT16)((1 << bitWidth) - 1);
    image = new UINT16[size];
}

RAM::RAM(UINT16 size, UINT16 location, UINT16 readAddressMask, UINT16 writeAddressMask, UINT8 bitWidth)
{
    this->size = size;
    this->location = location;
    this->readAddressMask = readAddressMask;
    this->writeAddressMask = writeAddressMask;
    this->bitWidth = bitWidth;
    this->trimmer = (UINT16)((1 << bitWidth) - 1);
    image = new UINT16[size];
}

RAM::~RAM()
{
    delete[] image;
}

void RAM::reset()
{
    for (UINT16 i = 0; i < size; i++)
        image[i] = 0;
}

UINT8 RAM::getBitWidth()
{
    return bitWidth;
}

UINT16 RAM::getSize()
{
    return size;
}

UINT16 RAM::getAddress()
{
    return location;
}

UINT16 RAM::getReadAddressMask()
{
    return readAddressMask;
}

UINT16 RAM::getWriteAddressMask()
{
    return writeAddressMask;
}

UINT16 RAM::peek(UINT16 location)
{
    return image[location-this->location];
}

void RAM::poke(UINT16 location, UINT16 value)
{
    image[location-this->location] = (value & trimmer);
}

