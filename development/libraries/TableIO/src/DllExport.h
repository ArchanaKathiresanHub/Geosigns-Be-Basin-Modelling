#ifndef TABLEIO_DLL_EXPORT_H
#define TABLEIO_DLL_EXPORT_H

#if defined(_WIN32) || defined (_WIN64)

#if defined TABLEIO_STATIC
#	define TABLEIO_DLL_EXPORT
#else
#	ifdef TABLEIO_EXPORTS
#		define TABLEIO_DLL_EXPORT __declspec(dllexport)
#	else
#		define TABLEIO_DLL_EXPORT __declspec(dllimport)
#	endif
#endif

#else

#define TABLEIO_DLL_EXPORT

#endif

#endif
