#ifdef sgi
#ifdef _STANDARD_C_PLUS_PLUS
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;
#else                           // !_STANDARD_C_PLUS_PLUS
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include<strstream.h>
typedef strstream ostringstream;
typedef istrstream istringstream;
#endif                          // _STANDARD_C_PLUS_PLUS
#else                           // !sgi
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;
#endif                          // sgi

#include "EosPack.h"

void ReadComposition( int argc, char* argv[], double compMasses[CBMGenerics::ComponentManager::NumberOfOutputSpecies], double & pressure, double & temperature, double & gorm )
{
   const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;

   if ( argc > 1 )
   {
      std::ifstream ifc( argv[1], std::ifstream::in );

      if ( ifc.good() )
      {
         ifc >> pressure >> temperature;

         for ( int i = NUM_COMP_TOT - 1; i >= 0; --i ) ifc >> compMasses[i];

         cout << "Read" << endl;

         char c;
         ifc >> c;
         if ( c == 'g' )
         {
            cin >> gorm;
         }
      }
      return;
   }
   
   cin >> pressure >> temperature;

   for ( int i = NUM_COMP_TOT - 1; i >= 0; --i ) cin >> compMasses[i];

   cout << "Read" << endl;
   char c;
   cin >> c;
   if (c == 'g')
   {
      cin >> gorm;
   }
}

int main (int nArg, char *pszArgs[])
{
   CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
   const int NUM_COMP     = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
   const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;

   double pressure;             //[Pa]               
   double temperature;          //[K]=[Celsius+273.15]
   double compMasses[NUM_COMP_TOT];
   double gorm = -1;
   bool   isGormPrescribed = false;

   double phaseCompMasses[pvtFlash::N_PHASES][NUM_COMP_TOT];
   double phaseDensity[pvtFlash::N_PHASES];
   double phaseViscosity[pvtFlash::N_PHASES];

   ReadComposition( nArg, pszArgs, compMasses, pressure, temperature, gorm );
   isGormPrescribed = gorm < 0 ? false : true;

   cout.precision (8);

   double totMass = 0;
   for (int i = 0; i < NUM_COMP_TOT; ++i)
   {
      cout << "mass " << setw(12) << theComponentManager.GetSpeciesName (i) << ": " << setw(14) << compMasses[i] << "\n";
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

      for ( int i = 0; i < NUM_COMP_TOT; ++i )
      {
         liqPhaseMass += phaseCompMasses[1][i];
         vapPhaseMass += phaseCompMasses[0][i];
      }
      
      cout << "Liquid phase mass: " << liqPhaseMass << "\n";
      cout << "Vapour phase mass: " << vapPhaseMass << "\n\n";

      cout << "Liquid mass phase fraction: " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n";
      cout << "Vapour mass phase fraction: " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n\n";

      vector<double>compMassesVec( compMasses, compMasses + NUM_COMP_TOT );

      double gorm = pvtFlash::gorm(compMassesVec);
      cout << "Gorm = " << gorm << "\n\n";

      liqPhaseMass = 0;
      vapPhaseMass = 0;

      for ( int i = 0; i < NUM_COMP_TOT; ++i )
      {
         double molW = pvtFlash::EosPack::getInstance().getMolWeightLumped( i, gorm );
         liqPhaseMass += phaseCompMasses[1][i]/molW;
         vapPhaseMass += phaseCompMasses[0][i]/molW;
      }
      cout << "Liquid mole phase fraction: " << liqPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n";
      cout << "Vapour mole phase fraction: " << vapPhaseMass/(liqPhaseMass + vapPhaseMass) << "\n\n";
   }

   for ( int i = 0; i < NUM_COMP; ++i )
   {
      cout << "mass " << setw(12) << theComponentManager.GetSpeciesName (i) << " [gasphase]: " << setw(12) << phaseCompMasses[0][i] << " --- "
           << "mass " << setw(12) << theComponentManager.GetSpeciesName (i) << " [oilphase]: " << setw(12) << phaseCompMasses[1][i] << "\n";
   }


   if ( pvtFlash::EosPack::getInstance().computeWithLumping( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm ) )
   {
      //cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      cout.precision (8);
      cout << "\nresults of run 2:" << "\n";
      cout << "OilDensity: " << phaseDensity[1] << "\n";
      cout << "GasDensity: " << phaseDensity[0] << "\n\n";

      
      vector<double> compMassesVec( compMasses, compMasses + NUM_COMP_TOT );

      double compMolWeight0, compMolWeight, compMolWeightL, compMolWeightV;

      double gorm_unlumped = pvtFlash::gorm( compMassesVec );
      cout << "Gorm unlumped = " << gorm_unlumped << "\n\n";
      
      vector<double> outWeights(NUM_COMP);
      double unlumpFractions[NUM_COMP_TOT];

      pvtFlash::EosPack::getInstance ().lumpComponents(compMasses, (double *)(&outWeights[0]), unlumpFractions);
 
      //    double gorm_lumped = pvtFlash::gorm(outWeights);
      //  cout << "Gorm lumped = " << gorm_lumped  << endl;
      double gorm_lumped =  gorm_unlumped;
      for (int i = 0; i < NUM_COMP_TOT; ++i )
      {
         compMolWeight0 = compMolWeight = compMolWeightV = 0;
         if( i < NUM_COMP )
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

         cout << "Mol weight(0, unlumpgorm, lumpgorm, testgorm): " << setw(12) << theComponentManager.GetSpeciesName(i) << " (" << setw(2) << i << ") = ";

         cout << setw(10) << compMolWeight0 << ", " << setw(10) << compMolWeight  << ", ";
         cout << setw(10) << compMolWeightV << ", " << setw(10) << compMolWeightL << ", " << setw(10) << testMW << "\n";
      }

      double testBuf[NUM_COMP_TOT + 2];
      
      for( int o = 0; o < NUM_COMP_TOT; ++o )
      {
         testBuf[o]  = compMasses[o];
      }
      testBuf[NUM_COMP_TOT]      = 131;
      testBuf[NUM_COMP_TOT + 1]  = 231;
     
      cout << "\nSize = " << sizeof(compMasses) / sizeof(double) << ", " << NUM_COMP_TOT << endl;

      double gorm1 = pvtFlash::EosPack::getInstance ().gorm(  compMasses );
      cout << "Gorm with normal buffer = " << gorm1 << endl;
      
      gorm1 = pvtFlash::EosPack::getInstance ().gorm( testBuf );
      cout << "Gorm with bigger buffer = " << gorm1 << endl; 
   }  

   return 0;
}

