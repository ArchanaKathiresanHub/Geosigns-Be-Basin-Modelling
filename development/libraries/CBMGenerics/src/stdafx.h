// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#if defined(_WIN32) || defined (_WIN64)

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:


#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <limits>

#pragma warning(disable : 4996)

#endif

#include "DllExport.h"
