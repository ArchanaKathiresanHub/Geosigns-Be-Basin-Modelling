//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "EosPack.h"
#include "polynomials.h"
#include "PTDiagramCalculator.h"

#include "EosPackCAPI.h"

#include <algorithm>
#include <cassert>
#include <vector>
#include <string>
#include <memory>

namespace pvtFlash
{
   extern "C" 
   {
      void SetPvtPropertiesConfigFile(const char* fileName)
      {  
         pvtFlash::pvtPropertiesConfigFile = std::string(fileName);
      }
      
      bool EosPackComputeWithLumping(ComputeStruct* computeInfo)
      {
         double phaseCompMasses[PhaseId::NUMBER_OF_PHASES][ComponentId::NUMBER_OF_SPECIES];
         std::unique_ptr<EosPack> instance( EosPack::createNewInstance() );
         
         bool result = instance->computeWithLumping( computeInfo->temperature, 
                                                     computeInfo->pressure, 
                                                     computeInfo->compMasses,
                                                     phaseCompMasses,
                                                     computeInfo->phaseDensity, 
                                                     computeInfo->phaseViscosity, 
                                                     computeInfo->isGormPrescribed, 
                                                     computeInfo->gorm
                                                   );
         
         int index = 0;
         for( int i = 0; i < PhaseId::NUMBER_OF_PHASES; i++ )
         {
            for( int j = 0; j < ComponentId::NUMBER_OF_SPECIES; j++)
            {
               computeInfo->phaseCompMasses[index++] = phaseCompMasses[i][j];
            }
         }
         return result;
      }

      // Calculate phase masses, phase density and phase viscosity using C arrays as parameters
      // return true on success, false otherwise
      bool EosPackComputeWithLumpingArr( double temperature,       // temperature of composition
                                         double pressure,          // pressure of composition
                                         double * compMasses,      // array of size 23 with mass for each component
                                         bool isGormPrescribed,    // is gas/oil ration coeficient is given
                                         double gorm,              // value of gas/oil ration coeficient 
                                         double * phaseCompMasses, // aray of size 46 with masses for each phase for each component
                                         double * phaseDensity,    // array of size 2 with densities for liquid/vapour phases
                                         double * phaseViscosity   // array of size 2 with viscosities for liquid/vapour phases
                                       ) 
      {
         assert( compMasses );
         assert( phaseCompMasses );
         assert( phaseDensity );
         assert( phaseViscosity );

         double phaseMasses[PhaseId::NUMBER_OF_PHASES][ComponentId::NUMBER_OF_SPECIES];
         std::unique_ptr<EosPack> instance( EosPack::createNewInstance() );
         
         bool result = instance->computeWithLumping( temperature, 
                                                     pressure, 
                                                     compMasses,
                                                     phaseMasses,
                                                     phaseDensity, 
                                                     phaseViscosity, 
                                                     isGormPrescribed, 
                                                     gorm
                                                   );
         
         int index = 0;
         for( int i = 0; i < PhaseId::NUMBER_OF_PHASES; i++ )
         {
            for( int j = 0; j < ComponentId::NUMBER_OF_SPECIES; j++)
            {
               phaseCompMasses[index++] = phaseMasses[i][j];
            }
         }
         return result;
      }

      double GetMolWeight(int componentId, double gorm)
      {
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         return instance.getMolWeightLumped(componentId, gorm);
      }

      // Calculate gas/oil ratio
      // compMasses must be array of size 23
      double Gorm(double * compMasses)
      {  
         assert( compMasses );
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         return instance.gorm(compMasses);
      }

      // Create PT phase diagram
      // return true on success, false otherwise
      bool BuildPTDiagram( int diagType,     // type of diagram, 0 - mass, 1 - mole, 2 - volume
                           double T,         // trap temperature
                           double P,         // trap pressure
                           double * comp,    // array of size 23 for composition masses
                           double * points,  // points array, size of 8 which will contain on return CriticalT, CriticalP, BubbleT, BubbleP, CricondenthermT, CricondenthermP, CricondenbarT, CricondenbarP
                           int * szIso,      // szIso array sizeof 11 (number of isolines) for each isoline it contains on input maximum number of isoline points allocated in isoline array,
                                             // on output it contains real number of points for each isoline
                           double * isolines // isolines 1D array which keeps T, P values for each isoline, number of points for each isoline keeps szIso array 
                         )
      {
         const int iNc = ComponentId::NUMBER_OF_SPECIES;

         std::vector<double> masses( iNc );
         std::copy( comp, comp + iNc, masses.begin() );
        
         PTDiagramCalculator::DiagramType dType = PTDiagramCalculator::MoleMassFractionDiagram;
         switch( diagType )
         {
            case 0:  dType = PTDiagramCalculator::MassFractionDiagram;     break;
            case 1:  dType = PTDiagramCalculator::MoleMassFractionDiagram; break;
            case 2:  dType = PTDiagramCalculator::VolumeFractionDiagram;   break;
            default: break; // the default is MoleMassFraction diagram
         }

         try
         {
            PTDiagramCalculator diagBuilder( dType, masses );

            diagBuilder.setAoverBTerm( 2 );
            diagBuilder.setNonLinSolverConvPrms( 1e-6, 500, 0.3 );

            diagBuilder.findBubbleDewLines( T, P, std::vector<double>() );

            // fill critical point values
            const PTDiagramCalculator::TPPoint & critPt = diagBuilder.getCriticalPoint();
            points[0] = critPt.first;
            points[1] = critPt.second;

            // fill bubble point values
            points[2] = T;
            double bubbleP;
            if ( diagBuilder.getBubblePressure( T, &bubbleP ) )
            {
               points[3] = bubbleP;
            }
            else // if not found - assign to 0.0
            {
               points[2] = 0.0;
               points[3] = 0.0;
            }
            // fill cricondentherm point values
            const PTDiagramCalculator::TPPoint & tPt = diagBuilder.getCricondenthermPoint();
            points[4] = tPt.first;
            points[5] = tPt.second;

            // fill cricondenbar point values
            const std::pair<double, double> & pPt = diagBuilder.getCricondenbarPoint();
            points[6] = pPt.first;
            points[7] = pPt.second;

            // fill isolines array for 11 isoline, first - dew line, last bubble line
            double vals[]   = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
            std::vector<double> isoData;
            size_t is = 0;
            for ( int i = 0; i < sizeof(vals)/sizeof(double); ++i )
            {
               const PTDiagramCalculator::TPLine & isoline = diagBuilder.calcContourLine( vals[i] );
               szIso[i] = std::min( static_cast<int>(isoline.size()), szIso[i] );
               for ( size_t it = 0; it < szIso[i]; ++it )
               {
                  isolines[is]   = isoline[it].first;
                  isolines[is+1] = isoline[it].second;
                  is += 2;
               }
            }
         }
         catch( ... )
         {
            return false;
         }
         return true;
      }
   }
   
   //  Search critical point on PT phase diagram
   // return true on success, false otherwise
   bool FindCriticalPoint( int diagType,   // diagType - type of diagram, 0 - mass, 1 - mole, 2 - volume
                           double * comp,  // array of size 23 for composition mass fractions
                           double * critPt // critical point array, size of 2 which will contain on return CriticalT, CriticalP
                         )
   {
      const int iNc = ComponentId::NUMBER_OF_SPECIES;
      bool ret = false;

      std::vector<double> masses( iNc );
      std::copy( comp, comp + iNc, masses.begin() );
     
      PTDiagramCalculator::DiagramType dType = PTDiagramCalculator::MoleMassFractionDiagram;
      switch( diagType )
      {
         case 0:  dType = PTDiagramCalculator::MassFractionDiagram;     break;
         case 1:  dType = PTDiagramCalculator::MoleMassFractionDiagram; break;
         case 2:  dType = PTDiagramCalculator::VolumeFractionDiagram;   break;
         default: break; // the default is MoleMassFraction diagram
      }

      try
      {
         PTDiagramCalculator diagBuilder( dType, masses );

         diagBuilder.setAoverBTerm( 2 );
         diagBuilder.setNonLinSolverConvPrms( 1e-6, 500, 0.3 );
         const PTDiagramCalculator::TPPoint & pt = diagBuilder.searchCriticalPoint();
         if ( pt.first > 0.0 && pt.second > 0.0 )
         {
            critPt[0] = pt.first;
            critPt[1] = pt.second;
            ret = true;
         }
      }
      catch( ... )
      {
         return false;
      }
      return ret;
   }
}

