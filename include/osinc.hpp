//---------------------------------------------------------------------
// Copyright 2020 embeddedKeith
// -*^*-
//---------------------------------------------------------------------
#ifndef _OSINC_
#define _OSINC_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#define _Export
#define _MSExport 

#ifndef _BUILDSHARE
	#define _QnxExport extern
#else
	#define _QnxExport
#endif

#define embeddedSleep(msecs) delay(msecs)

#ifndef _T
typedef char _T;
#endif

#ifndef fd_t
typedef int fd_t;
#endif

#ifndef FILETIME
typedef time_t FILETIME;
#endif

#ifndef SYSTEMTIME
typedef time_t SYSTEMTIME;
#endif

#ifndef HANDLE
typedef int HANDLE;
#endif

#ifndef INVALID_HANDLE_VALUE 
#define INVALID_HANDLE_VALUE NULL
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET NULL
#endif

#define Sleep(msec) embeddedSleep(msec)

#ifndef BOOL
typedef bool BOOL;
#define TRUE 1
#define FALSE 0
#endif

#ifndef Boolean
typedef BOOL Boolean;
#define true TRUE
#define false FALSE
#endif

#ifndef BOOLEAN
typedef bool BOOLEAN;
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef PVoid
typedef void* PVoid;
#endif

#ifndef PVOID
typedef PVoid PVOID;
#endif

#ifndef MAXDWORD
#define MAXDWORD ((DWORD)0xffffffffffffffff)
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef USHORT
typedef unsigned short USHORT;
#endif

#ifndef SHORT
typedef short SHORT;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef INT
typedef int INT;
#endif

#ifndef PINT
typedef int* PINT;
#endif

#ifndef DOUBLE
typedef double DOUBLE;
#endif

#ifndef FLOAT
typedef float FLOAT;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef LONG
typedef long LONG;
#endif

#ifndef LARGE_INTEGER
typedef _Uint64t LARGE_INTEGER;
#endif

#ifndef UINT64
typedef _Uint64t UINT64;
#endif

#ifndef INT64
typedef _Int64t INT64;
#endif

#ifndef LPCTSTR
typedef const char* LPCTSTR;
#endif

#ifndef CHAR
typedef char CHAR;
#endif

#ifndef PUCHAR
typedef unsigned char* PUCHAR;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef UCHAR
typedef unsigned char UCHAR;
#endif

#ifndef PCHAR
typedef char* PCHAR;
#endif

#ifndef TCHAR
typedef char TCHAR;
#endif

#ifndef PSZ
typedef char* PSZ;
#endif

void deletePSZ(PSZ & psz); 

Boolean _Export loadPSZ(PSZ& pszTarget,const char* pszSource,const PSZ pszErr);


#ifndef COMSTAT
typedef unsigned long COMSTAT;
#endif

#ifndef INFINITE
#define INFINITE 0xffffffffL
#endif

#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_NORMAL 0
#endif

#define numElements(a) (sizeof(a) / sizeof(a[0]))
#define offset(a, b) (((PUCHAR)(((a *)0)->b)) - ((PUCHAR)((a *)0)))

#define blankField(a) memset(a, ' ', sizeof(a))
#define clearField(a) memset(a, 0, sizeof(a))
#define clearCopy(a, b) {memset(a, 0, sizeof(a)); strncpy(a, b, sizeof(a));}
#define stringFill(a, b) stringToChar(a, b, sizeof(b))
#define loadStringFromField(a) loadString(a, sizeof(a))
#define setFieldFromString(a, b, c) setStringField(b, a, sizeof(a), c)

int getNumber(const char* cp, int len);
INT getBcdDigit(UCHAR ucBcdDigit);
void setBcdDigit(UCHAR& ucBcdDigit, INT iVal);

#endif
