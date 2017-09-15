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
using namespace std;


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

         cout << "Read" << endl;

         char c;
         ifc >> c;
         if ( c == 'g' )
         {
            cin >> gorm;
         }
         return;
      }
   }
   
   cin >> pressure >> temperature;

   for ( int i = ComponentId::NUMBER_OF_SPECIES - 1; i >= 0; --i ) cin >> compMasses[i];

   cout << "Read" << endl;
   char c;
   cin >> c;
   if (c == 'g')
   {
      cin >> gorm;
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

   cout.precision (8);

   double totMass = 0;
   for (int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i)
   {
      cout << "mass " << setw(12) << theComponentManager.getSpeciesName (i) << ": " << setw(14) << compMasses[i] << "\n";
      totMass += compMasses[i];
   }

   cout << "\nTotal mass:    " << totMass << " [kg]\n";
   cout << "Temperature:   " << temperature << " [K]\n";
   cout << "Pressure:      " << pressure << " [Pa]\n\n";

   if ( pvtFlash::EosPack::getInstance().computeWithLumping ( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm ) )
   {
      //cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      cout << "results of run 1:\n\n";
      cout << "OilDensity: " << phaseDensity[1] << "\n";
      cout << "GasDensity: " << phaseDensity[0] << "\n\n";

      double liqPhaseMass = 0;
      double vapPhaseMass = 0;

      for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
      {
         liqPhaseMass += phaseCompMasses[1][i];
         vapPhaseMass += phaseCompMasses[0][i];
      }
      
      cout << "Liquid phase mass: " << liqPhaseMass << "\n";
      cout << "Vapour phase mass: " << vapPhaseMass << "\n\n";

      cout << "Liquid mass phase fraction: " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n";
      cout << "Vapour mass phase fraction: " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n\n";

      vector<double>compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );

      double gorm = pvtFlash::gorm(compMassesVec);
      cout << "Gorm = " << gorm << "\n\n";

      liqPhaseMass = 0;
      vapPhaseMass = 0;

      for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
      {
         double molW = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm );
         liqPhaseMass += phaseCompMasses[1][i]/molW;
         vapPhaseMass += phaseCompMasses[0][i]/molW;
      }
      cout << "Liquid mole phase fraction: " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n";
      cout << "Vapour mole phase fraction: " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n\n";
   }

   for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES_TO_FLASH; ++i )
   {
      cout << "mass " << setw(12) << theComponentManager.getSpeciesName (i) << " [gasphase]: " << setw(12) << phaseCompMasses[0][i] << " --- "
           << "mass " << setw(12) << theComponentManager.getSpeciesName (i) << " [oilphase]: " << setw(12) << phaseCompMasses[1][i] << "\n";
   }


   if ( pvtFlash::EosPack::getInstance().computeWithLumping( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm ) )
   {
      //cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      cout.precision (8);
      cout << "\nresults of run 2:" << "\n";
      cout << "OilDensity: " << phaseDensity[1] << "\n";
      cout << "GasDensity: " << phaseDensity[0] << "\n\n";

      
      vector<double> compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );

      double compMolWeight0, compMolWeight, compMolWeightL, compMolWeightV;

      double gorm_unlumped = pvtFlash::gorm( compMassesVec );
      cout << "Gorm unlumped = " << gorm_unlumped << "\n\n";
      
      vector<double> outWeights( ComponentId::NUMBER_OF_SPECIES_TO_FLASH );
      double unlumpFractions[ComponentId::NUMBER_OF_SPECIES];

      pvtFlash::EosPack::getInstance ().lumpComponents(compMasses, (double *)(&outWeights[0]), unlumpFractions);
 
      //    double gorm_lumped = pvtFlash::gorm(outWeights);
      //  cout << "Gorm lumped = " << gorm_lumped  << endl;
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

         cout << "Mol weight(0, unlumpgorm, lumpgorm, testgorm): " << setw(12) << theComponentManager.getSpeciesName(i) << " (" << setw(2) << i << ") = ";

         cout << setw(10) << compMolWeight0 << ", " << setw(10) << compMolWeight  << ", ";
         cout << setw(10) << compMolWeightV << ", " << setw(10) << compMolWeightL << ", " << setw(10) << testMW << "\n";
      }

      double testBuf[ComponentId::NUMBER_OF_SPECIES + 2];
      
      for( int o = 0; o < ComponentId::NUMBER_OF_SPECIES; ++o )
      {
         testBuf[o]  = compMasses[o];
      }
      testBuf[ComponentId::NUMBER_OF_SPECIES]      = 131;
      testBuf[ComponentId::NUMBER_OF_SPECIES + 1]  = 231;
     
      cout << "\nSize = " << sizeof(compMasses) / sizeof(double) << ", " << ComponentId::NUMBER_OF_SPECIES << endl;

      double gorm1 = pvtFlash::EosPack::getInstance ().gorm(  compMasses );
      cout << "Gorm with normal buffer = " << gorm1 << endl;
      
      gorm1 = pvtFlash::EosPack::getInstance ().gorm( testBuf );
      cout << "Gorm with bigger buffer = " << gorm1 << endl; 
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
         vector<double> compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );
         double gorm = pvtFlash::gorm( compMassesVec );
         cout << "Gorm = " << gorm << ";\n";
      }

      if ( calcProp )
      {
         if ( pvtFlash::EosPack::getInstance().computeWithLumping ( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, 
                                                                    phaseViscosity, (gorm < 0 ? false : true), gorm ) )
         {
            //cout.setf( std::ios_base::fixed, std::ios_base::floatfield );
            cout << "DensityLiq = " << phaseDensity[1] << "\n;";
            cout << "DensityVap = " << phaseDensity[0] << "\n;";

            cout << "ViscosityLiq = " << phaseViscosity[1] << "\n;";
            cout << "ViscosityVap = " << phaseViscosity[0] << "\n;";
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
         
            cout << "PhaseMassLig = " << liqPhaseMass << ";\n";
            cout << "PhaseMassVap = " << vapPhaseMass << ";\n";
            cout << "PhaseMassFracLiq = " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";
            cout << "PhaseMassFracVap = " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";

            vector<double> compMassesVec( compMasses, compMasses + ComponentId::NUMBER_OF_SPECIES );
            double gorm = pvtFlash::gorm( compMassesVec );

            liqPhaseMass = 0;
            vapPhaseMass = 0;

            for ( int i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i )
            {
               double molW = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm );
               liqPhaseMass += phaseCompMasses[1][i]/molW;
               vapPhaseMass += phaseCompMasses[0][i]/molW;
            }
            cout << "MolePhaseFracLiq = " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";
            cout << "MolePhaseFracVap = " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << ";\n";
         }
      }
   }

   return 0;
}
