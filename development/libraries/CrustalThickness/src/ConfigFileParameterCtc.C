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
#include "ConfigFileParameterCtc.h"

// file system
#include "FilePath.h"

// std library
#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>

// utilities
#include "StringHandler.h"
#include "LogHandler.h"

using namespace CrustalThickness;

//------------------------------------------------------------//
ConfigFileParameterCtc::ConfigFileParameterCtc() {
   clean();
}

//------------------------------------------------------------//
bool ConfigFileParameterCtc::loadConfigurationFileCtc( const std::string &inFile ) {
   char * CTCDIR = getenv( "CTCDIR" );
   char * MY_CTCDIR = getenv( "MY_CTCDIR" );

   std::string ctcdir;

   if (MY_CTCDIR != 0) { ctcdir = MY_CTCDIR; }
   else if (CTCDIR != 0) { ctcdir = CTCDIR; }
   else {
      throw ConfigFileCtcException() << "Environment Variable CTCDIR is not set. Aborting...";
   }

   ibs::FilePath fullpath( ctcdir );
   fullpath << inFile;

   std::ifstream  ConfigurationFile;
   ConfigurationFile.open( fullpath.cpath() );

   if (!ConfigurationFile) {
      throw ConfigFileCtcException() << "Attempting to open configuration file " << fullpath.path() << " but the file does not exist... Aborting... ";
   }

   std::string line;
   size_t firstNotSpace;

   while (!ConfigurationFile.eof()) {

      getline( ConfigurationFile, line, '\n' );
      if (line.size() != 0) {
         firstNotSpace = line.find_first_not_of( " \t" );

         if (line[firstNotSpace] != '#') {

            if (line == ConfigFileAlcCtc::TableBasicConstants || line.find( ConfigFileAlcCtc::TableBasicConstants, 0 ) != std::string::npos) {

               loadBasicConstants( ConfigurationFile );

            }
            else if (line == ConfigFileAlcCtc::TableLithoAndCrustProperties || line.find( ConfigFileAlcCtc::TableLithoAndCrustProperties, 0 ) != std::string::npos) {

               loadLithoAndCrustProperties( ConfigurationFile );

            }
            else if (line == ConfigFileAlcCtc::TableTemperatureData || line.find( ConfigFileAlcCtc::TableTemperatureData, 0 ) != std::string::npos) {

               loadTemperatureData( ConfigurationFile );

            }
            else if (line == ConfigFileAlcCtc::TableSolidus || line.find( ConfigFileAlcCtc::TableSolidus, 0 ) != std::string::npos) {

               loadSolidus( ConfigurationFile );

            }
            else if (line == ConfigFileAlcCtc::TableMagmaLayer || line.find( ConfigFileAlcCtc::TableMagmaLayer, 0 ) != std::string::npos) {

               loadMagmaLayer( ConfigurationFile );

            }
         }
      }
   }
   ConfigurationFile.close();
}

//------------------------------------------------------------//
void ConfigFileParameterCtc::clean() {
   //-------------- Basic constants ---------------------
   m_coeffThermExpansion  = 0.0;
   m_initialSubsidenceMax = 0.0;
   m_pi                   = M_PI;
   m_E0                   = 0.0;
   m_tau                  = 0.0;

   //-------------- Lithospphere and crust properties ---------------------
   m_modelTotalLithoThickness    = 0.0;
   m_backstrippingMantleDensity  = 0.0;
   m_lithoMantleDensity          = 0.0;
   m_baseLithosphericTemperature = 0.0;
   m_referenceCrustThickness     = 0.0;
   m_referenceCrustDensity       = 0.0;
   m_waterDensity                = 0.0;

   //------------- Asthenosphere potential temperature data ---------------------
   m_A = 0.0;
   m_B = 0.0;

   //------------- Solidus (onset of adiabatic melting) ---------------------
   m_C = 0.0;
   m_D = 0.0;

   //------------- Magma-layer density ---------------------
   m_E             = 0.0;
   m_F             = 0.0;
   m_decayConstant = 0.0;
}

//------------------------------------------------------------//
ConfigFileParameterCtc& ConfigFileParameterCtc::operator=(const ConfigFileParameterCtc& toCopy) {

   //-------------- Basic constants ---------------------
   m_coeffThermExpansion  = toCopy.getCoeffThermExpansion();
   m_initialSubsidenceMax = toCopy.getInitialSubsidenceMax();
   m_pi                   = toCopy.getPi();
   m_E0                   = toCopy.getE0();
   m_tau                  = toCopy.getTau();

   //-------------- Lithospphere and crust properties ---------------------
   m_modelTotalLithoThickness    = toCopy.getModelTotalLithoThickness();
   m_backstrippingMantleDensity  = toCopy.getBackstrippingMantleDensity();
   m_lithoMantleDensity          = toCopy.getLithoMantleDensity();
   m_baseLithosphericTemperature = toCopy.getBaseLithosphericTemperature();
   m_referenceCrustThickness     = toCopy.getReferenceCrustThickness();
   m_referenceCrustDensity       = toCopy.getReferenceCrustDensity();
   m_waterDensity                = toCopy.getWaterDensity();

   //------------- Asthenosphere potential temperature data ---------------------
   m_A = toCopy.getA();
   m_B = toCopy.getB();

   //------------- Solidus (onset of adiabatic melting) ---------------------
   m_C = toCopy.getC();
   m_D = toCopy.getD();

   //------------- Magma-layer density ---------------------
   m_E             = toCopy.getE();
   m_F             = toCopy.getF();
   m_decayConstant = toCopy.getDecayConstant();

   return *this;
}

//------------------------------------------------------------//
void ConfigFileParameterCtc::loadBasicConstants( std::ifstream &ConfigurationFile ) {

   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   size_t firstNotSpace;
   int countParam = 0;

   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
         break;
      }
      firstNotSpace = line.find_first_not_of(" \t"); 
      
      if( line[firstNotSpace] != '#' ) {
      
         StringHandler::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {
         if( theTokens[0] == ConfigFileAlcCtc::coeffThermExpansion ) {
            
            m_coeffThermExpansion = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::initialSubsidenceMax ) {
            
            m_initialSubsidenceMax = atof( theTokens[1].c_str() );
               ++ countParam;
            
            } else if( theTokens[0] == ConfigFileAlcCtc::E0 ) {
            
            m_E0 = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::tau ) {
            
            m_tau = atof( theTokens[1].c_str() );
               ++ countParam;
         }  
            else{
               LogHandler( LogHandler::WARNING_SEVERITY ) << "CTC configuration file BasicConstants table: unknown CTC parameter '" << theTokens[0] << "'.";
            }
         }
         else {
         theTokens.clear();
            throw ConfigFileCtcException() << "CTC configuration file BasicConstants table: unexpected parameter definition (should be Name, Value).";
      }
      theTokens.clear();
   }
   }
   if( countParam != 4 ) {
      throw ConfigFileCtcException() << "CTC configuration file BasicConstants table: 4 parameters expected but only " << countParam << " found.";
   }
   if( m_tau == 0 ) {
      throw ConfigFileCtcException() << "CTC configuration file BasicConstants table: Tau=0 but should be !=0.";
   }
 
}

//------------------------------------------------------------//
void ConfigFileParameterCtc::loadLithoAndCrustProperties( std::ifstream &ConfigurationFile ) {

   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   size_t firstNotSpace;
   int countParam = 0;
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
         break;
      }
      firstNotSpace = line.find_first_not_of(" \t"); 
      
      if( line[firstNotSpace] != '#' ) {
      
         StringHandler::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {
         if( theTokens[0] == ConfigFileAlcCtc::modelTotalLithoThickness ) {

            m_modelTotalLithoThickness = atof( theTokens[1].c_str() );
               ++ countParam;

         }  else if( theTokens[0] == ConfigFileAlcCtc::backstrippingMantleDensity ) {

            m_backstrippingMantleDensity = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::lithoMantleDensity ) {

            m_lithoMantleDensity = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::baseLithosphericTemperature ) {

               ++ countParam;
            m_baseLithosphericTemperature = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == ConfigFileAlcCtc::referenceCrustThickness ) {

            m_referenceCrustThickness = atof( theTokens[1].c_str() );
               ++ countParam;

         }  else if( theTokens[0] == ConfigFileAlcCtc::referenceCrustDensity ) {

            m_referenceCrustDensity = atof( theTokens[1].c_str() );
               ++ countParam;

         }  else if( theTokens[0] == ConfigFileAlcCtc::waterDensity ) {

            m_waterDensity = atof( theTokens[1].c_str() );
               ++ countParam;
         } 
            // lithosphereThicknessMin and maxNumberOfMantleElements will be defined in BasementLithology.C so they should not return a warning
            else if (theTokens[0] != ConfigFileAlcCtc::lithosphereThicknessMin
                     and theTokens[0] != ConfigFileAlcCtc::maxNumberOfMantleElements) {
               LogHandler( LogHandler::WARNING_SEVERITY ) << "CTC configuration file LithoAndCrustProperties table: unknown CTC parameter '" << theTokens[0] << "'.";
            }
      } else {
         theTokens.clear();
            throw ConfigFileCtcException() << "CTC configuration file LithoAndCrustProperties table: unexpected parameter definition (should be Name, Value).";
      }
      theTokens.clear();
   }
   }
   if( countParam != 7 ) {
      throw ConfigFileCtcException() << "CTC configuration file LithoAndCrustProperties table: 7 parameters expected but only " << countParam << " found.";
   }
   if( m_modelTotalLithoThickness == 0 ) {
      throw ConfigFileCtcException() << "CTC configuration file LithoAndCrustProperties table: TotalLithoThickness of the Model = 0 but should be !=0.";
   }
}

//------------------------------------------------------------//
void ConfigFileParameterCtc::loadTemperatureData( std::ifstream &ConfigurationFile ) {

   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   size_t firstNotSpace;
   int countParam = 0;
   
   for (;;) {
      getline( ConfigurationFile, line, '\n' );
        
      if (line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
         break;
      }
      
      firstNotSpace = line.find_first_not_of( " \t" );

      if (line[firstNotSpace] != '#') {

         StringHandler::parseLine( line, delim, theTokens );
      
         if (theTokens.size() == 2) {

            if (theTokens[0] == ConfigFileAlcCtc::A) {
               m_A = atof( theTokens[1].c_str() );
               ++countParam;
            }
            else if (theTokens[0] == ConfigFileAlcCtc::B) {
               m_B = atof( theTokens[1].c_str() );
               ++countParam;
            } 
            else{
               LogHandler( LogHandler::WARNING_SEVERITY ) << "CTC configuration file TemperatureData table: unknown CTC parameter '" << theTokens[0] << "'.";
            }

         }
         else {
            theTokens.clear();
            throw ConfigFileCtcException() << "CTC configuration file TemperatureData table: unexpected parameter definition (should be Name, Value).";
      }
      theTokens.clear();
   }
}
   if (countParam != 2) {
      throw ConfigFileCtcException() << "CTC configuration file TemperatureData table: 2 parameters expected but only " << countParam << " found.";
   }
}
//------------------------------------------------------------//
void ConfigFileParameterCtc::loadSolidus( std::ifstream &ConfigurationFile ) {

   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   size_t firstNotSpace;
   int countParam = 0;
   
   for (;;) {
      getline( ConfigurationFile, line, '\n' );
        
      if (line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
         break;
      }
      firstNotSpace = line.find_first_not_of( " \t" );
   
      if (line[firstNotSpace] != '#') {
      
         StringHandler::parseLine( line, delim, theTokens );
      
         if (theTokens.size() == 2) {

            if (theTokens[0] == ConfigFileAlcCtc::C) {
               m_C = atof( theTokens[1].c_str() );
               ++countParam;
            }
            else if (theTokens[0] == ConfigFileAlcCtc::D) {
               m_D = atof( theTokens[1].c_str() );
               ++countParam;
            } 
            else{
               LogHandler( LogHandler::WARNING_SEVERITY ) << "CTC configuration file Solidus table: unknown CTC parameter '" << theTokens[0] << "'.";
            }

         }
         else {
         theTokens.clear();
         throw ConfigFileCtcException() << "CTC configuration file Solidus table: unexpected parameter definition (should be Name, Value).";
      }
      theTokens.clear();
   }
}
   if (countParam != 2) {
      throw ConfigFileCtcException() << "CTC configuration file Solidus table: 2 parameters expected but only " << countParam << " found.";
   }
}
//------------------------------------------------------------//
void ConfigFileParameterCtc::loadMagmaLayer( std::ifstream &ConfigurationFile ) {

   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   size_t firstNotSpace;
   int countParam = 0;
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
         break;
      }
      
      firstNotSpace = line.find_first_not_of(" \t"); 
      
      if( line[firstNotSpace] != '#' ) {
         StringHandler::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {

         if( theTokens[0] == ConfigFileAlcCtc::E ) {

            m_E = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::F ) {

            m_F = atof( theTokens[1].c_str() );

         } else if( theTokens[0] == ConfigFileAlcCtc::decayConstant ) {

            m_decayConstant = atof( theTokens[1].c_str() );
               ++ countParam;

         } 
            else{
               LogHandler( LogHandler::WARNING_SEVERITY ) << "CTC configuration file MagmaLayer table: unknown CTC parameter '" << theTokens[0] << "'.";
            }
         }
         else {
         theTokens.clear();
            throw ConfigFileCtcException() << "CTC configuration file MagmaLayer table: unexpected parameter definition (should be Name, Value).";
      }
      theTokens.clear();
   }
}
   if( countParam != 2 ) {
      throw ConfigFileCtcException() << "CTC configuration file MagmaLayer table: 2 parameters expected but only " << countParam << " found.";
   }
}
