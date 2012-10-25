#ifndef SERIALDATAACCESS_DLL_EXPORT_H
#define SERIALDATAACCESS_DLL_EXPORT_H

#if defined(_WIN32) || defined (_WIN64)

#if defined SERIALDATAACCESS_STATIC
#	define SERIALDATAACCESS_DLL_EXPORT
#else
#	ifdef SERIALDATAACCESS_EXPORTS
#		define SERIALDATAACCESS_DLL_EXPORT __declspec(dllexport)
#	else
#		define SERIALDATAACCESS_DLL_EXPORT __declspec(dllimport)
#	endif
#endif

#else

#define SERIALDATAACCESS_DLL_EXPORT

#endif

#endif
