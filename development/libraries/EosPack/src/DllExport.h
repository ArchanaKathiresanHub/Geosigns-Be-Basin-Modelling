#ifndef DLLEXPORT_H
#define DLLEXPORT_H

#include "EosPack.h"
#include "ComponentManager.h"
#include "polynomials.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace pvtFlash
{


   #if defined(_WIN32) || defined (_WIN64)	


   struct ComputeStruct
   {
   public:
      double temperature;
      double pressure;
      double compMasses[CBMGenerics::ComponentManager::NumberOfOutputSpecies];
      double phaseCompMasses[N_PHASES * CBMGenerics::ComponentManager::NumberOfOutputSpecies];
      double phaseDensity[N_PHASES];
      double phaseViscosity[N_PHASES];
      bool isGormPrescribed;
      double gorm;        
   };

   extern "C" 
   {
      EOSPACK_DLL_EXPORT void __stdcall SetPvtPropertiesConfigFile(char* fileName)
      {	
         pvtFlash::pvtPropertiesConfigFile = string(fileName);
      }
      
      EOSPACK_DLL_EXPORT bool __stdcall EosPackComputeWithLumping(ComputeStruct* computeInfo)
      {
         double phaseCompMasses[N_PHASES][CBMGenerics::ComponentManager::NumberOfOutputSpecies];
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         
         bool result = instance.computeWithLumping(
                                                   computeInfo->temperature, 
                                                   computeInfo->pressure, 
                                                   computeInfo->compMasses,
                                                   phaseCompMasses,
                                                   computeInfo->phaseDensity, 
                                                   computeInfo->phaseViscosity, 
                                                   computeInfo->isGormPrescribed, 
                                                   computeInfo->gorm);
         
         int index = 0;
         for( int i = 0; i < N_PHASES; i++ )
            for( int j = 0; j < CBMGenerics::ComponentManager::NumberOfOutputSpecies; j++)
               computeInfo->phaseCompMasses[index++] = phaseCompMasses[i][j];
         return result;
      }
      
	  EOSPACK_DLL_EXPORT double __stdcall GetMolWeight(int componentId, double gorm)
      {
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         return instance.getMolWeightLumped(componentId, gorm);
      }
      
      EOSPACK_DLL_EXPORT double __stdcall Gorm(ComputeStruct* computeInfo)
      {
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         computeInfo->gorm = instance.gorm(computeInfo->compMasses);
		 return computeInfo->gorm;
      }
      
   }
#endif

}

#endif // DLLEXPORT