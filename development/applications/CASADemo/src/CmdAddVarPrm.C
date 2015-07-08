//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "CasaCommander.h"
#include "CmdAddVarPrm.h"
#include "UndefinedValues.h"
#include "CfgFileParser.h"

#include "casaAPI.h"

#include <cstdlib>
#include <iostream>


static casa::VarPrmContinuous::PDF Str2pdf( const std::string & pdf )
{
   if (      pdf == "Block"    ) return casa::VarPrmContinuous::Block;    // equal PDF
   else if ( pdf == "Triangle" ) return casa::VarPrmContinuous::Triangle; // triangle PDF
   else if ( pdf == "Normal"   ) return casa::VarPrmContinuous::Normal;   // gauss PDF

   return casa::VarPrmContinuous::Block;
}



CmdAddVarPrm::CmdAddVarPrm( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No name of variable parameter was given";
   }

   if ( m_prms[0] != "TopCrustHeatProduction"
     && m_prms[0] != "SourceRockTOC"
     && m_prms[0] != "SourceRockHC"
     && m_prms[0] != "SourceRockHI"
     && m_prms[0] != "SourceRockType"
     && m_prms[0] != "SourceRockPreasphActEnergy"
     && m_prms[0] != "CrustThinningOneEvent"
     && m_prms[0] != "PorosityModel"
     && m_prms[0] != "STPThermalCondCoeff" 
      )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Unknown variable parameter name: " << m_prms[0];
   }
   // check number of command parameters for var parameter
   if ( m_prms[0] == "TopCrustHeatProduction"     && m_prms.size() !=  4 ||
        m_prms[0] == "SourceRockTOC"              && m_prms.size() !=  5 ||
        m_prms[0] == "SourceRockHC"               && m_prms.size() !=  5 ||
        m_prms[0] == "SourceRockHI"               && m_prms.size() !=  5 ||
        m_prms[0] == "SourceRockType"             && m_prms.size() !=  4 ||
        m_prms[0] == "SourceRockPreasphActEnergy" && m_prms.size() !=  5 ||
        m_prms[0] == "CrustThinningOneEvent"      && m_prms.size() != 10 ||
        m_prms[0] == "PorosityModel"              &&(m_prms.size()  <  8 || m_prms.size() > 12) ||
        m_prms[0] == "STPThermalCondCoeff"        && m_prms.size() !=  5 
      )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << m_prms[0];
   }
}

void CmdAddVarPrm::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Add variable parameter: " << m_prms[0] << "(";
      for ( size_t i = 1; i < m_prms.size(); ++i )
      {
         std::cout << m_prms[i] << ((i == m_prms.size() - 1) ? "" : ",");
      }
      std::cout << ")" << std::endl;
   }

   casa::VarPrmContinuous::PDF ppdf = casa::VarPrmContinuous::Block;

   if ( m_prms[0] == "TopCrustHeatProduction" )
   {
      double minVal = atof( m_prms[1].c_str() );
      double maxVal = atof( m_prms[2].c_str() );

      ppdf = Str2pdf( m_prms[3] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryTopCrustHeatProduction( *sa.get(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "SourceRockTOC" )
   {
      double minVal = atof( m_prms[2].c_str() );
      double maxVal = atof( m_prms[3].c_str() );

      ppdf = Str2pdf( m_prms[4] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockTOC( *sa.get(), m_prms[1].c_str(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "SourceRockHC" )
   {
      double minVal = atof( m_prms[2].c_str() );
      double maxVal = atof( m_prms[3].c_str() );

      ppdf = Str2pdf( m_prms[4] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockHC( *sa.get(), m_prms[1].c_str(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "SourceRockHI" )
   {
      double minVal = atof( m_prms[2].c_str() );
      double maxVal = atof( m_prms[3].c_str() );

      ppdf = Str2pdf( m_prms[4] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockHI( *sa.get(), m_prms[1].c_str(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "SourceRockType" )
   {
      std::string layerName = m_prms[1];
      std::vector<std::string> srtList    = CfgFileParser::list2array( m_prms[2], ',' );
      std::vector<double>      srtWeights = CfgFileParser::set2array(  m_prms[3], ',' );
      if (ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockType( *sa.get()
                                                                                  , layerName.c_str()
                                                                                  , srtList
                                                                                  , srtWeights
                                                                                  ) )
      {
         throw ErrorHandler::Exception(sa->errorCode()) << sa->errorMessage();
      }
   }   
   else if ( m_prms[0] == "SourceRockPreasphActEnergy" )
   {
      double minVal = atof( m_prms[2].c_str() );
      double maxVal = atof( m_prms[3].c_str() );

      ppdf = Str2pdf( m_prms[4] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockPreAsphaltActEnergy( *sa.get(), m_prms[1].c_str(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "CrustThinningOneEvent" )
   {
      // Initial crustal thickness
      double minCrustThickn = atof( m_prms[1].c_str() );
      double maxCrustThickn = atof( m_prms[2].c_str() );

      // Start thinning time
      double minTStart = atof( m_prms[3].c_str() );
      double maxTStart = atof( m_prms[4].c_str() );

      // Thinning duration
      double minDeltaT = atof( m_prms[5].c_str() );
      double maxDeltaT = atof( m_prms[6].c_str() );

      double minFactor = atof( m_prms[7].c_str() );
      double maxFactor = atof( m_prms[8].c_str() );

      casa::VarPrmContinuous::PDF pdfType = Str2pdf( m_prms[9] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryOneCrustThinningEvent( *sa.get()
                                                                                          , minCrustThickn
                                                                                          , maxCrustThickn
                                                                                          , minTStart
                                                                                          , maxTStart
                                                                                          , minDeltaT
                                                                                          , maxDeltaT
                                                                                          , minFactor
                                                                                          , maxFactor
                                                                                          , pdfType
                                                                                          )
         ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   }
   else if ( m_prms[0] == "PorosityModel" )
   {
      size_t pos = 1;
      std::string litName   = m_prms[pos++];
      std::string modelName = m_prms[pos++];
      double minSurfPor     = atof( m_prms[pos++].c_str() );
      double maxSurfPor     = atof( m_prms[pos++].c_str() );
      double minCompCoef    = atof( m_prms[pos++].c_str() );
      double maxCompCoef    = atof( m_prms[pos++].c_str() );

      double minMinPor = UndefinedDoubleValue;
      double maxMinPor = UndefinedDoubleValue;

      double minCompCoef1 = UndefinedDoubleValue; 
      double maxCompCoef1 = UndefinedDoubleValue; 

      if ( m_prms.size() == 12 )
      {
         minMinPor    = atof( m_prms[pos++].c_str() );
         maxMinPor    = atof( m_prms[pos++].c_str() );
         minCompCoef1 = atof( m_prms[pos++].c_str() ); 
         maxCompCoef1 = atof( m_prms[pos++].c_str() ); 
      }
      else if ( m_prms.size() != 8 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << m_prms[0];
      }
      casa::VarPrmContinuous::PDF pdfType = Str2pdf( m_prms.back() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryPorosityModelParameters( *sa.get()
                                                                                            , litName.c_str()
                                                                                            , modelName.c_str()
                                                                                            , minSurfPor
                                                                                            , maxSurfPor
                                                                                            , minCompCoef
                                                                                            , maxCompCoef
                                                                                            , minMinPor
                                                                                            , maxMinPor
                                                                                            , minCompCoef1
                                                                                            , maxCompCoef1
                                                                                            , pdfType
                                                                                            )
         )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
   else if ( m_prms[0] == "STPThermalCondCoeff" )
   {
      double minVal = atof( m_prms[2].c_str() );
      double maxVal = atof( m_prms[3].c_str() );

      ppdf = Str2pdf( m_prms[4] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryLithoSTPThermalCondCoeffParameter( *sa.get(), m_prms[1].c_str(), minVal, maxVal, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
}

void CmdAddVarPrm::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <variable parameter type name> <min value> <max value> <pdf of parameter>\n\n";
   std::cout << "  Variable parameter - a parameter in Cauldron project file which exact value is unknown.\n";
   std::cout << "  There are only some estimations on it value range. For example - source rock TOC - [5:20]%.\n";
   std::cout << "  To define the variable parameter user should specify parameter type name and parameter range min/max values\n\n";

   std::cout << "  The following list of variable parameters is implemented for this command:\n";
   std::cout << "    TopCrustHeatProduction     - surface radiogenic heat production of the basement [ uW/m^3].\n";
   std::cout << "\n  (Source rock parameters variation:)\n";
   std::cout << "    SourceRockType             - categorical parameter which could variate source rock type for the layer.\n";
   std::cout << "                                 This parameter must be defined BEFORE any other SorceRock parameter\n";
   std::cout << "    SourceRockTOC              - the initial total organic content in source rock [ weight % ].\n";
   std::cout << "    SourceRockHC               - the initial H/C ratio in source rock [ kg/tonne C ].\n";
   std::cout << "    SourceRockHI               - the initial hydrogen index (HI) ratio in source rock [ kg/tonne ].\n";
   std::cout << "    SourceRockPreasphActEnergy - the activation energy limit for which the pre-asphalt cracking starts [ kJ/mol ].\n";
   std::cout << "\n  (Crust thinning variation:)\n";
   std::cout << "    CrustThinningOneEvent      - a crust thickness function with one crust thinning event.\n";
   std::cout << "\n  (Lithology parameters variation:)\n";
   std::cout << "    PorosityModel              - a variation of porosity model parameter for the given lithology.\n";
   std::cout << "    STPThermalCondCoeff        - a variation of STP (Standart P & T) thermal conductivity coefficient for the given lithology.\n";
   std::cout << "\n";
   std::cout << "\n";
   std::cout << "    TopCrustHeatProduction  <minVal> <maxVal> <prmPDF>\n";
   std::cout << "    Where:\n";
   std::cout << "       minVal - the parameter minimal range value\n";
   std::cout << "       maxVal - the parameter maximal range value\n";
   std::cout << "       prmPDF - the parameter probability density function type, the value could be one of the following:\n";
   std::cout << "                \"Block\"    - uniform probability between min and max values,\n";
   std::cout << "                \"Triangle\" - triangle shape probability function. The top triangle value is taken from the base case\n";
   std::cout << "                \"Normal\"   - normal (or Gaussian) probability function. The position of highest value is taken from the base case\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #              type               minVal  maxVal    prmPDF\n";
   std::cout << "    " << cmdName << " \"TopCrustHeatProduction\"    0.1     4.9  \"Block\"\n";
   std::cout << "\n";
   std::cout << "    SourceRockTOC  <layerName> <minVal> <maxVal> <prmPDF>\n";
   std::cout << "    Where:\n";
   std::cout << "       layerName - source rock layer name\n";
   std::cout << "       minVal    - the parameter minimal range value\n";
   std::cout << "       maxVal    - the parameter maximal range value\n";
   std::cout << "       prmPDF    - the parameter probability density function type\n";
   std::cout << "\n";
   std::cout << "    SourceRockHC  <layerName> <minVal> <maxVal> <prmPDF>\n";
   std::cout << "    Where:\n";
   std::cout << "       layerName - source rock layer name\n";
   std::cout << "       minVal    - the parameter minimal range value\n";
   std::cout << "       maxVal    - the parameter maximal range value\n";
   std::cout << "       prmPDF    - the parameter probability density function type\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #             type         layerName        minVal  maxVal   prmPDF\n";
   std::cout << "    "<< cmdName << " \"SourceRockHC\" \"Lower Jurassic\"   0.5    1.0  \"Block\"\n";
   std::cout << "\n";
   std::cout << "    SourceRockHI  <layerName> <minVal> <maxVal> <prmPDF>\n";
   std::cout << "    Where:\n";
   std::cout << "       layerName - source rock layer name\n";
   std::cout << "       minVal    - the parameter minimal range value\n";
   std::cout << "       maxVal    - the parameter maximal range value\n";
   std::cout << "       prmPDF    - the parameter probability density function type\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #             type         layerName        minVal  maxVal   prmPDF\n";
   std::cout << "    " << cmdName << " \"SourceRockHI\" \"Lower Jurassic\"    433.5    521.0  \"Block\"\n";
   std::cout << "\n";
   std::cout << "    SourceRockType  <layerName> \"SRType1,SRType2,SRType3\" [w1,w2,w3 ]\n";
   std::cout << "    Where:\n";
   std::cout << "       layerName - source rock layer name\n";
   std::cout << "       SRtype    - comma separated list of source rock types for variation\n";
   std::cout << "       w1-w2     - comma separated list of source rock types weights\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #             type         layerName          category list             categories weight\n";
   std::cout << "    "<< cmdName << " \"SourceRockType\" \"Lower Jurassic\"  \"Type1,Type2,Type3\" [0.8, 0.1, 0.1]\n";
   std::cout << "\n";
   std::cout << "    SourceRockPreasphActEnergy  <layerName> <minVal> <maxVal> <prmPDF>\n";
   std::cout << "    Where:\n";
   std::cout << "       layerName - source rock layer name\n";
   std::cout << "       minVal    - the parameter minimal range value\n";
   std::cout << "       maxVal    - the parameter maximal range value\n";
   std::cout << "       prmPDF    - the parameter probability density function type\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #             type         layerName        minVal  maxVal   prmPDF\n";
   std::cout << "    " << cmdName << " \"SourceRockPreasphActEnergy\" \"Lower Jurassic\"    204.0  206.0  \"Block\"\n";
   std::cout << "\n";
   std::cout << "    CrustThinningOneEvent <IniCrstThickMn> <IniCrstThickMx> <mnT0> <mxT0> <mndT> <mxdT> <mnFact> <mxFct> <prmPDF>\n";
   std::cout << "    Where:\n";
   std::cout << "       IniCrstThickMn - initial crust thickness - minimal range value\n";
   std::cout << "       IniCrstThickMx - initial crust thickness - maximal range value\n";
   std::cout << "       mnT0           - crust thinning event start time - minimal range value\n";
   std::cout << "       mxT0           - crust thinning event start time - maximal range value\n";
   std::cout << "       mnVact         - crust thickness factor - minimal range value\n";
   std::cout << "       mxVact         - crust thickness factor - maximal range value\n";
   std::cout << "       prmPDF         - the parameter probability density function type\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #                                InCrThick   T0       dT    ThinFct  PDF\n";
   std::cout << "    " << cmdName << " \"CrustThinningOneEvent\" 15000 40000 120 180 30 45   0.5 0.8 \"Block\"\n";
   std::cout << "\n";
   std::cout << "    PorosityModel <lithologyName> <porModelName> <mnSurfPor> <mxSurfPor> <mnCmpCf> <mxCmpCf> [<mnMinPor> <mxMinPor> <mnCmpCf1> <mxCmpCf1>] <prmPDF>\n";
   std::cout << "    Where:\n";
   std::cout << "       lithologyName - lithology name\n";
   std::cout << "       porModelName  - porosity model name, allowed values: Exponential, Soil_Mechanics, Double_Exponential\n";
   std::cout << "       mnSurfPor     - surface porosity - minimal range value\n";
   std::cout << "       mxSurfPor     - surface porosity - maximal range value\n";
   std::cout << "       mnCmpCf       - compaction coefficient - minimal range value\n";
   std::cout << "       mxCmpCf       - compaction coefficient - maximal range value\n";
   std::cout << "       mnMinPor      - minimal porosity (for Double_Exponential model only) - minimal range value\n";
   std::cout << "       mxMinPor      - minimal porosity (for Double_Exponential model only) - maximal range value\n";
   std::cout << "       mnCmpCf1      - compaction coefficient for the second exponent (for Double_Exponential model only) - minimal range value\n";
   std::cout << "       mxCmpCf1      - compaction coefficient for the second exponent (for Double_Exponential model only) - maximal range value\n";
   std::cout << "       prmPDF        - the parameter probability density function type\n";
   std::cout << "\n";
   std::cout << "    Note: for the Soil_Mechanics model only one parameter variation is possible, the second one should has same values for min/max and will be ignored\n\n";
   std::cout << "    Example 1:\n";
   std::cout << "    #       VarPrmName      LithName             PorModel       SurfPor [%]  CompCoeff  Parameter PDF\n";
   std::cout << "    " << cmdName << "  \"PorosityModel\" \"SM.Mudstone40%Clay\" \"Exponential\"  15 85        7.27 7.27  \"Block\"\n";
   std::cout << "\n";
   std::cout << "    Example 2:\n";
   std::cout << "    #      VarPrmName      LithName              PorModel          SurfPor [%]  CompCoeff      Parameter PDF\n";
   std::cout << "    " << cmdName << " \"PorosityModel\" \"SM.Mudstone40%Clay\"  \"Soil_Mechanics\"  15 85        0.1988 0.1988  \"Block\"\n";
   std::cout << "\n";
   std::cout << "    STPThermalCondCoeff <lithologyName> <minValue> <maxValue> <prmPDF>\n";
   std::cout << "       lithologyName - lithology name\n";
   std::cout << "       minVal    - the parameter minimal range value\n";
   std::cout << "       maxVal    - the parameter maximal range value\n";
   std::cout << "       prmPDF    - the parameter probability density function type\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #       VarPrmName             LithName           min max  Parameter PDF\n";
   std::cout << "    " << cmdName << " \"STPThermalCondCoeff\"  \"SM.Mudstone40%Clay\" 2   4   \"Block\"\n";
   std::cout << "\n";
}

