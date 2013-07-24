#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <limits>

#include <assert.h>

void testPolynomialParse();

/*
// Program for testing Cauldron link
*/
#include <math.h>
#include <stdlib.h>

#include "DllExport.h"
#include "EosCauldron.h"
#include "EosPack.h"
#include "EosPvtTable.h"
#include "EosPvtModel.h"
#include "Parse.h"

std::string pvtFlash::pvtPropertiesConfigFile;

static const std::string ConfigFileName = "PVT_properties.cfg";

pvtFlash::EosPack::EosPack() : m_isReadInOk( true ),
                               m_propertyFunc( 0 ),
                               m_omegaA( 0 ),
                               m_omegaB( 0 ),
                               m_corrLBC( 0 )
{
#ifdef DEBUG_EXTENSIVE
   std::cout << "calling EosPack-constructor" << std::endl;
#endif

   resetToDefaultCritAoverBterm();     // init m_CritAoverB and m_phaseIdMethod
   setNonLinearSolverConvParameters(); // init max iterations number and stop tolerance value for nonlinear solver

   try
   {
      CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
      const int NUM_COMP = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;

      if( pvtFlash::pvtPropertiesConfigFile == "" )
      {
         const char * eosPackDir;
         if ( !( eosPackDir = getenv( "MYEOSPACKDIR" ) ) )
         {
            eosPackDir = getenv( "EOSPACKDIR" );
         }
         if ( !eosPackDir )
         {
            throw std::string(" Environment Variable EOSPACKDIR not set." );
         }

         // read config-file tables
         pvtFlash::pvtPropertiesConfigFile = std::string( eosPackDir ) + "/" + ConfigFileName;
      }

      // read config-file tables
      std::ifstream ifs( pvtFlash::pvtPropertiesConfigFile.c_str() );
      if ( !ifs )
      {
         throw pvtPropertiesConfigFile + " could not be opened.";
      }

      fileIO::Table compPropTable( ifs );
      
      string tableNameWanted = "PVT-Component-Properties";
      
      if ( ( ! compPropTable.isReadSuccessful() ) ||
           (   compPropTable.getTableName().find( tableNameWanted, 0 ) == string::npos )
         )
      {
         throw tableNameWanted + " table expected. Aborting...";
      }
     
      fileIO::Table generalPropTable;
      //concatenate as many "PVT-Component-Properties"-tables as available, last table contains general properties
      while ( true )
      {
         fileIO::Table table( ifs );
         if ( table.getTableName().find( tableNameWanted, 0 ) != std::string::npos ) 
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
      std::cout << "component table:" << compPropTable << std::endl<< std::endl;
      std::cout << "general table:" << std::endl << generalPropTable << std::endl << std::endl;
      std::ofstream ofs( "myout.txt", ios_base::out | ios_base::app );
      ofs << compPropTable;
      ofs << std::endl << std::endl;
#endif
      
      // parse component based data
      m_propertyFunc = new polynomials::PiecewisePolynomial*[NUM_COMP];
      int nCompProp = 6;
      m_propertyFunc[0] = new polynomials::PiecewisePolynomial[nCompProp * NUM_COMP];

      for ( int iComp = 1; iComp < NUM_COMP; ++iComp )
      {
         m_propertyFunc[iComp]= m_propertyFunc[iComp - 1] + nCompProp;
      }
      
      for ( int i = 0; i < NUM_COMP; ++i )
      {
         int indexFramework = theComponentManager.GetSpeciedIdByName( compPropTable.getEntry( i, 0 ) );
         if ( indexFramework < 0 )
         {
            throw std::string( "Bad Name: " ) +  compPropTable.getEntry( i, 0 );
         }

         for ( int iProp = 0; iProp < nCompProp; ++iProp )
         {   
            const string& func = compPropTable.getEntry( i, iProp+1 );

            std::vector<polynomials::parse::Token> tokens;

            polynomials::parse::GetTokens( func, tokens );
            
            size_t start = 0;
            polynomials::parse::ParsePiecewisePolynomial( tokens, start, m_propertyFunc[indexFramework][iProp] );
         }
      }

#ifdef DEBUG_EXTENSIVE         
      for ( int i = 0; i < NUM_COMP; ++i )
      {   
         int indexFramework = theComponentManager.GetSpeciedIdByName( compPropTable.getEntry( i, 0 ) );

         // test output: are critical temperatures read correctly?
         std::cout << "parsed Property[" << theComponentManager.GetSpeciesName( indexFramework ) << "] = " <<
                      m_propertyFunc[indexFramework][5](0.0) << " Index for re-ordering: " << indexFramework << std::endl;
      }
#endif

      //parse general data
      std::string EquOfState = generalPropTable.getEntry( 0, 0 );
      if ( EquOfState.find( "SRK", 0 ) != std::string::npos )
      {
         m_isRK = 1;
      }
      else
      {
         m_isRK = 0;
      }
      
      m_omegaA  = new polynomials::PiecewisePolynomial;
      m_omegaB  = new polynomials::PiecewisePolynomial;
      m_corrLBC = new polynomials::PiecewisePolynomial[5];
      
      std::vector<polynomials::parse::Token> tokens;
      polynomials::parse::GetTokens( generalPropTable.getEntry( 0, 1 ).c_str(), tokens );
      size_t start = 0;
      polynomials::parse::ParsePiecewisePolynomial(tokens, start, *m_omegaA );

      tokens.clear();
      polynomials::parse::GetTokens( generalPropTable.getEntry( 0, 2 ).c_str(), tokens );
      start = 0;
      polynomials::parse::ParsePiecewisePolynomial(tokens, start, *m_omegaB);

      for ( int i = 0 ;i < 5; ++i )
      {
         tokens.clear();
         polynomials::parse::GetTokens( generalPropTable.getEntry( 0, 3 + i ).c_str(), tokens );
         start = 0;
         polynomials::parse::ParsePiecewisePolynomial( tokens, start, m_corrLBC[i] );
      }

      for ( int i = 0; i < LSC; ++i )
      {
         lumpedSpeciesIndex[i] = i;
      }

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
      std::cout.precision( 15 );
      std::cout << "VolShift C15+Sat  (1.242) = " << m_propertyFunc[C15_SAT ][3](1.242) << std::endl;
      std::cout << "VolShift C6-14Sat (1.242) = " << m_propertyFunc[C6_14SAT][3](1.242) << std::endl;
      std::cout << "VolShift C6-14Aro (1.242) = " << m_propertyFunc[C6_14ARO][3](1.242) << std::endl;

      std::cout << std::endl << "CritPressure" << std::endl;
      
      std::cout <<"Pc resins      (2.4) = " << m_propertyFunc[RESINS     ][4](2.4) << std::endl;
      std::cout <<"Pc resins      (2.5) = " << m_propertyFunc[RESINS     ][4](2.5) << std::endl;
      std::cout <<"Pc C15+Sat     (2.4) = " << m_propertyFunc[C15_SAT    ][4](2.4) << std::endl;
      std::cout <<"Pc C15+Sat     (2.5) = " << m_propertyFunc[C15_SAT    ][4](2.5) << std::endl;
      std::cout <<"Pc C6-14Sat    (2.4) = " << m_propertyFunc[C6_14SAT   ][4](2.4) << std::endl;
      std::cout <<"Pc C6-14Sat    (2.5) = " << m_propertyFunc[C6_14SAT   ][4](2.5) << std::endl;
      std::cout <<"Pc asphaltenes (2.4) = " << m_propertyFunc[ASPHALTENES][4](2.4) << std::endl;
      std::cout <<"Pc asphaltenes (2.5) = " << m_propertyFunc[ASPHALTENES][4](2.5) << std::endl;
      std::cout <<"Pc C15+Aro     (2.4) = " << m_propertyFunc[C15_ARO    ][4](2.4) << std::endl;
      std::cout <<"Pc C15+Aro     (2.5) = " << m_propertyFunc[C15_ARO    ][4](2.5) << std::endl;
      std::cout <<"Pc C6-14Aro    (2.4) = " << m_propertyFunc[C6_14ARO   ][4](2.4) << std::endl;
      std::cout <<"Pc C6-14Aro    (2.5) = " << m_propertyFunc[C6_14ARO   ][4](2.5) << std::endl;
      
      std::cout << std::endl << "CritTemperature" << std::endl;

      std::cout <<"Tc resins      (2.4) = " << m_propertyFunc[RESINS     ][5](2.4) << std::endl;
      std::cout <<"Tc resins      (2.5) = " << m_propertyFunc[RESINS     ][5](2.5) << std::endl;
      std::cout <<"Tc C15+Sat     (2.4) = " << m_propertyFunc[C15_SAT    ][5](2.4) << std::endl;
      std::cout <<"Tc C15+Sat     (2.5) = " << m_propertyFunc[C15_SAT    ][5](2.5) << std::endl;
      std::cout <<"Tc C6-14Sat    (2.4) = " << m_propertyFunc[C6_14SAT   ][5](2.4) << std::endl;
      std::cout <<"Tc C6-14Sat    (2.5) = " << m_propertyFunc[C6_14SAT   ][5](2.5) << std::endl;
      std::cout <<"Tc asphaltenes (2.4) = " << m_propertyFunc[ASPHALTENES][5](2.4) << std::endl;
      std::cout <<"Tc asphaltenes (2.5) = " << m_propertyFunc[ASPHALTENES][5](2.5) << std::endl;
      std::cout <<"Tc C15+Aro     (2.4) = " << m_propertyFunc[C15_ARO    ][5](2.4) << std::endl;
      std::cout <<"Tc C15+Aro     (2.5) = " << m_propertyFunc[C15_ARO    ][5](2.5) << std::endl;
      std::cout <<"Tc C6-14Aro    (2.4) = " << m_propertyFunc[C6_14ARO   ][5](2.4) << std::endl;
      std::cout <<"Tc C6-14Aro    (2.5) = " << m_propertyFunc[C6_14ARO   ][5](2.5) << std::endl;
#endif
   }   
   catch ( polynomials::error::SyntaxError& s )
   {
      m_isReadInOk = false;
      std::cerr << "Syntax Error in EosPack: " << s.info << std::endl;
      throw s;
   }
   catch ( polynomials::error::NumericError& n )
   {
      m_isReadInOk = false;
      std::cerr << "Numeric Error in EosPack: " << n.info << std::endl;
      throw n;
   }
   catch ( std::string& s )
   {
      m_isReadInOk = false;
      std::cerr << "Error in EosPack: " << s << std::endl;
      throw s;
   }
   catch (...)
   {
      m_isReadInOk = false;
      std::cerr << "Error: unhandled exception in EosPack." << std::endl;
      throw m_isReadInOk;
   }
}

pvtFlash::EosPack::~EosPack()
{
   if ( m_propertyFunc )
   {
      if ( m_propertyFunc[0] ) delete [] m_propertyFunc[0];
      delete [] m_propertyFunc;
   }
   
   if ( m_omegaA  ) delete m_omegaA;
   if ( m_omegaB  ) delete m_omegaB;
   if ( m_corrLBC ) delete [] m_corrLBC;
}   


pvtFlash::EosPack& pvtFlash::EosPack::getInstance()
{
   static EosPack theEosCalculator;
   return theEosCalculator;
}

int pvtFlash::EosPack::getLumpedIndex( int ind ) const
{
   return lumpedSpeciesIndex[ind];
}

void pvtFlash::EosPack::getLumpingFractions( const vector<double>& inWeights, double unlumpFractions[] )
{
   const int NUM_COMP  = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
   double outWeights [NUM_COMP];

   pvtFlash::EosPack::lumpComponents( (double *)(&inWeights[0]), outWeights, unlumpFractions );
}

void pvtFlash::EosPack::lumpComponents( const double in_compMasses[], // size is NumberOfOutputSpecies;
                                        double out_compMasses[],      // size is NumberOfSpeciesToFlash
                                        double unlump_fraction[] )    // size is NumberOfOutputSpecies
{
   CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
   
   const int NUM_COMP     = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
   const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;

   for ( int i = 0; i < NUM_COMP; ++i )
   {
      out_compMasses[i] = in_compMasses[i];
   }
   std::fill( unlump_fraction, unlump_fraction + NUM_COMP_TOT, 1.0 );

   for ( int k = 0; k < 2; ++k )
   {
      int compIndex[5];
      int cmpNum = 0;
      int lump_component_ind = -1;

      switch( k )
      {
         case 0:
            // Lump LSC, C15+AT, C15+AroS, C15+SatS to C15+Sat
            compIndex[cmpNum++] = LSC;      // theComponentManager.GetSpeciedIdByName("LSC");
            compIndex[cmpNum++] = C15_AT;   // theComponentManager.GetSpeciedIdByName("C15+AT");
            compIndex[cmpNum++] = C15_AROS; // theComponentManager.GetSpeciedIdByName("C15+AroS");
            compIndex[cmpNum++] = C15_SATS; // theComponentManager.GetSpeciedIdByName("C15+SatS");

            lump_component_ind = C15_SAT;   // theComponentManager.GetSpeciedIdByName("C15+Sat");
            break;

         case 1:
            // Lump C6-14BT, C6-14DBT, C6-14BP, C6-14SatS, C6-14AroS to C16-14Aro
            compIndex[cmpNum++] = C6_14BT;   // theComponentManager.GetSpeciedIdByName("C6-14BT");
            compIndex[cmpNum++] = C6_14DBT;  // theComponentManager.GetSpeciedIdByName("C6-14DBT");
            compIndex[cmpNum++] = C6_14BP;   // theComponentManager.GetSpeciedIdByName("C6-14BP");
            compIndex[cmpNum++] = C6_14SATS; // theComponentManager.GetSpeciedIdByName("C6-14SatS");
            compIndex[cmpNum++] = C6_14AROS; // theComponentManager.GetSpeciedIdByName("C6-14AroS");

            lump_component_ind = C6_14ARO;   // theComponentManager.GetSpeciedIdByName("C6-14Aro");
            break;
         
         default: 
            assert(0);
            break;
      }

#ifdef DEBUG_EXTENSIVE1
      std::cout << "Lumped to "<<  lump_component_ind << " : ";
      for ( int i = 0; i < cmpNum; ++i ) std::cout << compIndex[i] << ", ";
      std::cout << std::endl;
#endif
   
      for( int i = 0; i < cmpNum; ++i )
      { 
         out_compMasses[lump_component_ind] += in_compMasses[compIndex[i]];
      }

      if( out_compMasses[lump_component_ind] != 0.0 )
      {
         double invMass = 1.0 / out_compMasses[lump_component_ind];

         for( int i = 0; i < cmpNum; ++i )
         { 
            unlump_fraction[compIndex[i]] = in_compMasses[compIndex[i]] * invMass;
         }
         unlump_fraction[lump_component_ind] = in_compMasses[lump_component_ind] * invMass;
      }
   }

#ifdef DEBUG_EXTENSIVE1         
   // debug check
  
   double in_totMasses = 0, out_totMasses = 0, unlump_sum = 0;
   for( int i = 0; i < NUM_COMP; ++ i )
   {
      out_totMasses += out_compMasses[i];
   }
   std::cout << "Unlump fractions: " << std::endl;
   for ( int i = 0; i < NUM_COMP_TOT; ++i )
   {
      in_totMasses += in_compMasses[i];
      unlump_sum += unlump_fraction[i];
      std::cout << unlump_fraction[i] << ";";
   }
   std::cout << std::endl;
   std::cout << "In masses = " << in_totMasses << " , Out masses = " << out_totMasses << ", Unlumpfraction = " <<  unlump_sum  << "." << std::endl;
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
   for ( int i = 0; i < NUM_COMP; ++i )
   {
      out_phaseCompMasses[iOil][i] = in_paseCompMasses[iOil][i];
      out_phaseCompMasses[iGas][i] = in_paseCompMasses[iGas][i];
   }

   for ( int k = 0; k < 2; ++k )
   {
      int compIndex[5];
      int cmpNum = 0;
      int lump_component_ind = -1;

      switch( k )
      {
         case 0:
            // UnLump LSC, C15+AT, C15+AroS, C15+SatS from C15+Sat
            compIndex[cmpNum++] = LSC;      // theComponentManager.GetSpeciedIdByName("LSC");
            compIndex[cmpNum++] = C15_AT;   // theComponentManager.GetSpeciedIdByName("C15+AT");
            compIndex[cmpNum++] = C15_AROS; // theComponentManager.GetSpeciedIdByName("C15+AroS");
            compIndex[cmpNum++] = C15_SATS; // theComponentManager.GetSpeciedIdByName("C15+SatS");

            lump_component_ind = C15_SAT;   // theComponentManager.GetSpeciedIdByName("C15+Sat");
            break;

         case 1:
            // Unlump C6-14BT, C6-14DBT, C6-14BP, C6-14SatS, C6-14AroS from C16-14Aro
            compIndex[cmpNum++] = C6_14BT;   // theComponentManager.GetSpeciedIdByName("C6-14BT");
            compIndex[cmpNum++] = C6_14DBT;  // theComponentManager.GetSpeciedIdByName("C6-14DBT");
            compIndex[cmpNum++] = C6_14BP;   // theComponentManager.GetSpeciedIdByName("C6-14BP");
            compIndex[cmpNum++] = C6_14SATS; // theComponentManager.GetSpeciedIdByName("C6-14SatS");
            compIndex[cmpNum++] = C6_14AROS; // theComponentManager.GetSpeciedIdByName("C6-14AroS");

            lump_component_ind = C6_14ARO;   // theComponentManager.GetSpeciedIdByName("C6-14Aro");
            break;
         
         default: 
            assert(0);
            break;
      }

      for ( int i = 0; i < cmpNum; ++i )
      {
         out_phaseCompMasses[iOil][compIndex[i]] = in_paseCompMasses[iOil][lump_component_ind] * unlump_fraction[compIndex[i]];
         out_phaseCompMasses[iGas][compIndex[i]] = in_paseCompMasses[iGas][lump_component_ind] * unlump_fraction[compIndex[i]];
      }
      out_phaseCompMasses[iOil][lump_component_ind] = in_paseCompMasses[iOil][lump_component_ind] * unlump_fraction[lump_component_ind];
      out_phaseCompMasses[iGas][lump_component_ind] = in_paseCompMasses[iGas][lump_component_ind] * unlump_fraction[lump_component_ind];

#ifdef DEBUG_EXTENSIVE1         
      std::cout << "Unlumped from "<<  lump_component_ind << " : ";
      for ( int i = 0; i < cmpNum; ++i ) std::cout << compIndex[i] << ", ";
      std::cout << std::endl;
      double out_totMassesGas = 0, out_totMassesOil = 0;
      for ( int i = 0; i < CBMGenerics::ComponentManager::NumberOfOutputSpecies; ++i )
      {
         out_totMassesGas += out_phaseCompMasses[iGas][i];
         out_totMassesOil += out_phaseCompMasses[iOil][i];
      }
      std::cout << std::endl;
      std::cout << "Out masses = gas:" << out_totMassesGas << " , oil: " <<  out_totMassesOil << ", Total = " <<  out_totMassesGas+out_totMassesOil  << "." << std::endl;
#endif
   } 
}


bool pvtFlash::EosPack::computeWithLumping( double  temperature, 
                                            double  pressure, 
                                            double  in_compMasses[],                     
                                            double  out_phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfOutputSpecies],
                                            double  phaseDensity [],
                                            double  phaseViscosity[],
                                            bool    isGormPrescribed,
                                            double  gorm,
                                            double* pKValues
                                          )   
{
   const int NUM_COMP     = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
   const int NUM_COMP_TOT = CBMGenerics::ComponentManager::NumberOfOutputSpecies;
   
   double compMasses[NUM_COMP];
   double phaseCompMasses[CBMGenerics::ComponentManager::NumberOfPhases][NUM_COMP];
   double unlump_fractions[NUM_COMP_TOT];

   pvtFlash::EosPack::lumpComponents( in_compMasses, compMasses, unlump_fractions );
   
   bool ret = pvtFlash::EosPack::compute( temperature, pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, gorm, pKValues );

   pvtFlash::EosPack::unlumpComponents( phaseCompMasses, out_phaseCompMasses, unlump_fractions );

   return ret;
} 


bool pvtFlash::EosPack::compute( double  temperature, 
                                 double  pressure, 
                                 double  compMasses[],                     
                                 double  phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfSpeciesToFlash],
                                 double  phaseDensity[],
                                 double  phaseViscosity[],
                                 bool    isGormPrescribed,
                                 double  gorm,
                                 double* pKValues
                               )   
{
   try
   {     
      if ( !m_isReadInOk )
      {
         throw std::string( "PVT configuration file not read correctly." );
      }
      
#ifdef DEBUG_EXTENSIVE1         
      std::cout << "Compute for: T = " << temperature << ", P = " << pressure << std::endl;
#endif

      /* Declarations */
      CBMGenerics::ComponentManager& theComponentManager = CBMGenerics::ComponentManager::getInstance();
      const int iNc = CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; //CBMGenerics::ComponentManager::NumberOfOutputSpecies;

      double pLB[5];


      // double dCvm;
      // double dCvp, dCvt, dCvv;
      EosPvtTable* pvttable;

      /* Terms for equation of state */
      double pMw[ 1 + iNc * ( 9 + iNc ) ];  
      double* pPc     = pMw     + iNc;
      double* pTc     = pPc     + iNc;
      double* pVc     = pTc     + iNc;

      double* pIft    = pVc     + iNc; // dummy here but it is used in ConcoctBrew
      double* pAc     = pIft    + iNc;
      double* pT      = pAc     + iNc; // dummy here but it is used in ConcoctBrew
      double* pOmegaA = pT      + 1;

      double* pOmegaB = pOmegaA + iNc;
      double* pShiftC = pOmegaB + iNc;
      double* pBinary = pShiftC + iNc;

#ifdef DEBUG_EXTENSIVE         
//      testPolynomialParse();
#endif      
      
      if ( !isGormPrescribed )
      {
         double gasMass = compMasses [ CBMGenerics::ComponentManager::C1 ] + 
                          compMasses [ CBMGenerics::ComponentManager::C2 ] +
                          compMasses [ CBMGenerics::ComponentManager::C3 ] +
                          compMasses [ CBMGenerics::ComponentManager::C4 ] +
                          compMasses [ CBMGenerics::ComponentManager::C5 ] +
                          compMasses [ CBMGenerics::ComponentManager::H2S ];

         double oilMass = compMasses [ CBMGenerics::ComponentManager::resin        ] + 
                          compMasses [ CBMGenerics::ComponentManager::C15PlusSat   ] +
                          compMasses [ CBMGenerics::ComponentManager::C6Minus14Sat ] +
                          compMasses [ CBMGenerics::ComponentManager::asphaltene   ] +
                          compMasses [ CBMGenerics::ComponentManager::C15PlusAro   ] +
                          compMasses [ CBMGenerics::ComponentManager::C6Minus14Aro ];

         gorm = oilMass != 0.0 ? gasMass / oilMass : 1.0e+80;
      }

      //fill arrays
      for ( int i = 0; i < iNc; ++i )
      {   
         pMw[i]     =  m_propertyFunc[i][0]( gorm );
         pAc[i]     =  m_propertyFunc[i][1]( gorm );
         pVc[i]     =  m_propertyFunc[i][2]( gorm );
         pShiftC[i] =  m_propertyFunc[i][3]( gorm );
         pPc[i]     =  m_propertyFunc[i][4]( gorm );
         pTc[i]     =  m_propertyFunc[i][5]( gorm );
         
         pOmegaA[i] =  (*m_omegaA)( gorm );
         pOmegaB[i] =  (*m_omegaB)( gorm );
      }

      for ( int i = 0; i < 5; ++i )
      {
         pLB[i] = m_corrLBC[i]( gorm );
      }

      //normalize volume shift terms
      for ( int i = 0; i < iNc; ++i )
      {   
         pShiftC[i] = pShiftC[i] * ( pPc[i] / 8314.472 ) / ( pOmegaB[i] * pTc[i] ); //consistent units
         //pShiftC[i] = pShiftC[i] * ( pPc[i] / 8.314472 ) / ( pOmegaB[i] * pTc[i] );
      }

      std::fill( pIft, pIft + iNc, 0.0 );
      // Binary interaction terms
      std::fill( pBinary, pBinary + iNc * iNc, 0.0 );

#ifdef EOSPACK_OUT
      std::ofstream eosout( "EosPack.out", ios_base::out | ios_base::app );
      eosout << std::endl << std::endl;
      eosout << "gorm:  " << gorm << std::endl << std::endl << std::endl;

      for ( int i = 0; i < iNc; ++i )
      {   
         eosout << "pMw[ "     << theComponentManager.GetSpeciesName(i) << "]    " << pMw[i] << std::endl;
         eosout << "pAc[ "     << theComponentManager.GetSpeciesName(i) << "]    " << pAc[i] << std::endl;
         eosout << "pVc[ "     << theComponentManager.GetSpeciesName(i) << "]    " << pVc[i] << std::endl;
         eosout << "pShiftC[ " << theComponentManager.GetSpeciesName(i) << "]    " << pShiftC[i] / ( ( pPc[i] / 8314.472 ) / ( pOmegaB[i] * pTc[i] ) ) << std::endl;
         eosout << "pPc[ "     << theComponentManager.GetSpeciesName(i) << "]    " << pPc[i] << std::endl;
         eosout << "pTc[ "     << theComponentManager.GetSpeciesName(i) << "]    " << pTc[i] << std::endl;
         eosout << "pOmegaA[ " << theComponentManager.GetSpeciesName(i) << "]    " << pOmegaA[i] << std::endl;
         eosout << "pOmegaB[ " << theComponentManager.GetSpeciesName(i) << "]    " << pOmegaB[i] << std::endl;
         eosout << std::endl <<endl;   
      }

      for ( int i = 0; i < 5; ++i )
      {
         eosout << "pLB[ " << i << "]    " <<  pLB[i] << std::endl;
      }

      eosout << "isRK "     << m_isRK << std::endl;
      eosout << "iNc "      << iNc  << std::endl;
      eosout << "pBinary[ " << iNc  << ", " << iNc << " ]" << std::endl;

      for ( int i = 0; i < iNc; ++i )
      {   
         for ( int j = 0; j < iNc; ++j )
         {
            eosout << pBinary[i*iNc+j] << "   ";
         }
         eosout << std::endl;
      }
#endif

      /* Construct pvt table */
      pvttable = EosCauldron::ConcoctBrew ( iNc, m_isRK, pMw, pT, pLB, m_CritAoverB, m_phaseIdMethod );

      /* Constants */
      int iOil = 1;
      int iGas = 0;
      
      /* Set conversion terms */
//      dCvp = 6894.7;         //psia to pascal
//      dCvt = 1.0 / 1.8;      //fahrenheit to celsius without -32 translation
//      dCvv = 1.0 / 16.01846; // kg/m3 to pound/ft3
//      dCvm = 1.0 / 1000.0;   // Pa.s to mPa.s (cp)

      /* Memory distribution done in stack */
      const int iFlashes = 1;
      
      double pTemperature[ iFlashes * ( 2 + iNc + 2 * ( 3 + iNc )) ];
      double* pPressure     = pTemperature  + iFlashes;
      double* pAccumulation = pPressure     + iFlashes;
      double* pDensity      = pAccumulation + iFlashes * iNc;
      double* pPhaseAcc     = pDensity      + iFlashes * 2;
      double* pViscosity    = pPhaseAcc     + iFlashes * 2;
      double* pMassFraction = pViscosity    + iFlashes * 2;

      /* Pressure factors */
      pTemperature[0] = temperature;
      pPressure[0]    = pressure;

      /* Set compositions */
      for ( int i = 0; i < iNc; ++i )
      {
         pAccumulation[i] = compMasses[i];
      }
      
#ifdef DEBUG_EXTENSIVE
      for ( int i = 0; i < iFlashes; i++ )
      {
         std::cout << setw(  9 ) << i
              << setw( 15 ) << pPressure[i]
              << setw( 15 ) << pTemperature[i];
         for ( int iJ = 0; iJ < iNc; iJ++ ) 
         {
            std::cout << setw( 20 ) << pAccumulation[i+iJ*iFlashes];
         }
         std::cout << std::endl;
      }
#endif


      /* Call the function */
      EosCauldron::EosGetProperties( iFlashes, iOil, iGas, pPressure, pTemperature, pAccumulation, pKValues,
                                     pPhaseAcc, pMassFraction, pDensity, pViscosity, 
                                     pvttable, m_maxItersNum, m_stopTolerance, m_NewtonRelaxCoeff );

#ifdef EOSPACK_OUT
      //output 
      eosout << "Pressure        " << pPressure[0]    << std::endl;
      eosout << "Temperature     " << pTemperature[0] << std::endl;

      eosout << "Oil_Phase_Acc   " << ( iOil > -1 ? pPhaseAcc[iOil]  : 0.0 ) << std::endl;
      eosout << "Gas_Phase_Acc   " << ( iGas > -1 ? pPhaseAcc[iGas]  : 0.0 ) << std::endl;
      eosout << "Oil_Density     " << ( iOil > -1 ? pDensity[iOil]   : 0.0 ) << std::endl;
      eosout << "Gas_Density     " << ( iGas > -1 ? pDensity[iGas]   : 0.0 ) << std::endl;
      eosout << "Oil_Viscosity   " << ( iOil > -1 ? pViscosity[iOil] : 0.0 ) << std::endl;
      eosout << "Gas_Viscosity   " << ( iGas > -1 ? pViscosity[iGas] : 0.0 ) << std::endl;

      for ( int i = 0; i < iNc; ++i )
      {   
         eosout << "Gas_Phase_Frac_" << theComponentManager.GetSpeciesName(i) << "   " << pMassFraction[i+iNc*iGas] << "   "
                << "Oil_Phase_Frac_" << theComponentManager.GetSpeciesName(i) << "   " << pMassFraction[i+iNc*iOil] << std::endl;
      }
      eosout << std::endl << std::endl;   
#endif

#ifdef DEBUG_EXTENSIVE      
      std::cout 
      << setw(  9 ) << "Flash_Nb"
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
      << std::endl;
#endif      

      for ( int i = 0; i < iFlashes; ++i ) 
      {
         phaseDensity[iOil]   = iOil > -1 ? pDensity[ i + iOil * iFlashes] : 0.0;
         phaseDensity[iGas]   = iGas > -1 ? pDensity[ i + iGas * iFlashes] : 0.0;

         if ( phaseViscosity )
         {
            phaseViscosity[iOil] = iOil > -1 ? pViscosity[ i + iOil * iFlashes] * 1000 : 0.0;
            phaseViscosity[iGas] = iGas > -1 ? pViscosity[ i + iGas * iFlashes] * 1000 : 0.0;
         }
         
#ifdef DEBUG_EXTENSIVE
         std::cout.precision( 7 );

         std::cout << setw( 9 ) 
              << i
              << setw( 17 ) << pPressure[i]
              << setw( 17 ) << pTemperature[i]
              << setw( 17 ) << ( iOil > -1 ? pPhaseAcc[i + iOil * iFlashes] : 0.0 )
              << setw( 17 ) << ( iGas > -1 ? pPhaseAcc[i + iGas * iFlashes] : 0.0 )
              << setw( 17 ) << phaseDensity[iOil]
              << setw( 17 ) << phaseDensity[iGas];
         if ( phaseViscosity )
         {
            std::cout << setw( 17 ) << phaseViscosity[iOil]
                      << setw( 17 ) << phaseViscosity[iGas];
         }
#endif

         for ( int iJ = 0; iJ < iNc; ++iJ ) 
         {
            phaseCompMasses[iOil][iJ] = iOil > -1 ? pMassFraction[i + ( iJ + iNc * iOil) * iFlashes] * pPhaseAcc[i + iOil * iFlashes] : 0.0;
            phaseCompMasses[iGas][iJ] = iGas > -1 ? pMassFraction[i + ( iJ + iNc * iGas) * iFlashes] * pPhaseAcc[i + iGas * iFlashes] : 0.0;
         }

#ifdef DEBUG_EXTENSIVE
         for ( int iJ = 0; iJ < iNc; ++iJ )
         {
            std::cout << setw( 17 ) << phaseCompMasses[iOil][iJ];
         }
         for ( int iJ = 0; iJ < iNc; ++iJ ) 
         {
            std::cout << setw( 17 ) << phaseCompMasses[iGas][iJ];
         }
#endif

      }
      /* Free memory */
      delete pvttable;

#ifdef DEBUG_EXTENSIVE1         
      std::cout << "Computed successfully " << std::endl;
#endif

   }
   catch( polynomials::error::SyntaxError& s )
   {
      cerr << "Syntax Error in EosPack: " << s.info << std::endl;
      return false;
   }
   catch( polynomials::error::NumericError& n )
   {
      cerr << "Numeric Error in EosPack: " << n.info << std::endl;
      return false;
   }
   catch( string& s )
   {
      cerr << "Error in EosPack: " << s << std::endl;
      return false;
   }
   catch(...)
   {
      cerr << "Error: unhandled exception in EosPack. " << std::endl;
      return false;
   }
   
   return true;
}

double pvtFlash::EosPack::getMolWeightLumped( int componentId, double gorm )
{
   return getMolWeight( getLumpedIndex( componentId ), gorm );
}

double pvtFlash::EosPack::getMolWeight( int componentId, double gorm )
{
   return m_propertyFunc[componentId][0]( gorm );
} 

double pvtFlash::EosPack::getCriticalTemperature( int componentId, double gorm )
{
   return m_propertyFunc[componentId][5]( gorm );
} 

double pvtFlash::EosPack::getCriticalVolume( int componentId, double gorm )
{
   return m_propertyFunc[componentId][2]( gorm );
}

double pvtFlash::EosPack::getCriticalTemperatureLumped( int componentId, double gorm )
{
   return getCriticalTemperature( getLumpedIndex( componentId ), gorm  );
} 

double pvtFlash::EosPack::getCriticalVolumeLumped( int componentId, double gorm )
{
   return getCriticalVolume( getLumpedIndex( componentId ), gorm );
} 

double pvtFlash::EosPack::gorm( const double weights[CBMGenerics::ComponentManager::NumberOfOutputSpecies] )
{
   const vector<double> weightVec( weights, weights + CBMGenerics::ComponentManager::NumberOfOutputSpecies );

   return pvtFlash::gorm( weightVec ); 
}

double pvtFlash::gorm( const vector<double>& weights )
{
   assert ( weights.size() == CBMGenerics::ComponentManager::NumberOfOutputSpecies );

   double denom = weights[C6_14ARO];
   denom       += weights[C6_14SAT];
   denom       += weights[C15_ARO];
   denom       += weights[C15_SAT];
   denom       += weights[RESINS];
   denom       += weights[ASPHALTENES];

   denom       += weights[LSC];
   denom       += weights[C15_AT];
   denom       += weights[C15_AROS];
   denom       += weights[C15_SATS];

   denom       += weights[C6_14BT];
   denom       += weights[C6_14DBT];
   denom       += weights[C6_14BP];
   denom       += weights[C6_14SATS];
   denom       += weights[C6_14AROS];

   assert( denom >= 0.0 );
   
   if ( denom == 0.0 )
   {
      // return CBMGenerics::undefined;
      return 1.0e+80;
   }

   double num = weights[C1];
   num       += weights[C2];
   num       += weights[C3];
   num       += weights[C4];
   num       += weights[C5];

   num       += weights[H2S];
   
   return num / denom;
}

double pvtFlash::getMolWeight( int componentId, const vector<double>& weights )
{
   pvtFlash::EosPack & eosPack = pvtFlash::EosPack::getInstance();
   return weights[componentId] / eosPack.getMolWeightLumped( componentId, gorm( weights ) );
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
double pvtFlash::criticalTemperatureAccordingToLiMixingRule( const vector<double>& phaseWeights, const double& gorm )
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

   for ( int i = 0; i != phaseWeights.size(); ++i )
   {
      if ( phaseWeights[i] == 0.0 ) continue;

      double molWeight_i = eosPack.getMolWeight( i, gorm );
      assert( molWeight_i > 0.0 );
      double moles_i = phaseWeights[i] / molWeight_i;
      
      double V_ci = eosPack.getCriticalVolume( i, gorm );
      double T_ci = eosPack.getCriticalTemperature( i, gorm );

      double lambda_i = moles_i * V_ci;
      critialTemperature += lambda_i * T_ci;
      norm += lambda_i;
   }   
   assert(norm > 0.0);
   critialTemperature /= norm;
   return critialTemperature;
}


double pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping( const vector<double>& phaseWeights, const double& gorm )
{
   EosPack& eosPack = EosPack::getInstance();

   double critialTemperature = 0.0;
   double norm = 0.0;

   for ( int i = 0; i != phaseWeights.size(); ++i )
   {
      if ( phaseWeights[i] == 0.0 ) continue;

      int lumpedInd = eosPack.getLumpedIndex( i );
      double molWeight_i = eosPack.getMolWeight( lumpedInd, gorm );
 
      assert( molWeight_i > 0.0 );
      double moles_i = phaseWeights[i] / molWeight_i;
      
      double V_ci = eosPack.getCriticalVolume( lumpedInd, gorm );
      double T_ci = eosPack.getCriticalTemperature( lumpedInd, gorm );

      double lambda_i = moles_i * V_ci;
      critialTemperature += lambda_i * T_ci;
      norm += lambda_i;
   }
   assert( norm > 0.0 );
   critialTemperature /= norm;
  
   return critialTemperature;
}

void pvtFlash::EosPack::setCritAoverBterm( double val )
{
   m_CritAoverB = val;
   m_phaseIdMethod = EOS_SINGLE_PHASE_AOVERB;
}

void pvtFlash::EosPack::resetToDefaultCritAoverBterm()
{
   m_CritAoverB = 5.0;
   m_phaseIdMethod = EOS_SINGLE_PHASE_DEFAULT;
}

void pvtFlash::EosPack::setNonLinearSolverConvParameters( int maxItersNum, double stopTol, double newtonRelCoeff  )
{
   m_maxItersNum   = maxItersNum;
   m_stopTolerance = stopTol;
   m_NewtonRelaxCoeff = newtonRelCoeff;
}



void testPolynomialParse()
{
   std::string func="[*:  3.1415] 5-2.5x^2;[3.1415:*] 6e-1+4x^3;";
   //   string func="5";

   std::cout << "string to be parsed:  " << func << std::endl;
   
   std::vector<polynomials::parse::Token> theTokens;

   polynomials::parse::GetTokens( func, theTokens );
   
   std::cout << std::endl << "theTokens: ";

   for ( size_t i = 0; i < theTokens.size(); ++i )
   {
      std::cout << theTokens[i];
   }
   std::cout << std::endl;
   
   polynomials::Polynomial poly;

   size_t start = 0;
   polynomials::PiecewisePolynomial piecewise;
   polynomials::parse::ParsePiecewisePolynomial( theTokens, start, piecewise );
   
   std::cout << std::endl << piecewise << std::endl;
   double x = 2.0;
   std::cout << "x=" << x << " -> " << piecewise(x) << std::endl;
   
   x = 4.0;
   std::cout << "x=" << x << " -> " << piecewise(x) << std::endl;
}

