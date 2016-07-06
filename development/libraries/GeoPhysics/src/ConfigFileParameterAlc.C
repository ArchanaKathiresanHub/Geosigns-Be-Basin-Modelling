//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ConfigFileParameter.h"
#include "ConfigFileParameterAlc.h"

#include <sstream>

//utilities
#include "StringHandler.h"
#include "LogHandler.h"

using namespace GeoPhysics;

//------------------------------------------------------------//
ConfigFileParameterAlc::ConfigFileParameterAlc() {
   clean();
}

//------------------------------------------------------------//
bool ConfigFileParameterAlc::loadConfigurationFileAlc( std::ifstream &ConfigurationFile ) {
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   
   clean();

   size_t firstNotSpace;
      
   while( !ConfigurationFile.eof() ) {
         
      getline ( ConfigurationFile, line, '\n' );
      if( line.size() != 0 ) { 
         firstNotSpace = line.find_first_not_of(" "); 
         
         if( line[firstNotSpace] != '#' ) {
            
            if( line == ConfigFileAlcCtc::TableLithoAndCrustProperties || line.find( ConfigFileAlcCtc::TableLithoAndCrustProperties, 0) != std::string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  StringHandler::parseLine( line, delim, theTokens );
      
                  if( theTokens.size() == 2 ) {
                     if( theTokens[0] == ConfigFileAlcCtc::lithosphereThicknessMin ) {
                        m_HLmin = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::maxNumberOfMantleElements ) {
                        m_NLMEmax = atoi( theTokens[1].c_str() );
                     }
                     
                  } else {
                     theTokens.clear();
                     LogHandler( LogHandler::ERROR_SEVERITY ) << "More or less arguments than expected in ALC-CTC configuration file for table " << ConfigFileAlcCtc::TableLithoAndCrustProperties;
                     return false;
                  }
                  theTokens.clear();
               }
            } else if( line == ConfigFileAlcCtc::TableStandardCrust || line.find( ConfigFileAlcCtc::TableStandardCrust, 0) != std::string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  StringHandler::parseLine( line, delim, theTokens );
      
                  if( theTokens.size() == 2 ) {
                     if( theTokens[0] == ConfigFileAlcCtc::Rho ) {
                        m_csRho = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::A ) {
                        m_csA = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::B ) {
                        m_csB = atof( theTokens[1].c_str() );
                     }
                     
                  } else {
                     theTokens.clear();
                     LogHandler( LogHandler::ERROR_SEVERITY ) << "More or less arguments than expected in ALC-CTC configuration file for table " << ConfigFileAlcCtc::TableStandardCrust;
                     return false;
                  }
                  theTokens.clear();
               }
             } else if( line == ConfigFileAlcCtc::TableLowCondCrust || line.find( ConfigFileAlcCtc::TableLowCondCrust, 0) != std::string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  StringHandler::parseLine( line, delim, theTokens );
      
                  if( theTokens.size() == 2 ) {
                     if( theTokens[0] == ConfigFileAlcCtc::Rho ) {
                        m_clRho = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::A ) {
                        m_clA = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::B ) {
                        m_clB = atof( theTokens[1].c_str() );
                     }
                       
                  } else {
                     theTokens.clear();
                     LogHandler( LogHandler::ERROR_SEVERITY ) << "More or less arguments than expected in ALC-CTC configuration file for table " << ConfigFileAlcCtc::TableLowCondCrust;
                     return false;
                  }
                  theTokens.clear();
               }
             } else if( line == ConfigFileAlcCtc::TableBasalt || line.find( ConfigFileAlcCtc::TableBasalt, 0) != std::string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  StringHandler::parseLine( line, delim, theTokens );
      
                  if( theTokens.size() == 2 ) {
                     if( theTokens[0] == ConfigFileAlcCtc::Rho ) {
                        m_bRho = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::A ) {
                        m_bA = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::B ) {
                        m_bB = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::T ) {
                        m_bT = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::Heat ) {
                        m_bHeat = atof( theTokens[1].c_str() );
                     }
                       
                  } else {
                     theTokens.clear();
                     LogHandler( LogHandler::ERROR_SEVERITY ) << "More or less arguments than expected in ALC-CTC configuration file for table " << ConfigFileAlcCtc::TableBasalt;
                     return false;
                  }
                  theTokens.clear();
               }
             } else if( line == ConfigFileAlcCtc::TableMantle || line.find( ConfigFileAlcCtc::TableMantle, 0) != std::string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  StringHandler::parseLine( line, delim, theTokens );
      
                  if( theTokens.size() == 2 ) {
                     if( theTokens[0] == ConfigFileAlcCtc::Rho ) {
                        m_mRho = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::A ) {
                        m_mA = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == ConfigFileAlcCtc::B ) {
                        m_mB = atof( theTokens[1].c_str() );
                     } 
                      
                  } else {
                     theTokens.clear();
                     LogHandler( LogHandler::ERROR_SEVERITY ) << "More or less arguments than expected in ALC-CTC configuration file for table " << ConfigFileAlcCtc::TableMantle;
                     return false;
                  }
                  theTokens.clear();
               }
             }
         }
      }
   } 

   return true;
}
   
//------------------------------------------------------------//
void ConfigFileParameterAlc::clean() {
   
   m_csRho = 0;
   m_clRho = 0;
   m_bRho  = 0;
   m_mRho  = 0;
   m_csA   = 0;
   m_csB   = 0;
   m_clA   = 0;
   m_clB   = 0;
   m_bA    = 0;
   m_bB    = 0;
   m_mA    = 0;
   m_mB    = 0;
   m_bT    = 0;
   m_bHeat = 0;

   m_HLmin   = 0;
   m_NLMEmax = 0;

}
//------------------------------------------------------------//
ConfigFileParameterAlc& ConfigFileParameterAlc::operator=(const ConfigFileParameterAlc& toCopy) {
   
   m_csRho = toCopy.m_csRho;
   m_clRho = toCopy.m_clRho;
   m_bRho  = toCopy.m_bRho;
   m_mRho  = toCopy.m_mRho;
   m_csA   = toCopy.m_csA;
   m_csB   = toCopy.m_csB;
   m_clA   = toCopy.m_clA;
   m_clB   = toCopy.m_clB;
   m_bA    = toCopy.m_bA;
   m_bB    = toCopy.m_bB;
   m_mA    = toCopy.m_mA;
   m_mB    = toCopy.m_mB;
   m_bT    = toCopy.m_bT;
   m_bHeat = toCopy.m_bHeat;
   
   m_HLmin   = toCopy.m_HLmin;
   m_NLMEmax = toCopy.m_NLMEmax;
   
   return * this;
}
