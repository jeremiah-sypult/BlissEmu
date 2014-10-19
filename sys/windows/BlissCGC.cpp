
#if defined( CLASSIC_GAME_CONTROLLER )
#include "cgc.h"

// Typedefs used to import functions
typedef int (* CGCOPEN)(UCHAR);
typedef int (* CGCCLOSE)();

// Local names for the functions we will import
CGCOPEN L_CGCOpen = NULL;
CGCCLOSE L_CGCClose = NULL;
CGCGETDATA L_CGCGetCookedIntyData = NULL;

bool usingCGC = false;
HMODULE hCGCLib = NULL;

void initCGC()
{
	usingCGC = false;

	hCGCLib  = LoadLibrary("cgc.dll");
	if (hCGCLib  == NULL)
        return;

	L_CGCOpen = (CGCOPEN)GetProcAddress(hCGCLib, "CGCOpen");
	L_CGCGetCookedIntyData = (CGCGETDATA)GetProcAddress(hCGCLib, "CGCGetCookedIntyData");
	L_CGCClose = (CGCCLOSE)GetProcAddress(hCGCLib, "CGCClose");

    if (!L_CGCOpen || !L_CGCGetCookedIntyData || !L_CGCClose) {
    	FreeLibrary(hCGCLib);
		return;
    }

    if (L_CGCOpen(TYPE_INTY) != CGC_OK) {
        L_CGCClose();
        usingCGC = false;
        L_CGCOpen = NULL;
        L_CGCGetCookedIntyData = NULL;
        L_CGCClose = NULL;
    }
    else
        usingCGC = true;
}

void releaseCGC()
{
    if (!usingCGC || !L_CGCClose)
        return;

    L_CGCClose();
	FreeLibrary(hCGCLib);
    usingCGC = false;
    L_CGCOpen = NULL;
    L_CGCGetCookedIntyData = NULL;
    L_CGCClose = NULL;
}

#endif /* CLASSIC_GAME_CONTROLLER */
