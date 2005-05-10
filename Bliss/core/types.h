
#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

//primitive types
typedef signed char				INT8;
typedef unsigned char			UINT8;
typedef signed short			INT16;
typedef unsigned short			UINT16;
typedef signed int				INT32;
typedef unsigned int			UINT32;

// There is no ANSI standard for 64 bit integers :(
#ifdef _MSC_VER
typedef __int64					INT64;  // MS VC++
typedef unsigned __int64		UINT64;  // MS VC++
#else
typedef signed long long int	INT64;  // Ming
typedef unsigned long long int	UINT64;	// Ming
#endif

typedef int                    BOOL;
#define TRUE    1
#define FALSE   0

typedef char                   CHAR;

#ifndef NULL
#define NULL    0
#endif

#ifdef __cplusplus
}
#endif

#endif //TYPES_H

