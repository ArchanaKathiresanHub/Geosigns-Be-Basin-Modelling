#ifndef SERIALHDF5_DLL_EXPORT_H
#define SERIALHDF5_DLL_EXPORT_H

#if defined(_WIN32) || defined (_WIN64)

#if defined SERIALHDF5_STATIC
#	define SERIALHDF5_DLL_EXPORT
#else
#	ifdef SERIALHDF5_EXPORTS
#		define SERIALHDF5_DLL_EXPORT __declspec(dllexport)
#	else
#		define SERIALHDF5_DLL_EXPORT __declspec(dllimport)
#	endif
#endif

#else

#define SERIALHDF5_DLL_EXPORT

#endif

#endif
