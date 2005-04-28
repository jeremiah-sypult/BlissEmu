

#ifndef TYPES_H
#define TYPES_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//primitive types defined for Windows systems
typedef signed char				INT8,        *PINT8;
typedef signed short			INT16,       *PINT16;
typedef signed int				INT32,       *PINT32;

// There is no ANSI standard for 64 bit integers :(
#ifdef _MSC_VER
typedef __int64					INT64,       *PINT64;  // MS VC++
#else
typedef signed long long int	INT64,       *PINT64;  // Ming
#endif

typedef unsigned char			UINT8,       *PUINT8;
typedef unsigned short			UINT16,      *PUINT16;
typedef unsigned int			UINT32,      *PUINT32;

// There is no ANSI standard for 64 bit integers :(
#ifdef _MSC_VER
typedef unsigned __int64		UINT64,      *PUINT64;  // MS VC++
#else
typedef unsigned long long int	UINT64,      *PUINT64;	// Ming
#endif

typedef int                    BOOL,        *PBOOL;
typedef char                   CHAR,        *PCHAR;

typedef void                *PVOID;
typedef void                SURFACE,    *PSURFACE;
typedef unsigned int        ERRCODE,    *PERRCODE;

typedef struct tagXY        // X/Y-Width/Height coordinate
{
    long  x;
    long  y;
} XY, *PXY;

typedef struct tagWHRect    
{
    long x;        // top
    long y;        // left
    long w;        // width
    long h;        // height
} WHRect, *PWHRect;

#ifndef NULL
#define NULL    0
#endif

#define FALSE   0
#define TRUE    1

#define OK      0

#ifdef __cplusplus
}
#endif

#endif

