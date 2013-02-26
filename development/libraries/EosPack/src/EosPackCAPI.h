#ifndef EOS_PACK_C_API_H
#define EOS_PACK_C_API_H

#include "ComponentManager.h"

/// \File EosPackCAPI.h
/// \brief Provides backward compatibility with C like API to EosPack library
///        The new implementation of API use swig
namespace pvtFlash
{
   struct ComputeStruct
   {
   public:
      double temperature;
      double pressure;
      double compMasses[CBMGenerics::ComponentManager::NumberOfOutputSpecies];
      double phaseCompMasses[CBMGenerics::ComponentManager::NumberOfPhases * CBMGenerics::ComponentManager::NumberOfOutputSpecies];
      double phaseDensity[CBMGenerics::ComponentManager::NumberOfPhases];
      double phaseViscosity[CBMGenerics::ComponentManager::NumberOfPhases];
      bool isGormPrescribed;
      double gorm;        
   };

   extern "C" 
   {
      void SetPvtPropertiesConfigFile(char* fileName);     
      bool EosPackComputeWithLumping(ComputeStruct* computeInfo);    
      double GetMolWeight(int componentId, double gorm);     
      double Gorm(ComputeStruct* computeInfo);

      /// \brief Create PT phase diagram
      /// \param diagType - type of diagram, 0 - mass, 1 - mole, 2 - volume
      /// \param T trap temperature
      /// \param P trap pressure
      /// \param[in] comp array of size 23 for composition mass fractions
      /// \param[out] points array, size of 8 which will contain on return CriticalT, CriticalP, BubbleT, BubbleP, CricondenthermT, CricondenthermP, CricondenbarT, CricondenbarP
      /// \param[in,out] szIso array sizeof 11 (number of isolines) for each isoline it contains on input maximum number of isoline points allocated in isoline array,
      ///                      on output it contains real number of points for each isoline
      /// \param[out] isolines 1D array which keeps T,P values for each isoline, number of points for each isoline keeps szIso array 
      /// \return true on success, false otherwise
      bool BuildPTDiagram( int diagType, double T, double P, double * comp, double * points, int * szIso, double * isolines );
   }
}

#endif // EOS_PACK_C_API_H
