
#if defined( CLASSIC_GAME_CONTROLLER )
extern bool usingCGC;
void initCGC();
void releaseCGC();

typedef int (* CGCGETDATA)(UCHAR, USHORT *);
extern CGCGETDATA L_CGCGetCookedIntyData;
#endif
