#if defined(_WIN32) || defined (_WIN64)

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#define NOMINMAX
#include <windows.h>


#define EOSPACK_EXPORTS

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EOSPACK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EOSPACK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#if defined EOSPACK_STATIC
#	define EOSPACK_DLL_EXPORT
#else
#	ifdef EOSPACK_EXPORTS
#		define EOSPACK_DLL_EXPORT __declspec(dllexport)
#	else
#		define EOSPACK_DLL_EXPORT __declspec(dllimport)
#	endif
#endif

#endif
