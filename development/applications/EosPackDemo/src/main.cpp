// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string.h>


#include "EosPack.h"

const char * composFileName = nullptr;

bool calcGorm   = false;
bool calcProp   = false;
bool calcPhases = false;


void ParseArgs( int argc, char * argv[] )
{
   for ( int i = 0; i < argc; ++i )
   {
      if (      !strcmp( argv[i], "-gorm"  ) ) calcGorm   = true;
      else if ( !strcmp( argv[i], "-prop"  ) ) calcProp   = true;
      else if ( !strcmp( argv[i], "-phases") ) calcPhases = true;
      else  composFileName = argv[i];
   }
}

void ReadComposition( double compMasses[ComponentId::NUMBER_OF_SPECIES], double & pressure, double & temperature, double & gorm )
{

   if ( composFileName )
   {
      std::ifstream ifc( composFileName, std::ifstream::in );

      if ( ifc.good() )
      {
         ifc >> pressure >> temperature;

         for ( int i = ComponentId::NUMBER_OF_SPECIES - 1; i >= 0; --i ) ifc >> compMasses[i];

         std::cout << "Read" << std::endl;

         char c;
         ifc >> c;
         if ( c == 'g' )
         {
            std::cin >> gorm;
         }
         return;
      }
   }
   
   std::cin >> pressure >> temperature;

   for ( int i = ComponentId::NUMBER_OF_SPECIES - 1; i >= 0; --i ) std::cin >> compMasses[i];

   std::cout << "Read" << std::endl;
   char c;
   std::cin >> c;
   if (c == 'g')
   {
      std::cin >> gorm;
   }
}

int CalcAll( double compMasses[ComponentId::NUMBER_OF_SPECIES], double pressure, double temperature, double gorm )
{
   CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();

   bool   isGormPrescribed = false;

   double phaseCompMasses[PhaseId::NUMBER_OF_PHASES][ComponentId::NUMBER_OF_SPECIES];
   double phaseDensity   [PhaseId::NUMBER_OF_PHASES];
   double phaseViscosity [PhaseId::NUMBER_OF_PHASES];

   isGormPrescribed = gorm < 0 ? false : true;

   std::cout.precision (8);

   double totMass = 0;
   for (int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i)
   {
      std::cout << "mass " << std::setw(12) << theComponentManager.getSpeciesName (i) << ": " << std::setw(14) << compMasses[i] << "\n";
      totMass += compMasses[i];
   }

   std::cout << "\nTotal mass:    " << totMass << " [kg]\n";
   std::cout << "Temperature:   " << temperature << " [K]\n";
   std::cout << "Pressure:      " << pressure << " [Pa]\n\n";

   if ( pvtFlash::EosPack::getInstance().computeWithLumping ( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm ) )
   {
      //std::cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      std::cout << "results of run 1:\n\n";
      std::cout << "OilDensity: " << phaseDensity[1] << "\n";
      std::cout << "GasDensity: " << phaseDensity[0] << "\n\n";

      double liqPhaseMass = 0;
      double vapPhaseMass = 0;

      for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
      {
         liqPhaseMass += phaseCompMasses[1][i];
         vapPhaseMass += phaseCompMasses[0][i];
      }
      
      std::cout << "Liquid phase mass: " << liqPhaseMass << "\n";
      std::cout << "Vapour phase mass: " << vapPhaseMass << "\n\n";

      std::cout << "Liquid mass phase fraction: " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n";
      std::cout << "Vapour mass phase fraction: " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n\n";

      std::vector<double>compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );

      double gorm = pvtFlash::gorm(compMassesVec);
      std::cout << "Gorm = " << gorm << "\n\n";

      liqPhaseMass = 0;
      vapPhaseMass = 0;

      for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
      {
         double molW = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm );
         liqPhaseMass += phaseCompMasses[1][i]/molW;
         vapPhaseMass += phaseCompMasses[0][i]/molW;
      }
      std::cout << "Liquid mole phase fraction: " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n";
      std::cout << "Vapour mole phase fraction: " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n\n";
   }

   for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES_TO_FLASH; ++i )
   {
      std::cout << "mass " << std::setw(12) << theComponentManager.getSpeciesName (i) << " [gasphase]: " << std::setw(12) << phaseCompMasses[0][i] << " --- "
           << "mass " << std::setw(12) << theComponentManager.getSpeciesName (i) << " [oilphase]: " << std::setw(12) << phaseCompMasses[1][i] << "\n";
   }


   if ( pvtFlash::EosPack::getInstance().computeWithLumping( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm ) )
   {
      //std::cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      std::cout.precision (8);
      std::cout << "\nresults of run 2:" << "\n";
      std::cout << "OilDensity: " << phaseDensity[1] << "\n";
      std::cout << "GasDensity: " << phaseDensity[0] << "\n\n";

      
      std::vector<double> compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );

      double compMolWeight0, compMolWeight, compMolWeightL, compMolWeightV;

      double gorm_unlumped = pvtFlash::gorm( compMassesVec );
      std::cout << "Gorm unlumped = " << gorm_unlumped << "\n\n";
      
      std::vector<double> outWeights( ComponentId::NUMBER_OF_SPECIES_TO_FLASH );
      double unlumpFractions[ComponentId::NUMBER_OF_SPECIES];

      pvtFlash::EosPack::getInstance ().lumpComponents(compMasses, (double *)(&outWeights[0]), unlumpFractions);
 
      //    double gorm_lumped = pvtFlash::gorm(outWeights);
      //  std::cout << "Gorm lumped = " << gorm_lumped  << std::endl;
      double gorm_lumped =  gorm_unlumped;
      for (int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
      {
         compMolWeight0 = compMolWeight = compMolWeightV = 0;
         if( i < ComponentId::NUMBER_OF_SPECIES_TO_FLASH)
         {
            compMolWeight0  = pvtFlash::EosPack::getInstance().getMolWeight( i, 0 );
            compMolWeight   = pvtFlash::EosPack::getInstance().getMolWeight( i, gorm_unlumped );
         }
         else
         {
            compMolWeight0  = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, 0 );
            compMolWeight   = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm_unlumped );
         }

         compMolWeightL = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm_lumped );

         const double testGorm = 2.3;

         double testMW = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, testGorm );

         std::cout << "Mol weight(0, unlumpgorm, lumpgorm, testgorm): " << std::setw(12) << theComponentManager.getSpeciesName(i) << " (" << std::setw(2) << i << ") = ";

         std::cout << std::setw(10) << compMolWeight0 << ", " << std::setw(10) << compMolWeight  << ", ";
         std::cout << std::setw(10) << compMolWeightV << ", " << std::setw(10) << compMolWeightL << ", " << std::setw(10) << testMW << "\n";
      }

      double testBuf[ComponentId::NUMBER_OF_SPECIES + 2];
      
      for( int o = 0; o < ComponentId::NUMBER_OF_SPECIES; ++o )
      {
         testBuf[o]  = compMasses[o];
      }
      testBuf[ComponentId::NUMBER_OF_SPECIES]      = 131;
      testBuf[ComponentId::NUMBER_OF_SPECIES + 1]  = 231;
     
      std::cout << "\nSize = " << sizeof(compMasses) / sizeof(double) << ", " << ComponentId::NUMBER_OF_SPECIES << std::endl;

      double gorm1 = pvtFlash::EosPack::getInstance ().gorm(  compMasses );
      std::cout << "Gorm with normal buffer = " << gorm1 << std::endl;
      
      gorm1 = pvtFlash::EosPack::getInstance ().gorm( testBuf );
      std::cout << "Gorm with bigger buffer = " << gorm1 << std::endl;
   }  

   return 0;
}

int main (int nArg, char *pszArgs[] )
{

   double pressure;             //[Pa]               
   double temperature;          //[K]=[Celsius+273.15]
   double gorm = -1;
   double compMasses[ComponentId::NUMBER_OF_SPECIES];

   ParseArgs( nArg, pszArgs );
   ReadComposition( compMasses, pressure, temperature, gorm );

   if ( !calcGorm && !calcProp && !calcPhases )
   {
      CalcAll( compMasses, pressure, temperature, gorm );
   }
   else
   {
      double phaseCompMasses[PhaseId::NUMBER_OF_PHASES][ComponentId::NUMBER_OF_SPECIES];
      double phaseDensity   [PhaseId::NUMBER_OF_PHASES];
      double phaseViscosity [PhaseId::NUMBER_OF_PHASES];

      if ( calcGorm )
      {
         std::vector<double> compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );
         double gorm = pvtFlash::gorm( compMassesVec );
         std::cout << "Gorm = " << gorm << ";\n";
      }

      if ( calcProp )
      {
         if ( pvtFlash::EosPack::getInstance().computeWithLumping ( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, 
                                                                    phaseViscosity, (gorm < 0 ? false : true), gorm ) )
         {
            //std::cout.setf( std::ios_base::fixed, std::ios_base::floatfield );
            std::cout << "DensityLiq = " << phaseDensity[1] << "\n;";
            std::cout << "DensityVap = " << phaseDensity[0] << "\n;";

            std::cout << "ViscosityLiq = " << phaseViscosity[1] << "\n;";
            std::cout << "ViscosityVap = " << phaseViscosity[0] << "\n;";
         }
      }

      if ( calcPhases )
      {
         if ( pvtFlash::EosPack::getInstance().computeWithLumping ( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, 
                                                                    (gorm < 0 ? false : true), gorm ) )
         {
            double liqPhaseMass = 0;
            double vapPhaseMass = 0;

            for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
            {
               liqPhaseMass += phaseCompMasses[1][i];
               vapPhaseMass += phaseCompMasses[0][i];
            }
         
            std::cout << "PhaseMassLig = " << liqPhaseMass << ";\n";
            std::cout << "PhaseMassVap = " << vapPhaseMass << ";\n";
            std::cout << "PhaseMassFracLiq = " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";
            std::cout << "PhaseMassFracVap = " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";

            std::vector<double> compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );
            double gorm = pvtFlash::gorm( compMassesVec );

            liqPhaseMass = 0;
            vapPhaseMass = 0;

            for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
            {
               double molW = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm );
               liqPhaseMass += phaseCompMasses[1][i]/molW;
               vapPhaseMass += phaseCompMasses[0][i]/molW;
            }
            std::cout << "MolePhaseFracLiq = " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";
            std::cout << "MolePhaseFracVap = " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";
         }
      }
   }

   return 0;
}
