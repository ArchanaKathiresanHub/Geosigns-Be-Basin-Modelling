#include "stdafx.h"

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
      #include <fstream>
      #include <iomanip>
      #include <sstream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include <iostream.h>
      #include <fstream.h>
      #include <iomanip.h>
      #include<strstream.h>
      typedef strstream ostringstream;
      typedef istrstream istringstream;
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <fstream>
   #include <iomanip>
   #include <sstream>
   using namespace std;
#endif // sgi

void testPolynomialParse();

/*
// Program for testing Cauldron link
*/
#include <math.h>

#include <stdlib.h>

#include "Parse.h" 

#include <limits>

#include "EosCauldron.h"
#include "EosPack.h"
#include "DllExport.h" 

#include <assert.h>

using namespace std;


string pvtFlash::pvtPropertiesConfigFile;



pvtFlash::EosPack::EosPack()
{
#ifdef DEBUG_EXTENSIVE
   cout << "calling EosPack-constructor" << endl;
#endif

   try
   {
      m_isReadInOk=true;
      m_propertyFunc=0;		
      m_omegaA=0; 
      m_omegaB=0; 
      m_corrLBC=0;
      
      //Genex5::Genex5Framework& TheFramework = Genex5::Genex5Framework::getInstance();
      CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
      const int NUM_COMP = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;

	  if( pvtFlash::pvtPropertiesConfigFile == "" )
	  {
		char* eosPackDir;
		if (!(eosPackDir = getenv ("MYEOSPACKDIR")))
		{
			eosPackDir = getenv ("EOSPACKDIR");
		}		
		if (!eosPackDir)
		{
			throw string ("Environment Variable EOSPACKDIR not set.");
		}
		// read config-file tables
		string fileName("PVT_properties.cfg");	
		pvtFlash::pvtPropertiesConfigFile = string(string(eosPackDir) + string("/") +  fileName);
	  }

      // read config-file tables
      ifstream ifs(pvtFlash::pvtPropertiesConfigFile.c_str());      
      if ( ! ifs ) throw string( pvtPropertiesConfigFile + " could not be opened.");

      fileIO::Table compPropTable(ifs);	
		
      string tableNameWanted = "PVT-Component-Properties";
      
      if ( ( ! compPropTable.isReadSuccessful() ) ||
           (   compPropTable.getTableName().find(tableNameWanted, 0) == string::npos) )
      {
         throw string(tableNameWanted + " table expected. Aborting...");
      }
     
      fileIO::Table generalPropTable;
      //concatenate as many "PVT-Component-Properties"-tables as available, last table contains general properties
      while (true)
      {
         fileIO::Table table(ifs);
         if ( table.getTableName().find(tableNameWanted, 0) != string::npos) 
         {
            compPropTable.concatTable(table);
         }
         else //last table: general data
         {
            generalPropTable = table;
            break;
         }
      } 
      
#ifdef DEBUG_EXTENSIVE
      cout << "component table:" << compPropTable << endl<< endl;
      cout << "general table:" << endl << generalPropTable << endl << endl;
#endif
      


      // parse component based data
      
#ifdef DEBUG_EXTENSIVE
      ofstream ofs("myout.txt");
      ofs << compPropTable;
      ofs << endl << endl;
#endif
      
      m_propertyFunc = new polynomials::PiecewisePolynomial*[NUM_COMP];
      int nCompProp = 6, iComp;
      for (iComp = 0; iComp < NUM_COMP; ++iComp) m_propertyFunc[iComp]= new polynomials::PiecewisePolynomial[nCompProp];
      
      int i;
      for (i=0;i<NUM_COMP;++i)
      {
         //int indexFramework = TheFramework.GetSpeciedIdByName( compPropTable.getEntry(i,0) ) ;
         int indexFramework = theComponentManager.GetSpeciedIdByName( compPropTable.getEntry(i,0) ) ;
         if (indexFramework < 0) {
            string s = "Bad Name: " + compPropTable.getEntry(i,0);
            throw s;
         }

         for (int iProp=0; iProp < nCompProp; ++iProp)			
         {	
            const string& func = compPropTable.getEntry(i,iProp+1);

            using namespace polynomials;

            vector<polynomials::parse::Token> tokens;

            polynomials::parse::GetTokens(func, tokens);
            
            size_t start=0;
            polynomials::parse::ParsePiecewisePolynomial(tokens, 
                                                         start, 
                                                         m_propertyFunc[indexFramework][iProp] ); 
         }
      }

#ifdef DEBUG_EXTENSIVE			
      for (i=0;i<NUM_COMP;++i)
      {	
         //test output: are critical temperatures read correctly?
         using namespace polynomials;
         //cout 	<< "parsed Property[" << TheFramework.GetSpeciesName(i) << "] = " 
         cout 	<< "parsed Property[" << theComponentManager.GetSpeciesName(i) << "] = " 
                << m_propertyFunc[i][5](0.0) << " Index for re-ordering: " 
            
            //<< TheFramework.GetSpeciedIdByName( table1.getEntry(i,0) )  << endl;
                << theComponentManager.GetSpeciedIdByName( table1.getEntry(i,0) )  << endl;			
      }
#endif

      //parse general data
      string EquOfState =  generalPropTable.getEntry(0,0);
      if ( EquOfState.find("SRK", 0) !=std::string::npos)
         isRK = 1;
      else
         isRK = 0;
      
      m_omegaA = new polynomials::PiecewisePolynomial;
      m_omegaB = new polynomials::PiecewisePolynomial;
      m_corrLBC = new polynomials::PiecewisePolynomial[5];
      
      
      using namespace polynomials;


      vector<polynomials::parse::Token> tokens;
      polynomials::parse::GetTokens(generalPropTable.getEntry(0,1).c_str(), tokens);
      size_t start=0;
      polynomials::parse::ParsePiecewisePolynomial(tokens, 
                                                   start, 
                                                   *m_omegaA); 

      tokens.clear();
      polynomials::parse::GetTokens(generalPropTable.getEntry(0,2).c_str(), tokens);
      start=0;
      polynomials::parse::ParsePiecewisePolynomial(tokens, 
                                                   start, 
                                                   *m_omegaB); 

      for ( i=0; i < 5; ++i )
      {
         tokens.clear();
         polynomials::parse::GetTokens(generalPropTable.getEntry(0,3 + i).c_str(), tokens);
         start=0;
         polynomials::parse::ParsePiecewisePolynomial(tokens, 
                                                      start, 
                                                      m_corrLBC[i]); 
         
      }

      for( i = 0; i < LSC; ++ i ) lumpedSpeciesIndex[i] = i;

      lumpedSpeciesIndex[LSC]      = lumpedSpeciesIndex[C15_SAT];
      lumpedSpeciesIndex[C15_AT]   = lumpedSpeciesIndex[C15_SAT];
      lumpedSpeciesIndex[C15_AROS] = lumpedSpeciesIndex[C15_SAT];
      lumpedSpeciesIndex[C15_SATS] = lumpedSpeciesIndex[C15_SAT];

      lumpedSpeciesIndex[C6_14BT]   = lumpedSpeciesIndex[C6_14ARO];
      lumpedSpeciesIndex[C6_14DBT]  = lumpedSpeciesIndex[C6_14ARO];
      lumpedSpeciesIndex[C6_14BP]   = lumpedSpeciesIndex[C6_14ARO];
      lumpedSpeciesIndex[C6_14SATS] = lumpedSpeciesIndex[C6_14ARO];
      lumpedSpeciesIndex[C6_14AROS] = lumpedSpeciesIndex[C6_14ARO];

      
#ifdef DEBUG_EXTENSIVE			
      //output for generating function values
      //cout.setf(std::ios_base::scientific, std::ios_base::floatfield);
      cout.precision(15); 
      cout <<"VolShift C15+Sat (1.242) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Sat")][3](1.242) << endl;
      cout <<"VolShift C6-14Sat (1.242) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Sat")][3](1.242) << endl;
      cout <<"VolShift C6-14Aro (1.242) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Aro")][3](1.242) << endl;

      cout << endl << "CritPressure" << endl;
      
      cout <<"Pc resins      (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("resins")][4](2.4) << endl;
      cout <<"Pc resins      (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("resins")][4](2.5) << endl;
      cout <<"Pc C15+Sat   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Sat")][4](2.4) << endl;
      cout <<"Pc C15+Sat   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Sat")][4](2.5) << endl;
      cout <<"Pc C6-14Sat   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Sat")][4](2.4) << endl;
      cout <<"Pc C6-14Sat   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Sat")][4](2.5) << endl;
      cout <<"Pc asphaltenes (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("asphaltenes")][4](2.4) << endl;
      cout <<"Pc asphaltenes (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("asphaltenes")][4](2.5) << endl;
      cout <<"Pc C15+Aro   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Aro")][4](2.4) << endl;
      cout <<"Pc C15+Aro   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Aro")][4](2.5) << endl;
      cout <<"Pc C6-14Aro   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Aro")][4](2.4) << endl;
      cout <<"Pc C6-14Aro   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Aro")][4](2.5) << endl;
		
      cout << endl << "CritTemperature" << endl;

      cout <<"Tc resins      (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("resins")][5](2.4) << endl;
      cout <<"Tc resins      (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("resins")][5](2.5) << endl;
      cout <<"Tc C15+Sat   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Sat")][5](2.4) << endl;
      cout <<"Tc C15+Sat   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Sat")][5](2.5) << endl;
      cout <<"Tc C6-14Sat   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Sat")][5](2.4) << endl;
      cout <<"Tc C6-14Sat   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Sat")][5](2.5) << endl;
      cout <<"Tc asphaltenes (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("asphaltenes")][5](2.4) << endl;
      cout <<"Tc asphaltenes (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("asphaltenes")][5](2.5) << endl;
      cout <<"Tc C15+Aro   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Aro")][5](2.4) << endl;
      cout <<"Tc C15+Aro   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C15+Aro")][5](2.5) << endl;
      cout <<"Tc C6-14Aro   (2.4) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Aro")][5](2.4) << endl;
      cout <<"Tc C6-14Aro   (2.5) = " << m_propertyFunc[TheFramework.GetSpeciedIdByName("C6-14Aro")][5](2.5) << endl;
#endif
      
   }	
   catch(polynomials::error::SyntaxError& s)
   {
      m_isReadInOk=false;
      cerr << "Syntax Error in EosPack: " << s.info << endl;  
      throw s;
   }
   catch(polynomials::error::NumericError& n)
   {
      m_isReadInOk=false;
      cerr << "Numeric Error in EosPack: " << n.info << endl;  
      throw n;
   }
   catch(string& s)
   {
      m_isReadInOk=false;
      cerr << "Error in EosPack: " << s << endl;  
      throw s;
   }
   catch(...)
   {
      m_isReadInOk=false;
      cerr << "Error: unhandled exception in EosPack." << endl;  
      throw m_isReadInOk;
   }


}

pvtFlash::EosPack::~EosPack()
{
   //const int NUM_COMP = Genex5::Genex5Framework::NumberOfOutputSpecies;
   //const int NUM_COMP = CBMGenerics::ComponentManager::NumberOfOutputSpecies;
   const int NUM_COMP = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
   if (m_propertyFunc)
   {
      for (int iComp = 0; iComp < NUM_COMP; ++iComp) delete[] m_propertyFunc[iComp];
      delete[] m_propertyFunc;
   }
   
   if (m_omegaA)
      delete m_omegaA;
   
   if (m_omegaB)
      delete m_omegaB;
   
   if (m_omegaB)
      delete[] m_corrLBC;
}	


pvtFlash::EosPack&  pvtFlash::EosPack::getInstance()
{
   static EosPack theEosCalculator;
   return theEosCalculator;
}

int pvtFlash::EosPack::getLumpedIndex( int ind ) const
{
   return lumpedSpeciesIndex[ind];
}

void pvtFlash::EosPack::getLumpingFractions(const vector<double>& inWeights, double unlumpFractions[])
{
   const int NUM_COMP  = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; 
   double outWeights [NUM_COMP];

   pvtFlash::EosPack::lumpComponents((double *)(&inWeights[0]), outWeights, unlumpFractions);
}

void pvtFlash::EosPack::lumpComponents( const double in_compMasses[], double out_compMasses[], double unlump_fraction[] ) 
{
   int i;
   double invMass;

   CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
   const int NUM_COMP     = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; 
   const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;
 
   for(i = 0; i < NUM_COMP; ++ i) { out_compMasses[i] = in_compMasses[i]; }

   for(i = 0; i < NUM_COMP_TOT; ++ i) { unlump_fraction[i] = 1;  }

   // Lump LSC, C15+AT, C15+AroS, C15+SatS to C15+Sat
   int compIndex[5];
   compIndex[0] = LSC;      //theComponentManager.GetSpeciedIdByName("LSC");
   compIndex[1] = C15_AT;   //theComponentManager.GetSpeciedIdByName("C15+AT");
   compIndex[2] = C15_AROS; //theComponentManager.GetSpeciedIdByName("C15+AroS");
   compIndex[3] = C15_SATS; //theComponentManager.GetSpeciedIdByName("C15+SatS");

   int lump_component_ind = C15_SAT; //theComponentManager.GetSpeciedIdByName("C15+Sat");

#ifdef DEBUG_EXTENSIVE1			
   cout << "Lumped to "<<  lump_component_ind << " : " << compIndex[0] << ", " <<  compIndex[1] << ", " << compIndex[2] << ", "<< compIndex[3] << ", " << endl;
#endif
   
   for(i = 0; i < 4; ++ i) { 
      out_compMasses[lump_component_ind] += in_compMasses[compIndex[i]]; 
   }
   if( out_compMasses[lump_component_ind] != 0 ){
      invMass = 1 / out_compMasses[lump_component_ind];

      for(i = 0; i < 4; ++ i) { 
         unlump_fraction[compIndex[i]] = in_compMasses[compIndex[i]] * invMass; 
      }
      unlump_fraction[lump_component_ind] = in_compMasses[lump_component_ind] * invMass;
   }
   // Lump C6-14BT, C6-14DBT, C6-14BP, C6-14SatS, C6-14AroS to C16-14Aro
   
   compIndex[0] = C6_14BT;   //theComponentManager.GetSpeciedIdByName("C6-14BT");
   compIndex[1] = C6_14DBT;  //theComponentManager.GetSpeciedIdByName("C6-14DBT");
   compIndex[2] = C6_14BP;   //theComponentManager.GetSpeciedIdByName("C6-14BP");
   compIndex[3] = C6_14SATS; //theComponentManager.GetSpeciedIdByName("C6-14SatS");
   compIndex[4] = C6_14AROS; //theComponentManager.GetSpeciedIdByName("C6-14AroS");

   lump_component_ind = C6_14ARO; //theComponentManager.GetSpeciedIdByName("C6-14Aro");

#ifdef DEBUG_EXTENSIVE1			
   cout << "Lumped to "<<  lump_component_ind << " : " << compIndex[0] << ", " <<  compIndex[1] << ", " << compIndex[2] << ", "<< compIndex[3] << ", "<< compIndex[4] << endl;
#endif

   for(i = 0; i < 5; ++ i) {
      out_compMasses[lump_component_ind] += in_compMasses[compIndex[i]];
   }

   if( out_compMasses[lump_component_ind] != 0 ){
      invMass = 1 / out_compMasses[lump_component_ind];

      for(i = 0; i < 5; ++ i) {
         unlump_fraction[compIndex[i]] = in_compMasses[compIndex[i]] * invMass;
      }
      unlump_fraction[lump_component_ind] = in_compMasses[lump_component_ind] * invMass;
   }
#ifdef DEBUG_EXTENSIVE1			
   // debug check
  
   double in_totMasses = 0, out_totMasses = 0, unlump_sum = 0;
   for(i = 0; i < NUM_COMP; ++ i) {
      out_totMasses += out_compMasses[i];
   }
   cout << "Unlump fractions: " <<  endl;
   for(i = 0; i < NUM_COMP_TOT; ++ i) {
      in_totMasses += in_compMasses[i];
      unlump_sum += unlump_fraction[i];
      cout << unlump_fraction[i] << ";  ";
   }
   cout << endl;
   cout << "In masses = " << in_totMasses << " , Out masses = " << out_totMasses << ", Unlumpfraction = " <<  unlump_sum  << "." << endl;
#endif
}
void pvtFlash::EosPack::unlumpComponents( double in_paseCompMasses[][CBMGenerics::ComponentManager::NumberOfSpeciesToFlash], 
                                          double out_phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfOutputSpecies], 
                                          double unlump_fraction[] ) 
{
   CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
   const int NUM_COMP = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;

   const int iOil = 1;
   const int iGas = 0;

   for(int i = 0; i < NUM_COMP; ++ i) {
      out_phaseCompMasses[iOil][i] = in_paseCompMasses[iOil][i];
      out_phaseCompMasses[iGas][i] = in_paseCompMasses[iGas][i];
   }
 
   // Unlump LSC, C15+AT, C15+AroS, C15+SatS from C15+Sat
   int compIndex[5];
   compIndex[0] = LSC;      //theComponentManager.GetSpeciedIdByName("LSC");
   compIndex[1] = C15_AT;   //theComponentManager.GetSpeciedIdByName("C15+AT");
   compIndex[2] = C15_AROS; //theComponentManager.GetSpeciedIdByName("C15+AroS");
   compIndex[3] = C15_SATS; //theComponentManager.GetSpeciedIdByName("C15+SatS");

   int lump_component_ind = C15_SAT; //theComponentManager.GetSpeciedIdByName("C15+Sat");

   for(int i = 0; i < 4; ++ i) {
      out_phaseCompMasses[iOil][compIndex[i]] = in_paseCompMasses[iOil][lump_component_ind] * unlump_fraction[compIndex[i]];
      out_phaseCompMasses[iGas][compIndex[i]] = in_paseCompMasses[iGas][lump_component_ind] * unlump_fraction[compIndex[i]];
   }
   out_phaseCompMasses[iOil][lump_component_ind] = in_paseCompMasses[iOil][lump_component_ind] * unlump_fraction[lump_component_ind];
   out_phaseCompMasses[iGas][lump_component_ind] = in_paseCompMasses[iGas][lump_component_ind] * unlump_fraction[lump_component_ind];

#ifdef DEBUG_EXTENSIVE1			
   cout << "Unlumped from "<<  lump_component_ind << " : " << compIndex[0] << ", " <<  compIndex[1] << ", " << compIndex[2] << ", "<< compIndex[3]  << endl;
#endif

   // Unlump C6-14BT, C6-14DBT, C6-14BP, C6-14SatS, C6-14AroS from C16-14Aro
   compIndex[0] = C6_14BT;   //theComponentManager.GetSpeciedIdByName("C6-14BT");
   compIndex[1] = C6_14DBT;  //theComponentManager.GetSpeciedIdByName("C6-14DBT");
   compIndex[2] = C6_14BP;   //theComponentManager.GetSpeciedIdByName("C6-14BP");
   compIndex[3] = C6_14SATS; //theComponentManager.GetSpeciedIdByName("C6-14SatS");
   compIndex[4] = C6_14AROS; //theComponentManager.GetSpeciedIdByName("C6-14AroS");

   lump_component_ind = C6_14ARO; //theComponentManager.GetSpeciedIdByName("C6-14Aro");
   for(int i = 0; i < 5; ++ i) {
      out_phaseCompMasses[iOil][compIndex[i]] = in_paseCompMasses[iOil][lump_component_ind] * unlump_fraction[compIndex[i]];
      out_phaseCompMasses[iGas][compIndex[i]] = in_paseCompMasses[iGas][lump_component_ind] * unlump_fraction[compIndex[i]];
   }
   out_phaseCompMasses[iOil][lump_component_ind] = in_paseCompMasses[iOil][lump_component_ind] * unlump_fraction[lump_component_ind];
   out_phaseCompMasses[iGas][lump_component_ind] = in_paseCompMasses[iGas][lump_component_ind] * unlump_fraction[lump_component_ind];

#ifdef DEBUG_EXTENSIVE1			
   cout << "Unlumped from "<<  lump_component_ind << " : " << compIndex[0] << ", " <<  compIndex[1] << ", " << compIndex[2] << ", "<< compIndex[3] << ", "<< compIndex[4] << endl;

   double out_totMassesGas = 0, out_totMassesOil = 0;
   for(int i = 0; i < CBMGenerics::ComponentManager::NumberOfOutputSpecies; ++ i) {
      out_totMassesGas += out_phaseCompMasses[iGas][i];
      out_totMassesOil += out_phaseCompMasses[iOil][i];
   }
   cout << endl;
   cout << "Out masses = gas:" << out_totMassesGas << " , oil: " <<  out_totMassesOil << ", Total = " <<  out_totMassesGas+out_totMassesOil  << "." << endl;
#endif
}
bool pvtFlash::EosPack::computeWithLumping(double temperature, 
                                           double pressure, 
                                           double in_compMasses[],  				       
                                           double out_phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfOutputSpecies],
                                           double phaseDensity [],
                                           double phaseViscosity[],
                                           bool   isGormPrescribed,
                                           double gorm
                                           )   
{
   const int NUM_COMP     = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; 
   const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;
   
   double compMasses[NUM_COMP];
   double phaseCompMasses[N_PHASES][NUM_COMP];
   double unlump_fractions[NUM_COMP_TOT];

   pvtFlash::EosPack::lumpComponents(in_compMasses, compMasses, unlump_fractions);
   
   bool ret = pvtFlash::EosPack::compute( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm);

   pvtFlash::EosPack::unlumpComponents(phaseCompMasses, out_phaseCompMasses, unlump_fractions);

   return ret;
}  
bool pvtFlash::EosPack::compute(double temperature, 
                                double pressure, 
                                double compMasses[],  				       
                                double phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfSpeciesToFlash],
                                double phaseDensity [],
                                double phaseViscosity[],
                                bool   isGormPrescribed,
                                double gorm
                                )   
{  

   try
   {
      
      if (!m_isReadInOk)
         throw string("PVT configuration file not read correctly.");		
      
      /* Declarations */
      //Genex5::Genex5Framework& TheFramework = Genex5::Genex5Framework::getInstance();
      CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
      //const int NUM_COMP = Genex5::Genex5Framework::NumberOfOutputSpecies;

      const int NUM_COMP = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; //CBMGenerics::ComponentManager::NumberOfOutputSpecies;

      int iFlashes, iJ, iOil, iGas, i, iNc = NUM_COMP;
      double *pMassFraction, *pPhaseAcc, *pViscosity;
      double *pPressure, *pAccumulation, *pDensity;
      double *pTemperature, pLB[5];
      //		double dCvm;
      //    double dCvp, dCvt, dCvv;
      EosPvtTable *pvttable;

      /* Terms for equation of state */
      double* pMw = new double [ iNc * ( 8 + iNc ) ];
      double* pPc     = pMw     + iNc;
      double* pTc     = pPc     + iNc;
      double* pVc     = pTc     + iNc;
      double* pAc     = pVc     + iNc;
      double* pOmegaA = pAc     + iNc;
      double* pOmegaB = pOmegaA + iNc;
      double* pShiftC = pOmegaB + iNc;
      double* pBinary = pShiftC + iNc;


#ifdef DEBUG_EXTENSIVE			
      testPolynomialParse();
#endif		
		
      if  (! isGormPrescribed) 
      {
         

#if 0
         int gasIndex[6];

         gasIndex[0] = CBMGenerics::ComponentManager::C1;
         gasIndex[1] = CBMGenerics::ComponentManager::C2;
         gasIndex[2] = CBMGenerics::ComponentManager::C3;
         gasIndex[3] = CBMGenerics::ComponentManager::C4;
         gasIndex[4] = CBMGenerics::ComponentManager::C5;
         gasIndex[5] = CBMGenerics::ComponentManager::H2S;
#endif

#if 0
         gasIndex[0] = theComponentManager.GetSpeciedIdByName("C1");
         gasIndex[1] = theComponentManager.GetSpeciedIdByName("C2");
         gasIndex[2] = theComponentManager.GetSpeciedIdByName("C3");
         gasIndex[3] = theComponentManager.GetSpeciedIdByName("C4");
         gasIndex[4] = theComponentManager.GetSpeciedIdByName("C5");
         gasIndex[5] = theComponentManager.GetSpeciedIdByName("H2S");
#endif
         
         double gasMass = compMasses [ CBMGenerics::ComponentManager::C1 ] + 
                          compMasses [ CBMGenerics::ComponentManager::C2 ] +
                          compMasses [ CBMGenerics::ComponentManager::C3 ] +
                          compMasses [ CBMGenerics::ComponentManager::C4 ] +
                          compMasses [ CBMGenerics::ComponentManager::C5 ] +
                          compMasses [ CBMGenerics::ComponentManager::H2S ];
                        
         // for (i=0; i < 6; ++i)
         //    gasMass += compMasses[gasIndex[i]];

#if 0
         int oilIndex[6];

         oilIndex[0] = CBMGenerics::ComponentManager::resin;
         oilIndex[1] = CBMGenerics::ComponentManager::C15PlusSat;
         oilIndex[2] = CBMGenerics::ComponentManager::C6Minus14Sat;
         oilIndex[3] = CBMGenerics::ComponentManager::asphaltene;
         oilIndex[4] = CBMGenerics::ComponentManager::C15PlusAro;
         oilIndex[5] = CBMGenerics::ComponentManager::C6Minus14Aro;
#endif

#if 0
         oilIndex[0] = theComponentManager.GetSpeciedIdByName("resins");
         oilIndex[1] = theComponentManager.GetSpeciedIdByName("C15+Sat");
         oilIndex[2] = theComponentManager.GetSpeciedIdByName("C6-14Sat");
         oilIndex[3] = theComponentManager.GetSpeciedIdByName("asphaltenes");
         oilIndex[4] = theComponentManager.GetSpeciedIdByName("C15+Aro");
         oilIndex[5] = theComponentManager.GetSpeciedIdByName("C6-14Aro");
#endif

         double oilMass = compMasses [ CBMGenerics::ComponentManager::resin ] + 
                          compMasses [ CBMGenerics::ComponentManager::C15PlusSat ] +
                          compMasses [ CBMGenerics::ComponentManager::C6Minus14Sat ] +
                          compMasses [ CBMGenerics::ComponentManager::asphaltene ] +
                          compMasses [ CBMGenerics::ComponentManager::C15PlusAro ] +
                          compMasses [ CBMGenerics::ComponentManager::C6Minus14Aro ];

         // for (i=0; i < 6; ++i)
         //    oilMass += compMasses[oilIndex[i]];
         

         if ( oilMass != 0.0 ) 
            gorm = gasMass / oilMass ;
         else
            gorm = 1.0e+80; 

      }

      //fill arrays
      for (i=0;i<NUM_COMP;++i)
      {	
         
         using namespace polynomials;
         pMw[i]     =  m_propertyFunc[i][0](gorm);
         pAc[i]     =  m_propertyFunc[i][1](gorm);
         pVc[i]     =  m_propertyFunc[i][2](gorm);
         pShiftC[i] =  m_propertyFunc[i][3](gorm);
         pPc[i]     =  m_propertyFunc[i][4](gorm);
         pTc[i]     =  m_propertyFunc[i][5](gorm);
         
         pOmegaA[i] =  (*m_omegaA)(gorm);
         pOmegaB[i] =  (*m_omegaB)(gorm);

      }
      for ( i=0; i < 5; ++i )
      {
         pLB[i] = m_corrLBC[i](gorm);
         
      }

      //normalize volume shift terms
      for (i=0;i<NUM_COMP;++i)
      {   
         pShiftC[i] = pShiftC[i] * ( pPc[i] / 8314.472 ) / ( pOmegaB[i] * pTc[i] );   //consistent units
         //pShiftC[i] = pShiftC[i] * ( pPc[i] / 8.314472 ) / ( pOmegaB[i] * pTc[i] );
      }

      


      /* Binary interaction terms */
      for ( iJ = 0; iJ < iNc * iNc; iJ++ ) 
      {
         int x = iJ / iNc;
         int y = iJ % iNc;
         pBinary[iJ] = 0.0; 
         if (abs (x - y) == 1)
            pBinary[iJ] = 0.0; 
      }

      //#define EOSPACK_OUT
#ifdef EOSPACK_OUT
      ofstream eosout("EosPack.out");
      eosout << endl << endl;
      eosout << "gorm:  " << gorm << endl << endl << endl;
      

      for (i=0;i<NUM_COMP;++i)
      {	
		
         eosout << "pMw[ " << theComponentManager.GetSpeciesName(i)<< "]	 " <<  pMw[i] << endl;
         eosout << "pAc[ " << theComponentManager.GetSpeciesName(i)<< "]	 " <<  pAc[i] << endl;
         eosout << "pVc[ " << theComponentManager.GetSpeciesName(i)<< "]	 " <<  pVc[i] << endl;
         eosout << "pShiftC[ " << theComponentManager.GetSpeciesName(i)<< "]	 " <<  pShiftC[i] / (( pPc[i] / 8314.472 ) / ( pOmegaB[i] * pTc[i] ) ) << endl;
         eosout << "pPc[ " << theComponentManager.GetSpeciesName(i)<< "]	 " <<  pPc[i] << endl;
         eosout << "pTc[ " << theComponentManager.GetSpeciesName(i)<< "]	 " <<  pTc[i] << endl;
         eosout << "pOmegaA[ " << theComponentManager.GetSpeciesName(i) << "]	 " << pOmegaA[i] << endl ;
         eosout << "pOmegaB[ " << theComponentManager.GetSpeciesName(i) << "]	 " << pOmegaB[i] << endl ;
         eosout << endl <<endl;   	
         
      }
      for ( i=0; i < 5; ++i )
      {
         eosout << "pLB[ " << i<< "]	 " <<  pLB[i] <<endl;
		
      }

      eosout << "isRK " << isRK << endl;
      eosout << "iNc " << iNc << endl;
      eosout << "pBinary[ " << iNc << ", " << iNc << " ]" << endl; 
      for (i=0;i<NUM_COMP;++i)
      {	
         for (int j=0;j<NUM_COMP;++j)
         {
            eosout << pBinary[i*iNc+j] << "   ";
         }
         eosout << endl;
      }
#endif
      

      /* Construct pvt table */
      pvttable = EosCauldron::ConcoctBrew ( iNc, isRK, pMw, pPc, pTc, pVc, pAc, pOmegaA, pOmegaB, 
                                            pShiftC, pBinary, pLB );

      /* Free memory */
      delete[] pMw ;

      /* Constants */
      iOil = 1;
      iGas = 0;
      
      /* Set conversion terms */
//      dCvp = 6894.7; /*psia to pascal */
//      dCvt = 1.0 / 1.8; /*fahrenheit to celsius without -32 translation */
//      dCvv = 1.0 / 16.01846; /* kg/m3 to pound/ft3 */
//      dCvm = 1.0 / 1000.0; /* Pa.s to mPa.s (cp) */

      /* Memory allocations */
      iFlashes = 1;
      iJ = iFlashes * ( 2 + iNc + 2 * ( 3 + iNc ) );
      pTemperature = new double [ iJ  ];
      pPressure = pTemperature + iFlashes;
      pAccumulation = pPressure + iFlashes;
      pDensity = pAccumulation + iFlashes * iNc;
      pPhaseAcc = pDensity + iFlashes * 2;
      pViscosity = pPhaseAcc + iFlashes * 2;
      pMassFraction = pViscosity + iFlashes * 2;

      /* Pressure factors */
      pTemperature[0] = temperature;
      pPressure[0] = pressure;

      /* Set compositions */
      pAccumulation[0]  = compMasses[0] ;
      pAccumulation[1]  = compMasses[1] ;
      pAccumulation[2]  = compMasses[2] ;
      pAccumulation[3]  = compMasses[3] ;
      pAccumulation[4]  = compMasses[4] ;
      pAccumulation[5]  = compMasses[5] ;
      pAccumulation[6]  = compMasses[6] ;
      pAccumulation[7]  = compMasses[7] ;
      pAccumulation[8]  = compMasses[8] ;
      pAccumulation[9]  = compMasses[9] ;
      pAccumulation[10] = compMasses[10];
      pAccumulation[11] = compMasses[11];
      pAccumulation[12] = compMasses[12];
      pAccumulation[13] = compMasses[13];
      
#ifdef DEBUG_EXTENSIVE
      for ( i = 0; i < iFlashes; i++ ) 
      {
         cout << setw(  9 ) << i
              << setw( 15 ) << pPressure[i]
              << setw( 15 ) << pTemperature[i];
         for ( iJ = 0; iJ < iNc; iJ++ ) 
         {
            cout << setw( 15 ) << pAccumulation[i+iJ*iFlashes];
         }
         cout << endl;
      }
#endif
      
      /* Call the function */
      EosCauldron::EosGetProperties ( iFlashes, iOil, iGas, pPressure,  
                                      pTemperature, pAccumulation, NULL,  
                                      pMassFraction, NULL, NULL, pvttable );

      /* Call the function again */
      EosCauldron::EosGetProperties ( iFlashes, iOil, iGas, pPressure, 
                                      pTemperature, pAccumulation, pPhaseAcc, 
                                      NULL, pDensity, pViscosity, pvttable );

#ifdef EOSPACK_OUT
      //output 
      eosout << "Pressure        " << pPressure[0] << endl;
      eosout << "Temperature     " << pTemperature[0] << endl;
      eosout << "Oil_Phase_Acc   " << ( iOil > -1 ? pPhaseAcc[iOil] : 0.0 ) << endl;
      eosout << "Gas_Phase_Acc   " << ( iGas > -1 ? pPhaseAcc[iGas] : 0.0 ) << endl;
      eosout << "Oil_Density     " << ( iOil > -1 ? pDensity[iOil] : 0.0 ) << endl;
      eosout << "Gas_Density     " << ( iGas > -1 ? pDensity[iGas] : 0.0 ) << endl;
      eosout << "Oil_Viscosity   " << ( iOil > -1 ? pViscosity[iOil] : 0.0 ) << endl;
      eosout << "Gas_Viscosity   " << ( iGas > -1 ? pViscosity[iGas] : 0.0 ) << endl;
      for (i=0;i<NUM_COMP;++i)
      {	
         
         eosout << "Gas_Phase_Frac_" << theComponentManager.GetSpeciesName(i) << "   " << pMassFraction[i+iNc*iGas] << "   "
                << "Oil_Phase_Frac_" << theComponentManager.GetSpeciesName(i) << "   " << pMassFraction[i+iNc*iOil] << endl;
         
      }
      eosout << endl <<endl;   	
#endif


#ifdef DEBUG_EXTENSIVE		
      cout << setw(  9 ) << "Flash_Nb"
	   << setw( 17 ) << "Pressure"
	   << setw( 17 ) << "Temperature"
	   << setw( 17 ) << "Oil_Phase_Acc"
	   << setw( 17 ) << "Gas_Phase_Acc"
	   << setw( 17 ) << "Oil_Density"
	   << setw( 17 ) << "Gas_Density"
	   << setw( 17 ) << "Oil_Viscosity"
	   << setw( 17 ) << "Gas_Viscosity"
	   << setw( 17 ) << "Oil_C1_Mass"
	   << setw( 17 ) << "Oil_C2_Mass"
	   << setw( 17 ) << "Oil_C3_Mass"
	   << setw( 17 ) << "Oil_C4_Mass"
	   << setw( 17 ) << "Oil_C5_Mass"
	   << setw( 17 ) << "Oil_N2_Mass"
	   << setw( 17 ) << "Oil_Cox_Mass"
	   << setw( 17 ) << "Oil_C6-14S_Mass"
	   << setw( 17 ) << "Oil_C6-14A_Mass"
	   << setw( 17 ) << "Oil_C15+_Mass"
	   << setw( 17 ) << "Gas_C1_Mass"
	   << setw( 17 ) << "Gas_C2_Mass"
	   << setw( 17 ) << "Gas_C3_Mass"
	   << setw( 17 ) << "Gas_C4_Mass"
	   << setw( 17 ) << "Gas_C5_Mass"
	   << setw( 17 ) << "Gas_N2_Mass"
	   << setw( 17 ) << "Gas_Cox_Mass"
	   << setw( 17 ) << "Gas_C6-14S_Mass"
	   << setw( 17 ) << "Gas_C6-14A_Mass"
	   << setw( 17 ) << "Gas_C15+_Mass"
	   << endl;
#endif		



      cout.precision( 7 );

      for ( i = 0; i < iFlashes; i++ ) 
      {
         phaseDensity[iOil]   = ( iOil > -1 ? pDensity[i+iOil*iFlashes] : 0.0 );
         phaseDensity[iGas]   = ( iGas > -1 ? pDensity[i+iGas*iFlashes] : 0.0 );   
         phaseViscosity[iOil] = ( iOil > -1 ? pViscosity[i+iOil*iFlashes] * 1000 : 0.0 );
         phaseViscosity[iGas] = ( iGas > -1 ? pViscosity[i+iGas*iFlashes] * 1000 : 0.0 );
         
#ifdef DEBUG_EXTENSIVE
         cout << setw(  9 ) << i
              << setw( 17 ) << pPressure[i]
              << setw( 17 ) << pTemperature[i]
              << setw( 17 ) << ( iOil > -1 ? pPhaseAcc[i+iOil*iFlashes] : 0.0 )
              << setw( 17 ) << ( iGas > -1 ? pPhaseAcc[i+iGas*iFlashes] : 0.0 )
              << setw( 17 ) << phaseDensity[iOil]
              << setw( 17 ) << phaseDensity[iGas]
              << setw( 17 ) << phaseViscosity[iOil]
              << setw( 17 ) << phaseViscosity[iGas];
#endif
         

         for ( iJ = 0; iJ < iNc; iJ++ ) 
         {
            phaseCompMasses[iOil][iJ]=( iOil > -1 ? pMassFraction[i+(iJ+iNc*iOil)*iFlashes] * pPhaseAcc[i+iOil*iFlashes]: 0.0 );
#ifdef DEBUG_EXTENSIVE
            cout << setw( 17 ) << phaseCompMasses[iOil][iJ];
#endif

         }
         for ( iJ = 0; iJ < iNc; iJ++ ) 
         {
            phaseCompMasses[iGas][iJ] = ( iGas > -1 ? pMassFraction[i+(iJ+iNc*iGas)*iFlashes] * pPhaseAcc[i+iGas*iFlashes]: 0.0);				  
#ifdef DEBUG_EXTENSIVE
            cout << setw( 17 ) << phaseCompMasses[iGas][iJ];
#endif
            
         }
         
      }
      
      /* Output in other units: not supported anymore */
      
      /*
 	 if ( EOS_DEBUG_INPUT )
	 {
             scanf ( "%i", &iJ ); 
	 }
    */

      /* Free memory */
      delete[] pTemperature;
      delete pvttable;

   }
   catch(polynomials::error::SyntaxError& s)
   {
      cerr << "Syntax Error in EosPack: " << s.info << endl;  
      return false;
   }
   catch(polynomials::error::NumericError& n)
   {
      cerr << "Numeric Error in EosPack: " << n.info << endl;  
      return false;
   }
   catch(string& s)
   {
      cerr << "Error in EosPack: " << s << endl;  
      return false;
   }
   catch(...)
   {
      cerr << "Error: unhandled exception in EosPack. " << endl;  
      return false;
   }
   
   /* End of routine */
   return true; 
   
}
double pvtFlash::EosPack::getMolWeightLumped(int componentId, double gorm)
{
   return getMolWeight( getLumpedIndex( componentId ), gorm );
}

double pvtFlash::EosPack::getMolWeight(int componentId, double gorm)
{
   return m_propertyFunc[componentId][0](gorm);
} 

double pvtFlash::EosPack::getCriticalTemperature(int componentId, double gorm)
{
   return m_propertyFunc[componentId][5](gorm);
} 

double pvtFlash::EosPack::getCriticalVolume(int componentId, double gorm)
{
   return m_propertyFunc[componentId][2](gorm);
}

double pvtFlash::EosPack::getCriticalTemperatureLumped(int componentId, double gorm)
{
   return getCriticalTemperature( getLumpedIndex( componentId ), gorm );
} 

double pvtFlash::EosPack::getCriticalVolumeLumped(int componentId, double gorm)
{
   return getCriticalVolume( getLumpedIndex( componentId ), gorm );
} 

double pvtFlash::EosPack::gorm(const double weights[CBMGenerics::ComponentManager::NumberOfOutputSpecies])
{
   const vector<double> weightVec(weights, weights + CBMGenerics::ComponentManager::NumberOfOutputSpecies);

   return pvtFlash::gorm( weightVec );
 
}
double pvtFlash::gorm(const vector<double>& weights)
{
   assert ( weights.size() == CBMGenerics::ComponentManager::NumberOfOutputSpecies );

   double denom = 0.0;
   denom += weights[C6_14ARO];
   denom += weights[C6_14SAT];
   denom += weights[C15_ARO];
   denom += weights[C15_SAT];
   denom += weights[RESINS];
   denom += weights[ASPHALTENES];

   denom += weights[LSC];
   denom += weights[C15_AT];
   denom += weights[C15_AROS];
   denom += weights[C15_SATS];

   denom += weights[C6_14BT];
   denom += weights[C6_14DBT];
   denom += weights[C6_14BP];
   denom += weights[C6_14SATS];
   denom += weights[C6_14AROS];

   assert(denom >= 0.0);
   
   if (denom == 0.0) {
      // return CBMGenerics::undefined;
      return 1.0e+80; 
   }

   double num = 0.0;
   num += weights[C1];
   num += weights[C2];
   num += weights[C3];
   num += weights[C4];
   num += weights[C5];

   num += weights[H2S];
   
   return num / denom;
}


double pvtFlash::getMolWeight(int componentId, const vector<double>& weights)
{
   pvtFlash::EosPack& eosPack = pvtFlash::EosPack::getInstance();
   return weights[componentId] / eosPack.getMolWeightLumped(componentId, gorm(weights) );
}

/// The determination of the critical temperature can be done using Li's mixing rule 
/// (based on effective concentration), which is more appropriate than the pseudocritical 
/// temperature calculated with Kay's mixing rule (based on molar fraction).  Compared 
/// to the full EOS (?) computation, deviation with Li's mixing rule average to less than 
/// 4K for binary systems and 11K for multicomponent systems.
///
/// Li's mixing rule is given by:
///
///          --
///    T  =  >  lambda   T
///     c    --       i   ci
///           i
///
/// with:
///
/// lambda_i: Effective concentration of component i.
/// T_ci    : Critical temperature of component i.
///
/// with lambda_i defined as:
///
///                             --
///    lambda  =   Z  V   /  (  >   Z  V  )
///          i      i  ci       --   i  ci
///                              i
/// with:
///
/// Z_i : molar fraction of component i
/// V_ci: critical molar volume of component i.
///

double pvtFlash::criticalTemperatureAccordingToLiMixingRule(const vector<double>& 
                                                            phaseWeights, const double& gorm)
{
   EosPack& eosPack = EosPack::getInstance();
   
   // We roll the three for loops (i.e. the loop in order to calculate the total number of moles 
   // (for the molar fraction), the loop in order to calculate the sum over Z_i V_ci into one 
   // and calculate:
   //
   //         --  weight[i]
   //         >  ------------   V    T
   //         -- molWeight[i]    ci   ci
   //          i
   //   T  =  --------------------------
   //    c    
   //             --  weight[i]
   //             >  ------------   V    
   //             -- molWeight[i]    ci
   //              i
   //
   double critialTemperature = 0.0;
   double norm = 0.0;

   for (int i = 0; i != phaseWeights.size(); ++i)
   {
      if (phaseWeights[i] == 0.0) continue;

      double molWeight_i = eosPack.getMolWeight(i,gorm);
      assert(molWeight_i > 0.0);
      double moles_i = phaseWeights[i] / molWeight_i;
      
      double V_ci = eosPack.getCriticalVolume(i,gorm);
      double T_ci = eosPack.getCriticalTemperature(i,gorm);

      double lambda_i = moles_i * V_ci;
      critialTemperature += lambda_i * T_ci;
      norm += lambda_i;
   }   
   assert(norm > 0.0);
   critialTemperature /= norm;
   return critialTemperature;
}


double pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(const vector<double>& phaseWeights, const double& gorm)
{
   EosPack& eosPack = EosPack::getInstance();

   double critialTemperature = 0.0;
   double norm = 0.0;

   for (int i = 0; i != phaseWeights.size(); ++i)
   {
      if (phaseWeights[i] == 0.0) continue;

      int lumpedInd = eosPack.getLumpedIndex(i);
      double molWeight_i = eosPack.getMolWeight(lumpedInd, gorm);
 
      assert(molWeight_i > 0.0);
      double moles_i = phaseWeights[i] / molWeight_i;
      
      double V_ci = eosPack.getCriticalVolume(lumpedInd, gorm);
      double T_ci = eosPack.getCriticalTemperature(lumpedInd, gorm) ;

      double lambda_i = moles_i * V_ci;
      critialTemperature += lambda_i * T_ci;
      norm += lambda_i;
   }   
   assert(norm > 0.0);
   critialTemperature /= norm;
  
   return critialTemperature;
}

void testPolynomialParse()
{

   string func="[*:  3.1415] 5-2.5x^2; [3.1415:*] 6e-1+4x^3; ";
   //	string func="5";


   cout << "string to be parsed:  " << func << endl;
   
   using namespace polynomials::parse;

	
   vector<Token> theTokens;

   polynomials::parse::GetTokens(func, theTokens);
   
   cout << endl << "theTokens: ";  
   for (size_t i=0;i<theTokens.size();++i)
      cout << theTokens[i];
   cout << endl;
   
   using namespace polynomials;

   Polynomial poly;

   size_t start=0;
   PiecewisePolynomial piecewise;
   ParsePiecewisePolynomial(theTokens, start, piecewise); 
   
   cout << endl << piecewise << endl;
   double x = 2.0;
   cout << "x=" << x << " -> " << piecewise(x) << endl;
   
   x = 4.0;
   cout << "x=" << x << " -> " << piecewise(x) << endl;
  
}
