#ifndef EOSPACKINDIRECT_DLL_EXPORT_H
#define EOSPACKINDIRECT_DLL_EXPORT_H

#if defined(_WIN32) || defined (_WIN64)	

#include "EosPack.h"

extern "C" 
{
    EOSPACK_DLL_EXPORT void __stdcall SetPvtPropertiesConfigFile(char* fileName);
    EOSPACK_DLL_EXPORT bool __stdcall EosPackComputeWithLumping(ComputeStruct* computeInfo);
	EOSPACK_DLL_EXPORT double __stdcall GetMolWeight(int componentId, double gorm);
	EOSPACK_DLL_EXPORT double __stdcall Gorm(ComputeStruct* computeInfo);
}
#endif

#endif
