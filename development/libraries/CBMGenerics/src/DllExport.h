#ifndef CBMGENERICS_DLL_EXPORT_H
#define CBMGENERICS_DLL_EXPORT_H

#if defined(_WIN32) || defined (_WIN64)

#if defined CBMGENERICS_STATIC
#	define CBMGENERICS_DLL_EXPORT
#else
#	ifdef CBMGENERICS_EXPORTS
#		define CBMGENERICS_DLL_EXPORT __declspec(dllexport)
#	else
#		define CBMGENERICS_DLL_EXPORT __declspec(dllimport)
#	endif
#endif

#else

#define CBMGENERICS_DLL_EXPORT

#endif

#endif
