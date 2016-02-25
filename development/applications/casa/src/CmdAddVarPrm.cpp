//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// casa app
#include "CasaCommander.h"
#include "CmdAddVarPrm.h"
#include "UndefinedValues.h"
#include "CfgFileParser.h"

// CASA API
#include "casaAPI.h"

// LogHandler lib
#include "LogHandler.h"

// STD C
#include <cstdlib>

// STL
#include <iostream>
#include <map>

// convert PDF name as string to CASA PDF type
static casa::VarPrmContinuous::PDF Str2pdf( const std::string & pdf )
{
   if (      pdf == "Block"    ) return casa::VarPrmContinuous::Block;    // equal PDF
   else if ( pdf == "Triangle" ) return casa::VarPrmContinuous::Triangle; // triangle PDF
   else if ( pdf == "Normal"   ) return casa::VarPrmContinuous::Normal;   // gauss PDF

   return casa::VarPrmContinuous::Block;
}

///////////////////////////////////////////////////////////////////////////////
// Class which define interface for parameter object factory
class PrmType
{
public:
   /// @brief Destructor
   virtual ~PrmType() {;}

   /// @brief add variable parameter to scenario
   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const = 0;
   
   /// @brief Get expected parameters number for the observable type
   virtual size_t expectedParametersNumber() const = 0;
   /// @brief Get optional parameters number for the observable type
   virtual size_t optionalParametersNumber() const = 0;

   /// @brief Get varparameter type name
   virtual std::string name() const = 0;
   /// @brief Get varparameter group description
   virtual std::string groupDescirption() const { return ""; }
   /// @brief Get short description for observable type
   virtual std::string description() const = 0;
   /// @brief Get full description for observable type
   virtual std::string fullDescription() const = 0;
    /// @brief Get observable example for using in commands script file
   virtual std::string usingExample( const char * cmdName ) const = 0;

protected:
   PrmType( std::string tblColNames = "" )
   { 
      if ( tblColNames.empty() ) return;

      const std::vector<std::string> & lst = CfgFileParser::list2array( tblColNames, ':' );
      if ( lst.size() != 2 )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Wrong format for variable parameters name: " << 
            tblColNames << ", it must be the following format: tblName:colName";
      }
   }

   std::string m_tblName;
   std::string m_colName;
};

////////////////////////////////////////////////////////////////
/// Parameters type definitions
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// TopCrustHeatProduction parameter
////////////////////////////////////////////////////////////////
//
class TopCrustHeatProduction : public PrmType
{
public:
   TopCrustHeatProduction( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~TopCrustHeatProduction() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      std::vector<double>      simpleRange;
      std::vector<std::string> mapRange;
      // if parameters - 2 doubles - it is a simple range
      if ( CfgFileParser::isNumericPrm( prms[1] ) && CfgFileParser::isNumericPrm( prms[2] ) )
      {
         simpleRange.push_back( atof( prms[1].c_str() ) );
         simpleRange.push_back( atof( prms[2].c_str() ) );
      }
      else // otherwise consider it as a map range
      {
         mapRange.push_back( prms[1] );
         mapRange.push_back( prms[2] );
      }

      casa::VarPrmContinuous::PDF ppdf = Str2pdf( prms[3] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryTopCrustHeatProduction( *sa.get(), name.c_str(), simpleRange, mapRange, ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
      
   size_t expectedParametersNumber() const { return 3; } // min, max, pdf
   size_t optionalParametersNumber() const { return 0; } 

   virtual std::string name() const { return "BasementIoTbl:TopCrustHeatProd"; }

   virtual std::string description() const { return "surface radiogenic heat production rate of the basement [ uW/m^3]"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;

      oss << "    [varPrmName] \"BasementIoTbl:TopCrustHeatProd\" <minVal> <maxVal> <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName - user specified variable parameter name (Optional)\n";
      oss << "       minVal     - the parameter minimal range value (double value or a map name)\n";
      oss << "       maxVal     - the parameter maximal range value (double value or a map name)\n";
      oss << "       prmPDF     - the parameter probability density function type, the value could be one of the following:\n";
      oss << "                \"Block\"    - uniform probability between min and max values,\n";
      oss << "                \"Triangle\" - triangle shape probability function. The top triangle value is taken from the base case\n";
      oss << "                \"Normal\"   - normal (or Gaussian) probability function. The position of highest value is taken from the base case\n";

      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #                                      type               minVal  maxVal prmPDF\n";
      oss << "    " << cmdName << " \"Radiogenic heat rate\"  \"" << name() << "\"    0.1     4.9  \"Block\"\n";
      oss << "\n";
      oss << "    #                                      type               minVal  maxVal prmPDF\n";
      oss << "    " << cmdName << " \"Radiogenic heat rate\"  \"" << name() << "\"  \"MinMapName\" \"MaxMapName\"  \"Block\"\n";
      oss << "\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////////
//  SourceRockTOC parameter
////////////////////////////////////////////////////////////////
//
class SourceRockTOC : public PrmType
{
public:
   SourceRockTOC( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~SourceRockTOC() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
      std::string                 srtMix = prms.size() > 5 ? ( prms[pos++] ) : "";
      std::string                 layerName = prms[pos++];
      std::string                 srType = prms.size() > 6 ? ( prms[pos++] ) : "";
      double                      minVal = atof(    prms[pos++].c_str() );
      double                      maxVal = atof(    prms[pos++].c_str() );
      casa::VarPrmContinuous::PDF ppdf   = Str2pdf( prms[pos++] );

      int mixID = srtMix.empty() ? 1 : atoi( srtMix.substr( srtMix.size() - 1 ).c_str() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockTOC( *sa.get()
                                                                                  , name.c_str()
                                                                                  , layerName.c_str()
                                                                                  , mixID
                                                                                  , ( srType.empty() ? 0 : srType.c_str() )
                                                                                  , minVal
                                                                                  , maxVal
                                                                                  , ppdf ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
      
   size_t expectedParametersNumber() const { return 4; } // layer_name, min, max, pdf
   size_t optionalParametersNumber() const { return 2; } // mixID, SR type

   virtual std::string name() const { return "SourceRockLithoIoTbl:TocIni"; }

   virtual std::string description() const { return "the initial total organic content in source rock [ weight % ]"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"SourceRockLithoIoTbl:TocIni\" [mixID] <layerName> [srTypeName] <minVal> <maxVal> <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName - user specified variable parameter name (Optional)\n";
      oss << "       mixID      - (Optional) \"StratIoTbl:SourceRockType1\" or \"StratIoTbl:SourceRockType2\". This parameter defines\n";
      oss << "                    which source rock type from the mixing in the layer will be updated. The default value is SourceRockType1\n";
      oss << "       layerName  - source rock layer name\n";
      oss << "       srType     - (Optional) if source rock type name. If TOC value is dependent on Categorical Source Rock Type parameter\n";
      oss << "                    this value will be used to connect and establish this dependency.\n";
      oss << "       minVal     - the parameter minimal range value\n";
      oss << "       maxVal     - the parameter maximal range value\n";
      oss << "       prmPDF     - the parameter probability density function type\n";
      oss << "\n";

      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #                              type         layerName        minVal  maxVal   prmPDF\n";
      oss << "    "<< cmdName << " \"Lower Jurasic TOC\" \"" << name() << "\" \"Lower Jurassic\"  0.5    1.0  \"Block\"\n";
      oss << "\n    # Example with source rock mixing, TOC Value is set for the second SR in the mix for the Spekk layer\n";

      oss << "    "<< cmdName << " \"SpekkTOC\" \"" << name() << "\" \"StratIoTbl:SourceRockType2\" \"Spekk\" 0.5 1.0 \"Block\"\n";
      oss << "\n    # Example with dependency of TOC values range from source rock type\n";

      oss << "    "<< cmdName << " \"SpekkTOC\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_I\" 0.5 1.0 \"Block\"\n";

      oss << "    "<< cmdName << " \"SpekkTOC\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_II\" 5 10 \"Block\"\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////////
// SourceRockHC parameter
////////////////////////////////////////////////////////////////
//
class SourceRockHC : public PrmType
{
public:
   SourceRockHC( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~SourceRockHC() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
      std::string                 srtMix = prms.size() > 5 ? ( prms[pos++] ) : "";
      std::string                 layerName = prms[pos++];
      std::string                 srType = prms.size() > 6 ? ( prms[pos++] ) : "";
      double                      minVal = atof(    prms[pos++].c_str() );
      double                      maxVal = atof(    prms[pos++].c_str() );
      casa::VarPrmContinuous::PDF ppdf   = Str2pdf( prms[pos++] );

      int mixID = srtMix.empty() ? 1 : atoi( srtMix.substr( srtMix.size() - 1 ).c_str() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockHC( *sa.get()
                                                                                 , name.c_str()
                                                                                 , layerName.c_str()
                                                                                 , mixID
                                                                                 , ( srType.empty() ? 0 : srType.c_str() )
                                                                                 , minVal
                                                                                 , maxVal
                                                                                 , ppdf
                                                                                 ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
      
   size_t expectedParametersNumber() const { return 4; } // layer_name, min, max, pdf
   size_t optionalParametersNumber() const { return 2; } 

   virtual std::string name() const { return "SourceRockLithoIoTbl:HcIni"; }

   virtual std::string groupDescirption() const { return "\n  (Source rock parameters variation:)\n"; }

   virtual std::string description() const { return "the initial H/C ratio in source rock [ kg/tonne C ]"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" [mixID] <layerName> [srTypeName] <minVal> <maxVal> <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName - user specified variable parameter name (Optional)\n";
      oss << "       mixID      - (Optional) \"StratIoTbl:SourceRockType1\" or \"StratIoTbl:SourceRockType2\". This parameter defines\n";
      oss << "                    which source rock type from the mixing in the layer will be updated. The default value is SourceRockType1\n";
      oss << "       layerName  - source rock layer name\n";
      oss << "       srType     - (Optional) if source rock type name. If TOC value is dependent on Categorical Source Rock Type parameter\n";
      oss << "                    this value will be used to connect and establish this dependency.\n";
      oss << "       minVal     - the parameter minimal range value\n";
      oss << "       maxVal     - the parameter maximal range value\n";
      oss << "       prmPDF     - the parameter probability density function type\n";

      oss << "    Note: This parameter can't be defined together with HI for the same source rock lithology\n";

      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #          type         layerName        minVal  maxVal   prmPDF\n";
      oss << "    "<< cmdName << " \"" << name() << "\" \"Lower Jurassic\"   0.5    1.0  \"Block\"\n";

      oss << "\n    # Example with source rock mixing, H/C Value is set for the second SR in the mix for the Spekk layer\n";
      oss << "    "<< cmdName << " \"SpekkHC\" \"" << name() << "\" \"StratIoTbl:SourceRockType2\" \"Spekk\" 0.5 1.25 \"Block\"\n";

      oss << "\n    # Example with dependency of TOC values range from source rock type\n";
      oss << "    "<< cmdName << " \"SpekkHC\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_I\" 0.5 1.1 \"Block\"\n";

      oss << "    "<< cmdName << " \"SpekkHC\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_II\" 1.2 1.4 \"Block\"\n";
       return oss.str();
   }
};

////////////////////////////////////////////////////////////////
//  SourceRockHI parameter
////////////////////////////////////////////////////////////////
//
class SourceRockHI : public PrmType
{
public:
   SourceRockHI( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   
   virtual ~SourceRockHI() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
      std::string                 srtMix = prms.size() > 5 ? ( prms[pos++] ) : "";
      std::string                 layerName = prms[pos++];
      std::string                 srType = prms.size() > 6 ? ( prms[pos++] ) : "";
      double                      minVal = atof(    prms[pos++].c_str() );
      double                      maxVal = atof(    prms[pos++].c_str() );
      casa::VarPrmContinuous::PDF ppdf   = Str2pdf( prms[pos++] );

      int mixID = srtMix.empty() ? 1 : atoi( srtMix.substr( srtMix.size() - 1 ).c_str() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockHI( *sa.get()
                                                                                 , name.c_str()
                                                                                 , layerName.c_str()
                                                                                 , mixID
                                                                                 , ( srType.empty() ? 0 : srType.c_str() )
                                                                                 , minVal
                                                                                 , maxVal
                                                                                 , ppdf
                                                                                 ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
      
   size_t expectedParametersNumber() const { return 4; } // layer_name, min, max, pdf
   size_t optionalParametersNumber() const { return 2; } 

   virtual std::string name() const { return "SourceRockLithoIoTbl:HiIni"; }

   virtual std::string description() const { return "the initial hydrogen index (HI) ratio in source rock [ kg/tonne ]"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" [mixID] <layerName> [srTypeName] <minVal> <maxVal> <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName - user specified variable parameter name (Optional)\n";
      oss << "       mixID      - (Optional) \"StratIoTbl:SourceRockType1\" or \"StratIoTbl:SourceRockType2\". This parameter defines\n";
      oss << "                    which source rock type from the mixing in the layer will be updated. The default value is SourceRockType1\n";
      oss << "       layerName  - source rock layer name\n";
      oss << "       srType     - (Optional) if source rock type name. If TOC value is dependent on Categorical Source Rock Type parameter\n";
      oss << "                    this value will be used to connect and establish this dependency.\n";
      oss << "       minVal     - the parameter minimal range value\n";
      oss << "       maxVal     - the parameter maximal range value\n";
      oss << "       prmPDF     - the parameter probability density function type\n";

      oss << "    Note: This parameter does not variate HI value in source rock lithologies table. Instead it convert HI value to H/C and\n";
      oss << "          change source rock lithology H/C value. This was done in such way, because the simulator accepts only H/C value,\n";
      oss << "          ingnores HI value.\n";

      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #           type         layerName        minVal  maxVal   prmPDF\n";
      oss << "    " << cmdName << " \"" << name() << "SourceRockLithoIoTbl::HiIni\" \"Lower Jurassic\"    433.5    521.0  \"Block\"\n";

      oss << "\n    # Example with source rock mixing, HI Value is set for the second SR in the mix for the Spekk layer\n";
      oss << "    "<< cmdName << " \"SpekkHI\" \"" << name() << "\" \"StratIoTbl:SourceRockType2\" \"Spekk\" 433.5 521.0 \"Block\"\n";

      oss << "\n    # Example with dependency of TOC values range from source rock type\n";
      oss << "    "<< cmdName << " \"SpekkHI\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_I\" 433.5 521.0 \"Block\"\n";
      oss << "    "<< cmdName << " \"SpekkHI\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_II\" 700.0 800.0 \"Block\"\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////////
// SourceRockPreasphActEnergy parameter
////////////////////////////////////////////////////////////////
//
class SourceRockPreasphActEnergy : public PrmType
{
public:
   SourceRockPreasphActEnergy( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~SourceRockPreasphActEnergy() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
      std::string                 srtMix = prms.size() > 5 ? ( prms[pos++] ) : "";
      std::string                 layerName = prms[pos++];
      std::string                 srType = prms.size() > 6 ? ( prms[pos++] ) : "";
      double                      minVal = atof(    prms[pos++].c_str() );
      double                      maxVal = atof(    prms[pos++].c_str() );
      casa::VarPrmContinuous::PDF ppdf   = Str2pdf( prms[pos++] );

      int mixID = srtMix.empty() ? 1 : atoi( srtMix.substr( srtMix.size() - 1 ).c_str() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockPreAsphaltActEnergy( *sa.get()
                                                                                                  , name.c_str()
                                                                                                  , layerName.c_str()
                                                                                                  , mixID
                                                                                                  , ( srType.empty() ? 0 : srType.c_str() )
                                                                                                  , minVal
                                                                                                  , maxVal
                                                                                                  , ppdf
                                                                                                  ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
    }
      
   size_t expectedParametersNumber() const { return 4; } // layer_name, min, max, pdf
   size_t optionalParametersNumber() const { return 2; } 

   virtual std::string name() const { return "SourceRockLithoIoTbl:PreAsphaltStartAct"; }

   virtual std::string description() const { return "the activation energy limit for which the pre-asphalt cracking starts [ kJ/mol ]"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" [mixID] <layerName> [srTypeName] <minVal> <maxVal> <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName - user specified variable parameter name (Optional)\n";
      oss << "       mixID      - (Optional) \"StratIoTbl:SourceRockType1\" or \"StratIoTbl:SourceRockType2\". This parameter defines\n";
      oss << "                    which source rock type from the mixing in the layer will be updated. The default value is SourceRockType1\n";
      oss << "       layerName  - source rock layer name\n";
      oss << "       srType     - (Optional) if source rock type name. If TOC value is dependent on Categorical Source Rock Type parameter\n";
      oss << "                    this value will be used to connect and establish this dependency.\n";
      oss << "       minVal     - the parameter minimal range value\n";
      oss << "       maxVal     - the parameter maximal range value\n";
      oss << "       prmPDF     - the parameter probability density function type\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #             type                   layerName        minVal  maxVal   prmPDF\n";
      oss << "    " << cmdName << " \"" << name() << "\" \"Lower Jurassic\"    204.0  206.0  \"Block\"\n";

      oss << "\n    # Example with source rock mixing, PreAsphaltStartAct Value is set for the second SR in the mix for the Spekk layer\n";
      oss << "    "<< cmdName << " \"SpekkHI\" \"" << name() << "\" \"StratIoTbl:SourceRockType2\" \"Spekk\"";
      oss << " 204.0 206.0 \"Block\"\n";

      oss << "\n    # Example with dependency of PreAsphaltStartAct values range from source rock type\n";
      oss << "    "<< cmdName << " \"SpekkHI\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_I\" 204.0 206.0 \"Block\"\n";
      oss << "    "<< cmdName << " \"SpekkHI\" \"" << name() << "\" \"StratIoTbl:SourceRockType1\" \n";
      oss << " \"Spekk\" \"Type_II\" 205.0 207.0 \"Block\"\n";
      return oss.str();
   }
};


////////////////////////////////////////////////////////////////
// SourceRockType parameter
////////////////////////////////////////////////////////////////
//
class SourceRockType : public PrmType
{
public:
   SourceRockType( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~SourceRockType() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {

      std::string layerName = prms[1];
      std::vector<std::string> srtList    = CfgFileParser::list2array( prms[2], ',' );
      std::vector<double>      srtWeights = CfgFileParser::set2array(  prms[3], ',' );
      
      int mixID = 1;
      if ( !m_colName.empty() )
      {
         const std::string & mixIDStr = m_colName.substr( m_colName.size() - 1 );
         if ( CfgFileParser::isNumericPrm( mixIDStr ) ) { mixID = atoi( mixIDStr.c_str() ); }
      }

      if (ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySourceRockType( *sa.get()
                                                                                  , name.c_str()
                                                                                  , layerName.c_str()
                                                                                  , mixID
                                                                                  , srtList
                                                                                  , srtWeights
                                                                                  ) )
      {
         throw ErrorHandler::Exception(sa->errorCode()) << sa->errorMessage();
      }
   }

   size_t expectedParametersNumber() const { return 3; } // layer_name, source rocks list, wheights list
   size_t optionalParametersNumber() const { return 0; } 

   virtual std::string name() const { return "StratIoTbl:SourceRockType"; }

   virtual std::string description() const
   { 
      std::ostringstream oss;

      oss << "categorical parameter which could variate source rock type for the layer.\n";
      oss << "                                 This parameter must be defined BEFORE any other SourceRock parameter";
      return oss.str();
   }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "[1|2]\"  <layerName> \"SRType1,SRType2,SRType3\" [w1,w2,w3 ]\n";
      oss << "    Where:\n";
      oss << "       varPrmName - user specified variable parameter name (Optional)\n";
      oss << "       layerName  - source rock layer name\n";
      oss << "       SRtype     - comma separated list of source rock types for variation\n";
      oss << "       w1-w2      - comma separated list of source rock types weights\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #             type         layerName          category list       categories weight\n";
      oss << "    "<< cmdName << " \"" << name() << "1\" \"Lower Jurassic\"  \"Type1,Type2,Type3\" [0.8, 0.1, 0.1]\n";
      return oss.str();
   }
};


////////////////////////////////////////////////////////////////
// CrustThinningOneEvent parameter
////////////////////////////////////////////////////////////////
//
class CrustThinningOneEvent : public PrmType
{
public:
   CrustThinningOneEvent()  {;}
   virtual ~CrustThinningOneEvent() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      // Initial crustal thickness
      double minCrustThickn = atof( prms[1].c_str() );
      double maxCrustThickn = atof( prms[2].c_str() );

      // Start thinning time
      double minTStart = atof( prms[3].c_str() );
      double maxTStart = atof( prms[4].c_str() );

      // Thinning duration
      double minDeltaT = atof( prms[5].c_str() );
      double maxDeltaT = atof( prms[6].c_str() );

      double minFactor = atof( prms[7].c_str() );
      double maxFactor = atof( prms[8].c_str() );

      casa::VarPrmContinuous::PDF pdfType = Str2pdf( prms[9] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryOneCrustThinningEvent( *sa.get()
                                                                                          , name.c_str()
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
      
   size_t expectedParametersNumber() const { return 9; } // init crust thick min/max, t0 min/max, dt min/max, factor min/max, pdf
   size_t optionalParametersNumber() const { return 0; } 

   virtual std::string name() const { return "CrustThinningOneEvent"; }

   virtual std::string description() const { return "a crust thickness function with one crust thinning event"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" <IniCrstThickMn> <IniCrstThickMx> <mnT0> <mxT0> <mndT> <mxdT> <mnFact> <mxFct> <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName     - user specified variable parameter name (Optional)\n";
      oss << "       IniCrstThickMn - initial crust thickness - minimal range value\n";
      oss << "       IniCrstThickMx - initial crust thickness - maximal range value\n";
      oss << "       mnT0           - crust thinning event start time [Ma] - minimal range value\n";
      oss << "       mxT0           - crust thinning event start time [Ma] - maximal range value\n";
      oss << "       mndT           - crust thinning event duration [MY] - minimal range value\n";
      oss << "       mxdT           - crust thinning event duration [MY] - maximal range value\n";
      oss << "       mnFact         - crust thickness factor - minimal range value\n";
      oss << "       mxFact         - crust thickness factor - maximal range value\n";
      oss << "       prmPDF         - the parameter probability density function type\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #                                InCrThick   T0      dT    ThinFct  PDF\n";
      oss << "    " << cmdName << " \"" << name() << "\" 15000 40000 120 180 30 45   0.5 0.8 \"Block\"\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////////
// CrustThinning parameter
////////////////////////////////////////////////////////////////
//
class CrustThinning : public PrmType
{
public:
   CrustThinning()  {;}
   virtual ~CrustThinning() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t eventsNumber = (prms.size() - 3) / 7;

      if ( prms.size() != (4+eventsNumber * 7) )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << prms[0];
      }

      // Initial crustal thickness
      double minCrustThickn = atof( prms[1].c_str() );
      double maxCrustThickn = atof( prms[2].c_str() );

      // Start thinning time
      std::vector<double> minT0( eventsNumber );
      std::vector<double> maxT0( eventsNumber );
      // Thinning duration
      std::vector<double> minDeltaT( eventsNumber );
      std::vector<double> maxDeltaT( eventsNumber );
      // Thinning factor
      std::vector<double> minFactor( eventsNumber );
      std::vector<double> maxFactor( eventsNumber );
      // Maps list
      std::vector<std::string> mapsList( eventsNumber );

      size_t pos = 3;

      for ( size_t i = 0; i < eventsNumber; ++i )
      {
         minT0[i]     = atof( prms[pos++].c_str() );
         maxT0[i]     = atof( prms[pos++].c_str() );
         minDeltaT[i] = atof( prms[pos++].c_str() );
         maxDeltaT[i] = atof( prms[pos++].c_str() );
         minFactor[i] = atof( prms[pos++].c_str() );
         maxFactor[i] = atof( prms[pos++].c_str() );
         mapsList[i]  = prms[pos++];
      }
      casa::VarPrmContinuous::PDF pdfType = Str2pdf( prms[pos] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryCrustThinning( *sa.get()
                                                                                  , name.c_str()
                                                                                  , minCrustThickn
                                                                                  , maxCrustThickn
                                                                                  , minT0
                                                                                  , maxT0
                                                                                  , minDeltaT
                                                                                  , maxDeltaT
                                                                                  , minFactor
                                                                                  , maxFactor
                                                                                  , mapsList
                                                                                  , pdfType
                                                                                  )
         ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   }

   size_t expectedParametersNumber() const { return 10; } // init crust thick min/max, [t0 min/max, dt min/max, factor min/max mapname]+, pdf
   size_t optionalParametersNumber() const { return 1000; } 

   virtual std::string name() const { return "CrustThinning"; }

   virtual std::string description() const
   {
      return "a crust thickness function with arbitrary number of crust thinning event and thickness maps support";
   }

   virtual std::string groupDescirption() const { return "\n  (Crust thinning variation:)\n"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" <IniCrstThickMn> <IniCrstThickMx> <mnT0> <mxT0> <mndT> <mxdT> <mnFact> <mxFct> <mapName>\n";
      oss << "                                                                [<mnT0> <mxT0> <mndT> <mxdT> <mnFact> <mxFct> <mapName>]\n";
      oss << "                                                                ...\n";
      oss << "                                                                <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName     - user specified variable parameter name (Optional)\n";
      oss << "       IniCrstThickMn - initial crust thickness - minimal range value\n";
      oss << "       IniCrstThickMx - initial crust thickness - maximal range value\n";
      oss << "       mnT0           - crust thinning event start time [Ma] - minimal range value\n";
      oss << "       mxT0           - crust thinning event start time [Ma] - maximal range value\n";
      oss << "       mndT           - crust thinning event duration [MY] - minimal range value\n";
      oss << "       mxdT           - crust thinning event duration [MY] - maximal range value\n";
      oss << "       mnFact         - crust thickness factor - minimal range value\n";
      oss << "       mxFact         - crust thickness factor - maximal range value\n";
      oss << "       mapName        - thickness map name\n";
      oss << "       prmPDF         - the parameter probability density function type\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #                      InCrThick      T0       dT      ThinFct   MapName\n";
      oss << "    " << cmdName << " \"" << name() << "\" 20000  40000   200 250  20 30   0.7  0.9   \\ # first tinning event.\n";
      oss << "                                          120 120  20 20   0.65 0.65  \\ # second tinnnng event - no variation\n";
      oss << "                                          60  80   10 20   0.4  0.5   \\ # third event\n";
      oss << "                                          \"Block\"                       # PDF\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////////
// Surface Porosity parameter
////////////////////////////////////////////////////////////////
//
class SurfacePorosity : public PrmType
{
public:
   SurfacePorosity( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~SurfacePorosity() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
 
      const std::vector<std::string>            & layersName  = CfgFileParser::list2array( prms[pos++], ',' );
      std::vector<std::pair<std::string,size_t> > layersList;
      if ( !layersName.empty() )
      {
         for ( size_t i = 0; i < layersName.size(); ++i )
         {
            const std::vector<std::string> & curLay = CfgFileParser::list2array( layersName[i], ':' );
            if ( curLay.size() != 2 )
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Layer name must be defined as \"LayerName\":\"Lithotype1\"" <<
                  ", but it is defined as: " << layersName[i];
            }

            size_t mixID = atoi( curLay[1].substr( curLay[1].size() - 1 ).c_str() );
            if ( mixID > 3 )
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Lithotype could be only 1,2 or 3, but given:" << curLay[1];
            }
            layersList.push_back( std::pair<std::string, size_t>( curLay[0], mixID-1 ) );
         }
      }

      const std::vector<std::string> & allochtonLithologiesName = CfgFileParser::list2array( prms[pos++], ',' );
      const std::vector<std::string> & faultsMapList            = CfgFileParser::list2array( prms[pos++], ',' );
      std::vector<std::pair< std::string,std::string> >           faultsName;

      if( !faultsMapList.empty() )
      {
         for ( size_t i = 0; i < faultsMapList.size(); ++i )
         {
            const std::vector<std::string> & vec = CfgFileParser::list2array( faultsMapList[i], ':' );
            if ( vec.size() != 2 )
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Fault name must be defined as \"MapName\":\"FaultCutName\"" <<
                  ", but it is defined as: " << faultsMapList[i];
            }
            faultsName.push_back( std::pair<std::string,std::string>( vec[0], vec[1] ) );
         }
      }

      const std::string & lithoName  = prms[pos++];

      double       minSurfPor  = atof( prms[pos++].c_str() );
      double       maxSurfPor  = atof( prms[pos++].c_str() );

      casa::VarPrmContinuous::PDF pdfType = Str2pdf( prms.back() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VarySurfacePorosity( *sa.get()
                                                                                    , name
                                                                                    , layersList
                                                                                    , allochtonLithologiesName
                                                                                    , faultsName
                                                                                    , lithoName
                                                                                    , minSurfPor
                                                                                    , maxSurfPor
                                                                                    , pdfType
                                                                                    )
         ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   }
      
   size_t expectedParametersNumber() const { return 7; } // lay_names, aloch_names, fault_names, lit_name, surf_por mn/mx, pdf
   size_t optionalParametersNumber() const { return 0; } 

   virtual std::string name() const { return "LithotypeIoTbl:SurfacePorosity"; }

   virtual std::string description() const
   {
      return "a variation of surface porosity parameter for the given formation, alochton litholog or fault lithologies";
   }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" <layName> <alochtLithName> <faultName>  <minSurfPor> <maxSurfPor> <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName     - user specified variable parameter name (Optional)\n";
      oss << "       layName        - array of layers name\n";
      oss << "       alochtLithName - array of alochton lithologies name\n"; 
      oss << "       faultName      - array of faults name\n";
      oss << "       litName        - lithology name\n";
      oss << "       minSurfPor     - surface porosity - minimal range value\n";
      oss << "       maxSurfPor     - surface porosity - maximal range value\n";
      oss << "       prmPDF         - the parameter probability density function type\n";
      oss << "\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       VarPrmName      LayLst      AlochtLithLst          FaultsLst      LithName       SurfPor [%]   Parameter PDF\n";
      oss << "    " << cmdName << "  \"" << name() << "\"   [\"Permian\",\"Tertiary\"] [\"Permian\"]   [\"MAP-1234\":\"Faultcut1\",\"MAP-234\":\"Faultcut1\"] \"Std. Sandstone\"  \"Soil_Mechanics\"   38     58   \"Normal\"\n";
      oss << "    Example 2:\n";
      oss << "    #       VarPrmName                LayLst  AlochtLithLst  FaultsLst  LithName SurfPor [%]   Parameter PDF\n";
      oss << "    " << cmdName << " \"" << name() << "\" []     []      []  \"SM.Mudstone40%Clay\"  15 85  \"Block\"\n";
      return oss.str();
   }
};


////////////////////////////////////////////////////////////////
// PorosityModel parameter
////////////////////////////////////////////////////////////////
//
class PorosityModel : public PrmType
{
public:
   PorosityModel( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~PorosityModel() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
      const char * layerName   = ( prms.size() == 13 || prms.size() == 9 ) ? prms[pos++].c_str() : NULL;
      const char * lithoName   = prms[pos++].c_str();
      const char * modelName   = prms[pos++].c_str();
      double       minSurfPor  = atof( prms[pos++].c_str() );
      double       maxSurfPor  = atof( prms[pos++].c_str() );
      double       minCompCoef = atof( prms[pos++].c_str() );
      double       maxCompCoef = atof( prms[pos++].c_str() );

      double minMinPor = UndefinedDoubleValue;
      double maxMinPor = UndefinedDoubleValue;

      double minCompCoef1 = UndefinedDoubleValue; 
      double maxCompCoef1 = UndefinedDoubleValue; 

      if ( prms.size() == 12 || prms.size() == 13 )
      {
         minMinPor    = atof( prms[pos++].c_str() );
         maxMinPor    = atof( prms[pos++].c_str() );
         minCompCoef1 = atof( prms[pos++].c_str() ); 
         maxCompCoef1 = atof( prms[pos++].c_str() ); 
      }
      else if ( prms.size() != 8 && prms.size() != 9 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << prms[0];
      }
      casa::VarPrmContinuous::PDF pdfType = Str2pdf( prms.back() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryPorosityModelParameters( *sa.get()
                                                                                            , name.c_str()
                                                                                            , layerName
                                                                                            , lithoName
                                                                                            , modelName
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
         ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   }
      
   size_t expectedParametersNumber() const { return 7; } // lay_name, lit_name, mod_name, surf_por mn/mx, cc mn/mx, min_por mn/mx, cc2 mn/mx, pdf
   size_t optionalParametersNumber() const { return 5; } 

   virtual std::string name() const { return "LithotypeIoTbl:Porosity_Model"; }

   virtual std::string description() const { return "a variation of porosity model parameter for the given lithology"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" [layName] <litName> <modelName> <mnSurfPr> <mxSurfPr> <mnCmpCf> <mxCmpCf> \n";
      oss << "                                                   [<mnMinPr> <mxMinPr> <mnCmpCf1> <mxCmpCf1>] <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName - user specified variable parameter name (Optional)\n";
      oss << "       layName    - layer name (Optional). If it is given, the lithology will be copied and all changes will be done for the copy only\n";
      oss << "       litName    - lithology name\n";
      oss << "       modelName  - porosity model name, allowed values: Exponential, Soil_Mechanics, Double_Exponential\n";
      oss << "       mnSurfPr   - surface porosity - minimal range value\n";
      oss << "       mxSurfPr   - surface porosity - maximal range value\n";
      oss << "       mnCmpCf    - compaction coefficient - minimal range value\n";
      oss << "       mxCmpCf    - compaction coefficient - maximal range value\n";
      oss << "       mnMinPr    - minimal porosity (for Double_Exponential model only) - minimal range value\n";
      oss << "       mxMinPr    - minimal porosity (for Double_Exponential model only) - maximal range value\n";
      oss << "       mnCmpCf1   - compaction coefficient for the second exponent (for Double_Exponential model only) - minimal range value\n";
      oss << "       mxCmpCf1   - compaction coefficient for the second exponent (for Double_Exponential model only) - maximal range value\n";
      oss << "       prmPDF     - the parameter probability density function type\n";
      oss << "\n";
      oss << "    Note: for the Soil_Mechanics model only one parameter variation is possible, the second one should has same values\n";
      oss << "          for min/max and will be ignored\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       VarPrmName      LithName             PorModel       SurfPor [%]  CompCoeff  Parameter PDF\n";
      oss << "    " << cmdName << "  \"" << name() << "\" \"SM.Mudstone40%Clay\" \"Exponential\"  15 85        7.27 7.27  \"Block\"\n";
      oss << "\n";
      oss << "    Example 2:\n";
      oss << "    #      VarPrmName      LithName              PorModel          SurfPor [%]  CompCoeff      Parameter PDF\n";
      oss << "    " << cmdName << " \"" << name() << "\" \"SM.Mudstone40%Clay\"  \"Soil_Mechanics\"  15 85        0.1988 0.1988  \"Block\"\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////////
// PermeabilityModel parameter
////////////////////////////////////////////////////////////////
//
class PermeabilityModel : public PrmType
{
public:
   PermeabilityModel( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~PermeabilityModel() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
      std::string layerName = prms[pos++];
      std::string lithoName = prms[pos++];
      std::string modelName = prms[pos++];

      std::vector<double> minModelPrms;
      std::vector<double> maxModelPrms;

      // Anis. coeff
      minModelPrms.push_back( atof( prms[pos++].c_str() ) );
      maxModelPrms.push_back( atof( prms[pos++].c_str() ) );

      if ( modelName == "Sandstone" || modelName == "Sands" )
      {
         if ( prms.size() != 11 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << prms[0] <<
               ", expected 10, given " << prms.size();
         }
         // Depositional permeability 
         minModelPrms.push_back( atof( prms[pos++].c_str() ) );
         maxModelPrms.push_back( atof( prms[pos++].c_str() ) );
         // Clay percentage
         minModelPrms.push_back( atof( prms[pos++].c_str() ) );
         maxModelPrms.push_back( atof( prms[pos++].c_str() ) );
      }
      else if ( modelName == "Mudstone"  || modelName == "Shales" )
      {
         if ( prms.size() != 13 )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << prms[0] <<
               ", expected 12, given " << prms.size();
         }
         // Depositional permeability 
         minModelPrms.push_back( atof( prms[pos++].c_str() ) );
         maxModelPrms.push_back( atof( prms[pos++].c_str() ) );

         // Sensitivity coeff. 
         minModelPrms.push_back( atof( prms[pos++].c_str() ) );
         maxModelPrms.push_back( atof( prms[pos++].c_str() ) );

         // Recover coeff.
         minModelPrms.push_back( atof( prms[pos++].c_str() ) );
         maxModelPrms.push_back( atof( prms[pos++].c_str() ) );
      }
      else if ( modelName == "Multipoint" )
      {
         // Min permeability curve
         if ( pos >= prms.size() )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Number of points for min. permeability profile for layer "
               << layerName << " not given";
         }
         size_t numPts = atol( prms[pos++].c_str() ); // number of points for minimum multipoint perm. profile

         minModelPrms.push_back( static_cast<double>( numPts ) );

         if ( pos + numPts * 2 >= prms.size() )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of points for of min. permeability profile for layer "
               << layerName;
         }
         // read points of minimal profile
         for ( size_t i = 0; i < numPts; ++i )
         {
            minModelPrms.push_back( atof( prms[pos++].c_str() ) ); // porosity value
            minModelPrms.push_back( atof( prms[pos++].c_str() ) ); // permeability value
         }

         // Max permeability curve
         if ( pos >= prms.size() )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Number of points for max. permeability profile for layer "
               << layerName << " not given";
         }
         numPts = atol( prms[pos++].c_str() ); // number of points for maximum multipoint perm. profile
         maxModelPrms.push_back( static_cast<double>( numPts ) );

         if ( pos + numPts * 2 >= prms.size() )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of points for of max. permeability profile for layer "
               << layerName;
         }
         // read points of maximal profile
         for ( size_t i = 0; i < numPts; ++i )
         {
            maxModelPrms.push_back( atof( prms[pos++].c_str() ) ); // porosity value
            maxModelPrms.push_back( atof( prms[pos++].c_str() ) ); // permeability value
         }
      }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unsupported porosity model (" << modelName <<") for the layer " << layerName; 
      }

      if ( pos >= prms.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "PDF of permeability model for layer " << layerName << " not specified";
      }
      casa::VarPrmContinuous::PDF pdfType = Str2pdf( prms.back() );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryPermeabilityModelParameters( *sa.get()
                                                                                                , name.c_str()
                                                                                                , layerName.c_str()
                                                                                                , lithoName.c_str()
                                                                                                , modelName.c_str()
                                                                                                , minModelPrms
                                                                                                , maxModelPrms
                                                                                                , pdfType
                                                                                                )
         ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   }

   size_t expectedParametersNumber() const { return 9; }
   size_t optionalParametersNumber() const { return 1000; }

   virtual std::string name() const { return "LithotypeIoTbl:PermMixModel"; }

   virtual std::string description() const { return "a variation of permeability model parameter for the given layer/lithology combination"; }

   virtual std::string groupDescirption() const { return "\n  (Lithology parameters variation:)\n"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" [<layName>] <litName> <modelName> <minAnisCf> <maxAnisCf> [other model min/max parameters value] <prmPDF>\n";
      oss << "    Where:\n";
      oss << "       varPrmName    - user specified variable parameter name (Optional)\n";
      oss << "       layName       - layer name (Optional). If it is given, the lithology will be copied and all changes will be done for the copy only\n";
      oss << "       litName       - lithology name\n";
      oss << "       modelName     - permeability model name, allowed values: Sands, Shales, Multipoint\n";
      oss << "       minAnisCf     - anisotropic coefficient - minimal range value                        (All models)\n";
      oss << "       maxAnisCf     - anisotropic coefficient - maximal range value                        (All models)\n";
      oss << "       minDepoPerm   - depositional permeability - minimal range value                      (Sands/Shales models)\n";
      oss << "       maxDepoPerm   - depositional permeability - maximal range value                      (Sands/Shales models)\n";
      oss << "       minClayPerc   - clay percentage - minimal range value                                (Sands model only)\n";
      oss << "       maxClayPerc   - clay percentage - maximal range value                                (Sands model only)\n";
      oss << "       minSensitCf   - sensitivity coefficient - minimal range value                        (Shales model only)\n";
      oss << "       maxSensitCf   - sensitivity coefficient - maximal range value                        (Shales model only)\n";
      oss << "       minRecoverCf  - recover coefficient - minimal range value                            (Shales model only)\n";
      oss << "       maxRecoverCf  - recover coefficient - maximal range value                            (Shales model only)\n";
      oss << "       minPPProfSize - number of points in minimal permeability vs porosity profile         (Multipoint model only)\n"; 
      oss << "       minPPProf     - minimal profile of permeability vs porosity as set of pairs values   (Multipoint model only)\n";
      oss << "       maxPPProfSize - number of points in maximal permeability vs porosity profile         (Multipoint model only)\n"; 
      oss << "       maxPPProf     - maximal profile of permeability vs porosity as set of pairs values   (Multipoint model only)\n";
      oss << "       prmPDF        - the parameter probability density function type\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       VarPrmName         LayerName        LitholName   PermModel AnisotCoef DepoPerm [mD] SensCoef RecovCoef Parameter PDF\n";
      oss << "    " << cmdName << " \"" << name() << "\" \"Lower Jurassic\" \"Std. Shale\" \"Shales\"  1.0  1.0   0.005  0.015  1.0 2.0  0.01 0.01 \"Block\"\n";
      oss << "\n";
      oss << "    Example 2:\n";
      oss << "    #       VarPrmName         LayerName        LitholName       PermModel AnisotCoef DepoPerm [mD] ClayPerc [%] Parameter PDF\n";
      oss << "    " << cmdName << " \"" << name() << "\" \"Upper Jurassic\" \"Std. Sandstone\" \"Sands\"   1.0  1.0   5000  7000    1.0  2.0     \"Block\"\n";
      oss << "\n";
      oss << "    Example 3:\n";
      oss << "    #       VarPrmName         LayerName    LitholName               PermModel     AnisotCoef\n";
      oss << "    " << cmdName << " \"" << name() << "\" \"Paleocene\"  \"SM.Mudstone.40%Clay\"    \"Multipoint\"  1.0  1.0 \\ \n";
      oss << "    #                                                        Min profile           NumPts   Por  Perm   Por Perm\n";
      oss << "                                                                                   2        5    -7     60  -1.0 \\ \n";
      oss << "    #                                                        Max profile           NumPts   Por  Perm   Por Perm\n";
      oss << "                                                                                   2        5    -5     60   0.0 \\ \n";
      oss << "    #                                                                              Parameter PDF\n";
      oss << "                                                                                   \"Block\" \n";
      oss << "\n";
      return oss.str();
   }
};


////////////////////////////////////////////////////////////////
// STPThermalCondCoeff parameter
////////////////////////////////////////////////////////////////
//
class STPThermalCondCoeff : public PrmType
{
public:
   STPThermalCondCoeff( const std::string & prmTypeName = "" ) : PrmType( prmTypeName ) {;}
   virtual ~STPThermalCondCoeff() {;}

   virtual void addParameterObject( std::unique_ptr<casa::ScenarioAnalysis> & sa
                                  , const std::string                     & name
                                  , const std::vector<std::string>        & prms
                                  ) const
   {
      size_t pos = 1;
      const char * layerName = prms.size() == 6 ? prms[pos++].c_str() : NULL;
      const char * lithoName = prms[pos++].c_str();
      double       minVal    = atof( prms[pos++].c_str() );
      double       maxVal    = atof( prms[pos++].c_str() );

      casa::VarPrmContinuous::PDF ppdf = Str2pdf( prms[pos++] );

      if ( ErrorHandler::NoError != casa::BusinessLogicRulesSet::VaryLithoSTPThermalCondCoeffParameter( *sa.get()
                                                                                                      , name.c_str()
                                                                                                      , layerName
                                                                                                      , lithoName
                                                                                                      , minVal
                                                                                                      , maxVal
                                                                                                      , ppdf
                                                                                                      )
         ) { throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage(); }
   }

   size_t expectedParametersNumber() const { return 4; } // layer_name, min, max, pdf
   size_t optionalParametersNumber() const { return 1; } 

   virtual std::string name() const { return "LithotypeIoTbl:StpThCond"; }

   virtual std::string description() const { return "a variation of STP (Standart P & T) thermal conductivity coefficient for the given lithology"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    [varPrmName] \"" << name() << "\" [layName] <lithologyName> <minValue> <maxValue> <prmPDF>\n";
      oss << "       varPrmName    - user specified variable parameter name (Optional)\n";
      oss << "       layName       - layer name (Optional). If it is given, the lithology will be copied and all changes will be done for the copy only\n";
      oss << "       lithologyName - lithology name\n";
      oss << "       minVal    - the parameter minimal range value\n";
      oss << "       maxVal    - the parameter maximal range value\n";
      oss << "       prmPDF    - the parameter probability density function type\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       VarPrmName             LithName           min max  Parameter PDF\n";
      oss << "    " << cmdName << " \"" << name() << "\"  \"SM.Mudstone40%Clay\" 2   4   \"Block\"\n";
      return oss.str();
   }
};


////////////////////////////////////////////////////////////////
/// Parameters types factory class
////////////////////////////////////////////////////////////////

class PrmTypesFactory
{
public:
   PrmTypesFactory()
   {
      m_prmType["TopCrustHeatProduction"    ] = new TopCrustHeatProduction();
      m_prmType["SourceRockType"            ] = new SourceRockType();
      m_prmType["SourceRockTOC"             ] = new SourceRockTOC();
      m_prmType["SourceRockHC"              ] = new SourceRockHC();
      m_prmType["SourceRockHI"              ] = new SourceRockHI();
      m_prmType["SourceRockPreasphActEnergy"] = new SourceRockPreasphActEnergy();
      m_prmType["CrustThinningOneEvent"     ] = new CrustThinningOneEvent();
      m_prmType["CrustThinning"             ] = new CrustThinning();
      m_prmType["PorosityModel"             ] = new PorosityModel();
      m_prmType["PermeabilityModel"         ] = new PermeabilityModel();
      m_prmType["STPThermalCondCoeff"       ] = new STPThermalCondCoeff();
      
      m_prmType["LithotypeIoTbl:PermMixModel"   ] = new PermeabilityModel(      "LithotypeIoTbl:PermMixModel"     );
      m_prmType["LithotypeIoTbl:Porosity_Model" ] = new PorosityModel(          "LithotypeIoTbl:Porosity_Model"   );
      m_prmType["LithotypeIoTbl:SurfacePorosity"] = new SurfacePorosity(        "LithotypeIoTbl::SurfacePorosity" );

      m_prmType["BasementIoTbl:TopCrustHeatProd"] = new TopCrustHeatProduction( "BasementIoTbl:TopCrustHeatProd"  );
      m_prmType["LithotypeIoTbl:StpThCond"      ] = new STPThermalCondCoeff(    "LithotypeIoTbl:StpThCond"        );

      m_prmType["StratIoTbl:SourceRockType"     ] = new SourceRockType( "StratIoTbl:SourceRockType" );
      m_prmType["StratIoTbl:SourceRockType1"    ] = new SourceRockType( "StratIoTbl:SourceRockType1" );
      m_prmType["StratIoTbl:SourceRockType2"    ] = new SourceRockType( "StratIoTbl:SourceRockType2" );

      m_prmType["SourceRockLithoIoTbl:TocIni"   ] = new SourceRockTOC( "SourceRockLithoIoTbl:TocIni" );
      m_prmType["SourceRockLithoIoTbl:HiIni"    ] = new SourceRockHI(  "SourceRockLithoIoTbl:HiIni" );
      m_prmType["SourceRockLithoIoTbl:HcIni"    ] = new SourceRockHC(  "SourceRockLithoIoTbl:HcIni" );
      m_prmType["SourceRockLithoIoTbl:PreAsphaltStartAct"] = new SourceRockPreasphActEnergy( "SourceRockLithoIoTbl:PreAsphaltStartAct" );
   }

   ~PrmTypesFactory()
   { 
      for ( std::map<std::string, PrmType*>::iterator it = m_prmType.begin(); it != m_prmType.end(); ++it ) { delete it->second; }
   };

   const PrmType * factory( const std::string & name ) const { return m_prmType.count( name ) ? m_prmType.find(name)->second : 0; }
   
   std::vector<std::string> typesNameList() const
   {
      std::vector<std::string> ret;
      for ( std::map<std::string, PrmType*>::const_iterator it = m_prmType.begin(); it != m_prmType.end(); ++it )
      {
         ret.push_back( it->first );
      }
      return ret;
   }

private:
   std::map<std::string, PrmType *> m_prmType; ///< Keeps all observables type
};

//////////////////////////////////////////
// Parameters factory
//////////////////////////////////////////
static const PrmTypesFactory g_prmFactory;

///////////////////////////////////////////////////////////////////////////////
// CmdAddVarPrm methods
///////////////////////////////////////////////////////////////////////////////
CmdAddVarPrm::CmdAddVarPrm( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No name of variable parameter was given";
   }

   // the first parameter could be varparameter name
   const PrmType * pt = g_prmFactory.factory( m_prms[0] );
   
   if ( !pt && m_prms.size() > 1 )
   {
      pt = g_prmFactory.factory( m_prms[1] );
      if ( pt )
      {
         m_prmName = m_prms[0];
         m_prms.erase( m_prms.begin() );
      }
   }
   
   if ( !pt ) {  throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Unknown variable parameter name: " << m_prms[0]; }

   size_t expPrmsNum = pt->expectedParametersNumber() + 1;
   size_t expOptPrmNum = expPrmsNum + pt->optionalParametersNumber();

   // check number of command parameters for var parameter
   if ( m_prms.size() < expPrmsNum || m_prms.size() > expOptPrmNum )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << m_prms[0] << " variable parameter";
   }
}

void CmdAddVarPrm::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Add variable parameter: " << ( m_prmName.empty() ?
                                                                  (m_prms[0] + "(" + CfgFileParser::implode( m_prms, ",", 1 ) + ")") :
                                                                  (m_prmName)
                                                              );

   g_prmFactory.factory( m_prms[0] )->addParameterObject( sa, m_prmName, m_prms );

}

void CmdAddVarPrm::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <variable parameter type name> <min value> <max value> <pdf of parameter>\n\n";
   std::cout << "  Variable parameter - a parameter in Cauldron project file which exact value is unknown.\n";
   std::cout << "  There are only some estimations on it value range. For example - source rock TOC - [5:20]%.\n";
   std::cout << "  To define the variable parameter user should specify parameter type name and parameter range min/max values\n\n";

   std::cout << "  The following list of variable parameters is implemented for this command:\n";
 
   const std::vector<std::string> & prmTypesName = g_prmFactory.typesNameList();

   // print short description for each varparameter type
   for ( size_t i = 0; i < prmTypesName.size(); ++ i )
   {
      const PrmType * prm = g_prmFactory.factory( prmTypesName[i] );

      std::cout << prm->groupDescirption(); 
      std::cout << "    " << prmTypesName[i] << " - " << prm->description() << ".\n";
   }
   std::cout << "\n\nDetailed description:\n\n";

   // print examples of varparameters definition 
   for ( size_t i = 0; i < prmTypesName.size(); ++ i )
   {
      const PrmType * prm = g_prmFactory.factory( prmTypesName[i] );

      std::cout << prm->fullDescription();
      std::cout << "\n";
      std::cout << "    Example:\n";
      std::cout << prm->usingExample( cmdName );
      std::cout << "\n";
   }  
}
