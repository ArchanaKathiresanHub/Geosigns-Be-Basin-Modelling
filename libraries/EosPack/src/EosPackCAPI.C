#include "EosPack.h"
#include "polynomials.h"
#include "PTDiagramCalculator.h"

#include "EosPackCAPI.h"

#include <vector>
#include <string>

namespace pvtFlash
{
   extern "C" 
   {
      void SetPvtPropertiesConfigFile(char* fileName)
      {  
         pvtFlash::pvtPropertiesConfigFile = std::string(fileName);
      }
      
      bool EosPackComputeWithLumping(ComputeStruct* computeInfo)
      {
         double phaseCompMasses[CBMGenerics::ComponentManager::NumberOfPhases][CBMGenerics::ComponentManager::NumberOfOutputSpecies];
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         
         bool result = instance.computeWithLumping( computeInfo->temperature, 
                                                    computeInfo->pressure, 
                                                    computeInfo->compMasses,
                                                    phaseCompMasses,
                                                    computeInfo->phaseDensity, 
                                                    computeInfo->phaseViscosity, 
                                                    computeInfo->isGormPrescribed, 
                                                    computeInfo->gorm
                                                  );
         
         int index = 0;
         for( int i = 0; i < CBMGenerics::ComponentManager::NumberOfPhases; i++ )
         {
            for( int j = 0; j < CBMGenerics::ComponentManager::NumberOfOutputSpecies; j++)
            {
               computeInfo->phaseCompMasses[index++] = phaseCompMasses[i][j];
            }
         }
         return result;
      }
      
      double GetMolWeight(int componentId, double gorm)
      {
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         return instance.getMolWeightLumped(componentId, gorm);
      }
      
      double Gorm(ComputeStruct* computeInfo)
      {
         pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         computeInfo->gorm = instance.gorm(computeInfo->compMasses);
         return computeInfo->gorm;
      }

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
      bool BuildPTDiagram( int diagType, double T, double P, double * comp, double * points, int * szIso, double * isolines )
      {
         const int iNc = CBMGenerics::ComponentManager::NumberOfOutputSpecies;

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
            diagBuilder.setNonLinSolverConvPrms( 1e-6, 500 );

            diagBuilder.findBubbleDewLines( T, P, std::vector<double>() );

            // fill critical point values
            const std::pair<double,double> & critPt = diagBuilder.getCriticalPoint();
            points[0] = critPt.first;
            points[1] = critPt.second;

            // fill bubble point values
            points[2] = T;
            double bubbleP;
            if ( diagBuilder.getBubblePressure( T, &bubbleP ) )
            {
               points[3] = bubbleP;
            }
            else
            {
               points[3] = P;
            }
            // fill cricondentherm point values
            const std::pair<double, double> & tPt = diagBuilder.getCricondenthermPoint();
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
               const std::vector< std::pair<double,double> > & isoline = diagBuilder.calcContourLine( vals[i] );
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
}
