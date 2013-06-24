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

      /// \brief Calculate phase masses, phase density and phase viscosity using C arrays as parameters
      /// \param temperature temperature of composition
      /// \param pressure pressure of composition
      /// \param[in] compMasses array of size 23 with mass for each component
      /// \param isGormPrescribed is gas/oil ration coeficient is given
      /// \param gorm value of gas/oil ration coeficient 
      /// \param phaseCompMasses[out] aray of size 46 with masses for each phase for each component
      /// \param phaseDensity array of size 2 with densities for liquid/vapour phases
      /// \param phaseViscosity array of size 2 with viscosities for liquid/vapour phases
      /// \return true if flashing was successfull, false otherwise
      bool EosPackComputeWithLumpingArr( double temperature, double pressure, double * compMasses, bool isGormPrescribed, double gorm,
                                         double * phaseCompMasses, double * phaseDensity, double * phaseViscosity );

      double GetMolWeight(int componentId, double gorm);     

      /// \brief Calculate gas/oil ratio for given composition
      /// \param[in] compMasses array of size 23
      /// \return calculated gas/oil ratio coefficient value
      double Gorm(double * compMasses);

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
