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

int main (int nArg, char *pszArgs[])
{
   CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
   const int NUM_COMP     = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
   const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;

   double pressure;             //[Pa]               
   double temperature;          //[K]=[Celsius+273.15]
   double compMasses[NUM_COMP_TOT];

   double phaseCompMasses[pvtFlash::N_PHASES][NUM_COMP_TOT];
   double phaseDensity[pvtFlash::N_PHASES];
   double phaseViscosity[pvtFlash::N_PHASES];

   cin >> pressure >> temperature;

   int i;

   for (i = NUM_COMP_TOT - 1; i >= 0; --i)
      cin >> compMasses[i];

   cout << "Read" << endl;

   // possibly read gorm
   double gorm;
   bool isGormPrescribed;
   char c;

   cin >> c;
   if (c == 'g')
   {
      cin >> gorm;
      isGormPrescribed = true;
   }
   else
      isGormPrescribed = false;

   //Genex5::Genex5Framework& genex5 = Genex5::Genex5Framework::getInstance();
   cout.precision (8);

   double totMass = 0;
   for (i = 0; i < NUM_COMP_TOT; ++i)
   {
      //cout << "mass " << genex5.GetSpeciesName(i) << ": " << compMasses[i] << endl;
      cout << "mass " << theComponentManager.GetSpeciesName (i) << ": " << compMasses[i] << endl;
      totMass += compMasses[i];
   }

   cout << "Total mass: " << totMass << endl;

   if (pvtFlash::EosPack::getInstance ().computeWithLumping (temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm))
   {
      //cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      cout << "results of run 1:" << endl;
      cout << "OilDensity: " << phaseDensity[1] << " GasDensity: " << phaseDensity[0] << endl;

      double liqPhaseMass = 0;
      double vapPhaseMass = 0;
      for ( i = 0; i < NUM_COMP_TOT; ++i )
      {
         liqPhaseMass += phaseCompMasses[1][i];
         vapPhaseMass += phaseCompMasses[0][i];
      }
      cout << "Liquid phase mass: " << liqPhaseMass << ", Vapour phase masss: " << vapPhaseMass << endl;
   }

   for (i = 0; i < NUM_COMP; ++i)
   {
      //cout << "mass " << genex5.GetSpeciesName(i) << " [gasphase]: " << phaseCompMasses[0][i] << " --- "
      //    << "mass " << genex5.GetSpeciesName(i) << " [oilphase]: " << phaseCompMasses[1][i] <<  endl;

      cout << "mass " << theComponentManager.GetSpeciesName (i) << " [gasphase]: " << phaseCompMasses[0][i] << " --- "
           << "mass " << theComponentManager.GetSpeciesName (i) << " [oilphase]: " << phaseCompMasses[1][i] << endl;
   }


   if (pvtFlash::EosPack::getInstance ().computeWithLumping (temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm))
   {
      //cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      cout.precision (8);
      cout << "results of run 2:" << endl;
      cout << "OilDensity: " << phaseDensity[1] << " GasDensity: " << phaseDensity[0] << endl;

      
      vector<double>compMassesVec(NUM_COMP_TOT);
      for(int o = 0; o <NUM_COMP_TOT; ++ o ) compMassesVec[o] = compMasses[o];

      double compMolWeight0, compMolWeight, compMolWeightL, compMolWeightV;

      double gorm_unlumped = pvtFlash::gorm(compMassesVec);
      cout << "Gorm unlumped = " << gorm_unlumped << endl;
      
      vector<double>outWeights (NUM_COMP);
      double unlumpFractions[NUM_COMP_TOT];

      pvtFlash::EosPack::getInstance ().lumpComponents(compMasses, (double *)(&outWeights[0]), unlumpFractions);
 
      //    double gorm_lumped = pvtFlash::gorm(outWeights);
      //  cout << "Gorm lumped = " << gorm_lumped  << endl;
      double gorm_lumped =  gorm_unlumped;
      for (i = 0; i < NUM_COMP_TOT; ++i)
      {
         compMolWeight0 = compMolWeight = compMolWeightV = 0;
         if( i < NUM_COMP) {
            compMolWeight0  = pvtFlash::EosPack::getInstance ().getMolWeight( i, 0 );
            compMolWeight   = pvtFlash::EosPack::getInstance ().getMolWeight( i, gorm_unlumped );
         } else {
            compMolWeight0  = pvtFlash::EosPack::getInstance ().getMolWeightLumped( i, 0 );
            compMolWeight   = pvtFlash::EosPack::getInstance ().getMolWeightLumped( i, gorm_unlumped );
        }
         compMolWeightL = pvtFlash::EosPack::getInstance ().getMolWeightLumped( i, gorm_lumped);
         double testGorm = 2.3, testMW;
         testMW = pvtFlash::EosPack::getInstance ().getMolWeightLumped( i, testGorm);
         cout << "Mol weight(0, unlumpgorm, lumpgorm, testgorm):  " << theComponentManager.GetSpeciesName (i) << " (" << i << ") = " << compMolWeight0 << ", " <<  
            compMolWeight <<  ", " << compMolWeightV <<  ", " << compMolWeightL << ", " << testMW << endl;
      }

      double testBuf[NUM_COMP_TOT + 2];
      double testBuf1[NUM_COMP];
      
      for(int o = 0; o < NUM_COMP_TOT; ++ o ) {
         testBuf[o]  = compMasses[o];
      }
      testBuf[NUM_COMP_TOT]  = 131;
      testBuf[NUM_COMP_TOT + 1]  = 231;
 
      for(int o = 0; o < NUM_COMP; ++ o ) {
         testBuf1[o] = compMasses[o];
      }
     
      cout << "Size = " << sizeof(compMasses) / sizeof(double) << ", " << NUM_COMP_TOT << endl;
      double gorm1 = pvtFlash::EosPack::getInstance ().gorm(  compMasses );
      cout << "Gorm with normal buffer = " << gorm1 << endl;
      
      gorm1 = pvtFlash::EosPack::getInstance ().gorm( testBuf );
      cout << "Gorm with bigger buffer = " << gorm1 << endl;
      
      gorm1 = pvtFlash::EosPack::getInstance ().gorm( testBuf1 );
      cout << "Gorm with smaller buffer = " << gorm1 << endl;
   }  
#if 0
   if (pvtFlash::EosPack::getInstance ().computeWithLumping (temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm))
   {
      //cout.setf( std::ios_base::fixed, std::ios_base::floatfield );

      cout.precision (8);
      cout << "results of run 3:" << endl;
      cout << "OilDensity: " << phaseDensity[1] << " GasDensity: " << phaseDensity[0] << endl;

      double compMolWeight;
      for (i = 0; i < NUM_COMP_TOT; ++i)
      {
         
         if( i < NUM_COMP) {
            compMolWeight = pvtFlash::EosPack::getInstance ().getMolWeight( i, 0 );
            cout << "Mol weight  " << theComponentManager.GetSpeciesName (i) << " ( " << i << " ) = " << compMolWeight << endl;
         }
         compMolWeight = pvtFlash::getMolWeightWithLumping( i, compMasses );
         cout << "Mol weight  with lump  " << theComponentManager.GetSpeciesName (i) << " ( " << i << " ) = " << compMolWeight << endl;
      }
   }
#endif

   return 0;
}

