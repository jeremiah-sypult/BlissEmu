
#ifndef RIP_H
#define RIP_H

#include <string.h>
#include <vector>
#include "ripxbf.h"
#include "core/types.h"
#include "core/Peripheral.h"
#include "core/memory/Memory.h"

using namespace std;

#define MAX_BIOSES      16
#define MAX_PERIPHERALS 16

typedef struct _CartridgeConfiguration CartridgeConfiguration;

class Rip : public Peripheral
{

public:
    virtual ~Rip();

    void SetTargetSystemID(UINT32 t) { this->targetSystemID = t; }
    UINT32 GetTargetSystemID() { return targetSystemID; }

    void SetName(const CHAR* p);

    void SetProducer(const CHAR* p);
    const CHAR* GetProducer() { return producer; }

    void SetYear(const CHAR* y);
    const CHAR* GetYear() { return year; }

    PeripheralCompatibility GetPeripheralUsage(const CHAR* periphName);

    //load a regular .rip file
    static Rip* LoadRip(const CHAR* filename);

    //load a raw binary Atari 5200 image of a game
    static Rip* LoadA52(const CHAR* filename);

    //load a raw binary Intellivision image of a game
    static Rip* LoadBin(const CHAR* filename, const CHAR* cfgFilename);

    //load a raw binary image contained within a .zip file
    static Rip* LoadZip(const CHAR* filename, const CHAR* cfgFilename);

    //load an Intellivision .rom file
    static Rip* LoadRom(const CHAR* filename);

    BOOL SaveRip(const CHAR* filename);

    const CHAR* GetFileName() {
        return this->filename;
    }

    UINT32 GetCRC() {
        return this->crc;
    }

private:
    Rip(UINT32 systemID);
    //Rip(UINT32 systemID, const CHAR* nme, const CHAR* prducer, const CHAR* yr);

    void AddPeripheralUsage(const CHAR* periphName, PeripheralCompatibility usage);
    static Rip* LoadCartridgeConfiguration(const CHAR* cfgFile, UINT32 crc);

    void SetFileName(const CHAR* fname) {
        strncpy(this->filename, fname, sizeof(this->filename));
    }

    UINT32 targetSystemID;
    CHAR* producer;
    CHAR* year;

    //peripheral compatibility indicators
    CHAR* peripheralNames[MAX_PERIPHERALS];
    PeripheralCompatibility peripheralUsages[MAX_PERIPHERALS];
    UINT32 peripheralCount;

	CHAR filename[MAX_PATH];
	UINT32 crc;
};

#endif
