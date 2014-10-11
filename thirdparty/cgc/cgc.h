#ifndef CGC_H_INCLUDED
#define CGC_H_INCLUDED

#ifdef CGC_EXPORTS
#define CGC_API __declspec(dllexport)
#else
#define CGC_API __declspec(dllimport)
#endif

enum
{
	CGC_OK = -100,
	ERR_GENERROR,
	ERR_THREAD,
	ERR_OPEN,
	ERR_PURGE,
	ERR_SEND,
	ERR_READ,
	ERR_TXTIMEOUT,
	ERR_RXTIMEOUT,
	ERR_SETBAUD,
	ERR_SETDATACHAR,
	ERR_SETFLOW,
	ERR_SETTIMEOUT,
	ERR_RESETDEV,
	ERR_CLOSE,
	ERR_ALREADYRUNNING,
	ERR_NOTOPEN
};

#ifndef USHORT
	typedef unsigned short USHORT;
#endif

#ifndef UCHAR
	typedef unsigned char UCHAR;
#endif

const UCHAR CONTROLLER_0 = 0;
const UCHAR CONTROLLER_1 = 1;
const UCHAR CONTROLLER_2 = 2;
const UCHAR CONTROLLER_3 = 3;
const UCHAR TYPE_INTY		= 0xA0;
const UCHAR TYPE_NES		= 0xA1;
const UCHAR TYPE_SNES		= 0xA2;
const UCHAR TYPE_ATARIJOY	= 0xA3;

#ifdef __cplusplus
extern "C" {
#endif

CGC_API int CGCOpen(UCHAR ucControlType);
CGC_API int CGCGetRawIntyData(UCHAR side, USHORT *pData);
CGC_API int CGCGetCookedIntyData(UCHAR side, USHORT *pData);
CGC_API int CGCClose();
CGC_API int CGCGetSerial(char *szSerial);
CGC_API int CGCGetErrorString(int iError, char *szErrorStr, DWORD cchTextMax);

#ifdef __cplusplus
}
#endif


#endif