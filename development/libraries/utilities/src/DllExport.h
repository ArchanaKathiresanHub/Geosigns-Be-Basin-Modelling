#ifndef UTILITIES_DLL_EXPORT_H
#define UTILITIES_DLL_EXPORT_H

#if defined(_WIN32) || defined (_WIN64)

#if defined UTILITIES_STATIC
#	define UTILITIES_DLL_EXPORT
#else
#	ifdef UTILITIES_EXPORTS
#		define UTILITIES_DLL_EXPORT __declspec(dllexport)
#	else
#		define UTILITIES_DLL_EXPORT __declspec(dllimport)
#	endif
#endif

#else

#define UTILITIES_DLL_EXPORT

#endif

#endif
