// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#ifdef UNICODE
#define STRING  LPWSTR
#define CSTRING LPCWSTR
#else
#define STRING  LPSTR
#define CSTRING LPCSTR
#endif

#define MAX_STRING 260

#ifdef WIN32X
#define Malloc(size)		HeapAlloc(GetProcessHeap(), NULL, size)
#define MallocZ(size)		HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define Free(p)				HeapFree(GetProcessHeap(), NULL, p);
#else
#include <malloc.h>
#define _CRT_SECURE_NO_WARNINGS
#define sprintf_s sprintf
#define _itoa_s _itoa
#define Malloc(size)		malloc(size)
#define MallocZ(size)	\
	x = malloc(size); \
	memset(x, 0, size);
#define Free(p)				free(p)
#endif
