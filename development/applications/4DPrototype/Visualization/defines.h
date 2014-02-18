#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#ifdef VISUALIZATIONDLL_EXPORTS
#define VISUALIZATIONDLL_API __declspec(dllexport) 
#else
#define VISUALIZATIONDLL_API __declspec(dllimport) 
#endif

#endif
