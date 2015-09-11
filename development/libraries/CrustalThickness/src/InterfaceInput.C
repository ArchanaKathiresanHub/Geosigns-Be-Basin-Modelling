#include "InterfaceInput.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "errorhandling.h"
#include <Interface/ProjectHandle.h>

//------------------------------------------------------------//
InterfaceInput::InterfaceInput(Interface::ProjectHandle * projectHandle, database::Record * record) :
   Interface::CrustalThicknessData(projectHandle, record) {

   clean();
   //-------------
   m_T0Map   = 0;
   m_TRMap   = 0;
   m_HCuMap  = 0;
   m_HLMuMap = 0;
   m_HBuMap  = 0;
   m_DeltaSLMap = 0;

   m_baseRiftSurfaceName = "";
} 
//------------------------------------------------------------//
InterfaceInput::~InterfaceInput() {

} 
//------------------------------------------------------------//
void InterfaceInput::clean() {

   //-------------- User defined ---------------------
   m_t_0 = 0.0;
   m_t_r = 0.0;
   m_initialCrustThickness = 0.0;
   m_initialLithosphericThickness = 0.0;
   m_maxBasalticCrustThickness = 0.0;
   m_seaLevelAdjustment = 0.0;

   //-------------- Basic constants ---------------------
   m_coeffThermExpansion = 0.0;
   m_initialSubsidenceMax = 0.0;
   m_thermalSubsidenceMax = 0.0;
   m_pi = M_PI;
   m_E0 = 0.0;
   m_tau = 0.0;
   
   //-------------- Lithospphere and crust properties ---------------------
   m_modelTotalLithoThickness = 0.0;
   m_backstrippingMantleDensity = 0.0;
   m_lithoMantleDensity = 0.0;
   m_baseLithosphericTemperature = 0.0;
   m_referenceCrustThickness = 0.0;
   m_referenceCrustDensity = 0.0;
   m_waterDensity = 0.0;
   
   //------------- Asthenosphere potential temperature data ---------------------
   m_A = 0.0;
   m_B = 0.0;
   
   //------------- Solidus (onset of adiabatic melting) ---------------------
   m_C = 0.0;
   m_D = 0.0;
   
   //------------- Magma-layer density ---------------------
   m_E = 0.0;
   m_F = 0.0;
   m_decayConstant = 0.0;

   //-------------
   m_modelCrustDensity = 0;
   m_TF_onset = 0;
   m_TF_onset_lin = 0;
   m_TF_onset_mig = 0;
   m_PTa = 0;
   m_magmaticDensity = 0;
   m_WLS_exhume = 0;
   m_WLS_crit = 0;
   m_WLS_onset = 0;
   m_WLS_exhume_serp = 0;  

   m_densityDiff = 0;
}
//------------------------------------------------------------//
void InterfaceInput::loadInputDataAndConfigurationFile( const string & inFile ) {
   loadInputData();
   loadConfigurationFile( inFile );
}
//------------------------------------------------------------//
void InterfaceInput::loadInputData() {
   
   m_T0Map   = getMap (Interface::T0Ini);
   m_TRMap   = getMap (Interface::TRIni);
   m_HCuMap  = getMap (Interface::HCuIni);
   m_HLMuMap = getMap (Interface::HLMuIni);
   m_HBuMap  = getMap (Interface::HBu);
   m_DeltaSLMap = getMap (Interface::DeltaSL);
   m_baseRiftSurfaceName = getSurfaceName();
   //cout << "Base of Rift = " << m_baseRiftSurfaceName << endl;
}
//------------------------------------------------------------//
void InterfaceInput::loadConfigurationFile( const string & inFile ) {

   char * CTCDIR = getenv("CTCDIR");
   char * MY_CTCDIR = getenv("MY_CTCDIR");

   string fullpath;
   
   if( MY_CTCDIR != 0 ) {
       fullpath = MY_CTCDIR + string("/") + inFile;
   } else if( CTCDIR != 0 ) {
      fullpath = CTCDIR + string("/") + inFile;
   } else {
      string s = "Environment Variable CTCDIR is not set. Aborting...";
      throw s;
   }

   ifstream  ConfigurationFile;
   ConfigurationFile.open( fullpath.c_str() );
   
   if(!ConfigurationFile) {
      getProjectHandle()->getMessageHandler().printLine( "Attempting to open file :" + fullpath + "\nNo cfg file available in the $CTCDIR directory... Aborting..." );
      throw RecordException( "Attempting to open file :" + fullpath + "\nNo cfg file available in the $CTCDIR directory... Aborting..." );
   }

   string line;
   size_t firstNotSpace;

   while( !ConfigurationFile.eof() ) {
      
      getline ( ConfigurationFile, line, '\n' );
      if( line.size() != 0 ) { 
         firstNotSpace = line.find_first_not_of(" "); 
      
         if( line[firstNotSpace] != '#' ) {
            
            if( line == CrustalThicknessInterface::TableBasicConstants || line.find( CrustalThicknessInterface::TableBasicConstants, 0) != string::npos ) {
               
               LoadBasicConstants( ConfigurationFile );
               
            } else if( line == CrustalThicknessInterface::TableLithoAndCrustProperties || line.find( CrustalThicknessInterface::TableLithoAndCrustProperties, 0 ) != string::npos ) {
               
               LoadLithoAndCrustProperties( ConfigurationFile );
               
            } else if( line == CrustalThicknessInterface::TableTemperatureData || line.find( CrustalThicknessInterface::TableTemperatureData, 0 ) != string::npos ) {
               
               LoadTemperatureData ( ConfigurationFile );
               
            } else if( line == CrustalThicknessInterface::TableSolidus || line.find( CrustalThicknessInterface::TableSolidus, 0) != string::npos ) {
               
               LoadSolidus ( ConfigurationFile );
               
            } else if( line == CrustalThicknessInterface::TableMagmaLayer || line.find( CrustalThicknessInterface::TableMagmaLayer, 0) != string::npos ) {
               
               LoadMagmaLayer ( ConfigurationFile );
               
            } else if( line == CrustalThicknessInterface::TableUserDefinedData || line.find( CrustalThicknessInterface::TableUserDefinedData, 0) != string::npos ) {
               
               // LoadUserDefinedData ( ConfigurationFile );
               // - now load from the project file
               
            } 
         }
      }
   }
   ConfigurationFile.close();
}
//------------------------------------------------------------//
void InterfaceInput::LoadBasicConstants( ifstream &ConfigurationFile ) {

   string line;
   vector<string> theTokens;
   string delim = ",";

   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
         break;
      }
      
      CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {
         if( theTokens[0] == CrustalThicknessInterface::coeffThermExpansion ) {
            
            m_coeffThermExpansion = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::initialSubsidenceMax ) {
            
            m_initialSubsidenceMax = atof( theTokens[1].c_str() );
            
         } else if( theTokens[0] == CrustalThicknessInterface::thermalSubsidenceMax ) {
            
            m_thermalSubsidenceMax = atof( theTokens[1].c_str() );
            
         } 
         // else if( theTokens[0] == CrustalThicknessInterface::pi ) {
            
         //    m_pi = atof( theTokens[1].c_str() );
            
         // } 
         else if( theTokens[0] == CrustalThicknessInterface::E0 ) {
            
            m_E0 = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::tau ) {
            
            m_tau = atof( theTokens[1].c_str() );
            
         }  
      } else {
         theTokens.clear();
         string s = "More or less arguments than expected.";
         throw s;
      }
      theTokens.clear();
   }
   if( m_tau == 0 ) {
      string s = "MeSsAgE ERROR  Tau = 0.";
      throw s;
   }
 
}

//------------------------------------------------------------//
void InterfaceInput::LoadLithoAndCrustProperties( ifstream &ConfigurationFile ) {

   string line;
   vector<string> theTokens;
   string delim = ",";
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
         break;
      }
      
      CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {
         if( theTokens[0] == CrustalThicknessInterface::modelTotalLithoThickness ) {

            m_modelTotalLithoThickness = atof( theTokens[1].c_str() );

         }  else if( theTokens[0] == CrustalThicknessInterface::backstrippingMantleDensity ) {

            m_backstrippingMantleDensity = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::lithoMantleDensity ) {

            m_lithoMantleDensity = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::baseLithosphericTemperature ) {

            m_baseLithosphericTemperature = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::referenceCrustThickness ) {

            m_referenceCrustThickness = atof( theTokens[1].c_str() );

         }  else if( theTokens[0] == CrustalThicknessInterface::referenceCrustDensity ) {

            m_referenceCrustDensity = atof( theTokens[1].c_str() );

         }  else if( theTokens[0] == CrustalThicknessInterface::waterDensity ) {

            m_waterDensity = atof( theTokens[1].c_str() );

         } 
         
      } else {
         theTokens.clear();
         string s = "More or less arguments than expected.";
         throw s;
      }
      theTokens.clear();
   }

   if( m_backstrippingMantleDensity - m_waterDensity != 0 ) {
      m_densityDiff = 1.0 / ( m_backstrippingMantleDensity - m_waterDensity );
   } else {
      string s = "BackstrippingMantleDensity = WaterDensity.";
      throw s;
   }
   if( m_modelTotalLithoThickness == 0 ) {
      string s = "MeSsAgE ERROR  TotalLithoThickness of the Model = 0.";
      throw s;
   }
}

//------------------------------------------------------------//
void InterfaceInput::LoadTemperatureData( ifstream &ConfigurationFile ) {

   string line;
   vector<string> theTokens;
   string delim = ",";
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
         break;
      }
      
      CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {

         if( theTokens[0] == CrustalThicknessInterface::A ) {

            m_A = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::B ) {

            m_B = atof( theTokens[1].c_str() );

         } 
      } else {
         theTokens.clear();
         string s = "More or less arguments than expected.";
         throw s;
      }
      theTokens.clear();
   }
}
//------------------------------------------------------------//
void InterfaceInput::LoadSolidus( ifstream &ConfigurationFile ) {

   string line;
   vector<string> theTokens;
   string delim = ",";
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
         break;
      }
      
      CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {

         if( theTokens[0] == CrustalThicknessInterface::C ) {

            m_C = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::D ) {

            m_D = atof( theTokens[1].c_str() );

         } 
      } else {
         theTokens.clear();
         string s = "More or less arguments than expected.";
         throw s;
      }
      theTokens.clear();
   }
}
//------------------------------------------------------------//
void InterfaceInput::LoadMagmaLayer( ifstream &ConfigurationFile ) {

   string line;
   vector<string> theTokens;
   string delim = ",";
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
         break;
      }
      
      CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {

         if( theTokens[0] == CrustalThicknessInterface::E ) {

            m_E = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::F ) {

            m_F = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::decayConstant ) {

            m_decayConstant = atof( theTokens[1].c_str() );

         } 
      } else {
         theTokens.clear();
         string s = "More or less arguments than expected.";
         throw s;
      }
      theTokens.clear();
   }
}
//------------------------------------------------------------//
void InterfaceInput::LoadUserDefinedData( ifstream &ConfigurationFile ) {

   string line;
   vector<string> theTokens;
   string delim = ",";
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
         break;
      }
      
      CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {

         if( theTokens[0] == CrustalThicknessInterface::t_0 ) {

            m_t_0 = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::t_r ) {

            m_t_r = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::initialCrustThickness ) {

            m_initialCrustThickness = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::maxBasalticCrustThickness ) {

            m_maxBasalticCrustThickness = atof ( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::initialLithosphericThickness ) {

            m_initialLithosphericThickness = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == CrustalThicknessInterface::seaLevelAdjustment ) {

            m_seaLevelAdjustment = atof( theTokens[1].c_str() );

         } 
      } else {
         theTokens.clear();
         string s = "More or less arguments than expected.";
         throw s;
      }
      theTokens.clear();
   }
}

//------------------------------------------------------------//
bool InterfaceInput::defineLinearFunction( LinearFunction & theFunction, unsigned int i, unsigned int j ) {

   // This method is to calculate coefficients for linear function to invert from WLS to TF (thinning factor)
   const double pi2 = pow (m_pi, 2);
   const double pi2_8 = pi2 / 8;
   
   // Step 4.1
   // average uppermost mantle density 
   if( m_HCuMap->getValue( i, j ) == m_HCuMap->getUndefinedValue() ) return false;
   m_initialCrustThickness = m_HCuMap->getValue( i, j );

   if( m_initialCrustThickness == 0 ) {
      string s = "MeSsAgE ERROR  initialCrustThickness = 0";
      throw s;
   }
   const double mantleDensityAV = m_lithoMantleDensity * (1 - (m_coeffThermExpansion * m_baseLithosphericTemperature / 2) * 
                                                          ((m_referenceCrustThickness + m_initialCrustThickness) / m_modelTotalLithoThickness));  
   // estimated continental crust density
   m_modelCrustDensity = (m_referenceCrustDensity * m_referenceCrustThickness + mantleDensityAV * (m_initialCrustThickness - m_referenceCrustThickness)) / m_initialCrustThickness;
   
   // Step 4.2
   // asthenosphere potential temperature
   if( m_HBuMap->getValue( i, j ) == m_HBuMap->getUndefinedValue() ) return false;
   m_maxBasalticCrustThickness = m_HBuMap->getValue( i, j );
   m_PTa = m_B + m_A * sqrt(m_maxBasalticCrustThickness);
   const double Hsol = m_C * m_PTa + m_D;
   // crustal thinning factor at melt onset
   if( m_HLMuMap->getValue( i, j ) == m_HLMuMap->getUndefinedValue() ) return false;
   m_initialLithosphericThickness = ( m_HLMuMap->getValue( i, j ) + m_initialCrustThickness );
   m_TF_onset = 1 - Hsol / m_initialLithosphericThickness;
   // liner approximation of thinning factor at melt onset
   m_TF_onset_lin = ( 1 + 2 * m_TF_onset ) / 3;
   // crustal thinning factor at threshold
   if( m_maxBasalticCrustThickness == 0 ) {
      m_TF_onset_mig = m_TF_onset;
   } else {
      m_TF_onset_mig = m_TF_onset + ((1 - m_TF_onset) * sqrt(2000 / m_maxBasalticCrustThickness));
   }
   if( m_decayConstant == 0 ) { m_decayConstant = 1; }
      
   m_magmaticDensity = m_E + (m_F - m_E) * (1 - exp( -1 * m_maxBasalticCrustThickness / m_decayConstant));
   
   // Step 4.3
   if( m_T0Map->getValue( i, j ) == m_T0Map->getUndefinedValue() || m_TRMap->getValue( i, j ) == m_TRMap->getUndefinedValue()) return false;
   const double t_mr = (m_T0Map->getValue(i, j) + m_TRMap->getValue(i, j)) / 2; 
   
   if( t_mr < 0 ) return false; // simple check if input data is valid 
   
   const double expValue = 1 - exp(-15 * t_mr / m_tau);

   m_WLS_exhume = m_initialSubsidenceMax + m_E0 * (( 1 - exp(- t_mr / m_tau)) + (pi2_8 - 1) * expValue );
   m_WLS_crit =  m_WLS_exhume - m_maxBasalticCrustThickness * (m_backstrippingMantleDensity - m_magmaticDensity) / (m_backstrippingMantleDensity - m_waterDensity);
   
   theFunction.setWLS_crit( m_WLS_crit );
   
   const double r = ( m_TF_onset_lin == 1.0 ? 1.0 : sin(m_pi * (1 - m_TF_onset_lin)) /(m_pi * (1 - m_TF_onset_lin)));
   m_WLS_onset = m_TF_onset_lin * m_initialSubsidenceMax + m_E0 * (r * (1 - exp(- t_mr / m_tau)) + (pi2_8 * m_TF_onset_lin - r) * expValue );
   
   //  if( m_WLS_crit < m_WLS_onset )  return false;

   theFunction.setWLS_onset( m_WLS_onset );
   
   m_WLS_exhume_serp = m_WLS_exhume - 681.6394;
   
  // Step 4.4
  const double m1 = m_TF_onset_lin / m_WLS_onset; // form Y = m1 * X
  const double m2 = ( m_WLS_crit == m_WLS_onset ? 0 : (1 -  m_TF_onset_lin) / ( m_WLS_crit - m_WLS_onset )); // form Y = m2 * X + c2
  const double c2 = m_TF_onset_lin - m_WLS_onset * m2;

  if( m_maxBasalticCrustThickness != 0 && ( m_backstrippingMantleDensity - m_magmaticDensity ) == 0.0 ) {
      string s = "MeSsAgE ERROR Backstripping Mantle density == magmatic density";
      throw s;
  }     
  const double magmaThicknessCoeff = (m_backstrippingMantleDensity - m_waterDensity) / (m_backstrippingMantleDensity - m_magmaticDensity);

  theFunction.setM1( m1 );
  theFunction.setM2( m2 );
  theFunction.setC2( c2 );
  theFunction.setMagmaThicknessCoeff( magmaThicknessCoeff );
  theFunction.setMaxBasalticCrustThickness( m_maxBasalticCrustThickness );
   
  return true;
}
//------------------------------------------------------------//
void InterfaceInput::retrieveData() {

   m_T0Map->retrieveData();
   m_TRMap->retrieveData();
   m_HCuMap->retrieveData();
   m_HLMuMap->retrieveData();
   m_HBuMap->retrieveData();
   m_DeltaSLMap->retrieveData();
}
//------------------------------------------------------------//
void InterfaceInput::restoreData() {

   m_T0Map->restoreData();
   m_TRMap->restoreData();
   m_HCuMap->restoreData();
   m_HLMuMap->restoreData();
   m_HBuMap->restoreData();
   m_DeltaSLMap->restoreData();
}
//------------------------------------------------------------//
namespace CrustalThicknessInterface {

void parseLine(const string &theString, const string &theDelimiter, vector<string> &theTokens)
{
   string::size_type startPos = 0;
   string::size_type endPos = 0;

   string::size_type increment = 0;
   string Token;

   if(theString.empty() || theDelimiter.empty()) {
      return;
   }
   while( endPos != string::npos ) {
      endPos = theString.find_first_of( theDelimiter, startPos );
      increment = endPos - startPos;

      Token = theString.substr( startPos, increment );
      if( Token.size() != 0 ) {
         theTokens.push_back( Token );
      }
      startPos += increment + 1;
   }
}

//------------------------------------------------------------//
void LoadALCParameters( ifstream &ConfigurationFile, double &HLmin, int &HLMEmax ) {

   string line;
   vector<string> theTokens;
   string delim = ",";
 
   HLmin = 100000.0;
   HLMEmax = 100;
   
   size_t firstNotSpace;
      
   while( !ConfigurationFile.eof() ) {
         
      getline ( ConfigurationFile, line, '\n' );
      if( line.size() != 0 ) { 
         firstNotSpace = line.find_first_not_of(" "); 
         
         if( line[firstNotSpace] != '#' ) {
            
            if( line == CrustalThicknessInterface::TableLithoAndCrustProperties || line.find( CrustalThicknessInterface::TableLithoAndCrustProperties, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
                  if( theTokens.size() == 2 ) {
                     if( theTokens[0] == CrustalThicknessInterface::lithosphereThicknessMin ) {
                        HLmin = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == CrustalThicknessInterface::maxNumberOfMantleElements ) {
                        HLMEmax = atoi( theTokens[1].c_str() );
                     }
                     
                  } else {
                     theTokens.clear();
                     string s = "More or less arguments than expected.";
                     throw s;
                  }
                  theTokens.clear();
               }
            }
         }
      }
   }
}

}
