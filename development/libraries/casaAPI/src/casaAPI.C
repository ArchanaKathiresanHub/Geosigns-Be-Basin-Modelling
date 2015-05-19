//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file casaAPI.C 
/// This file keeps API definitions for Business Logic Rules Set functions

// CASA
#include "casaAPI.h"

#include "PrmTopCrustHeatProduction.h"

#include "PrmSourceRockTOC.h"
#include "PrmSourceRockHC.h"
#include "PrmSourceRockHI.h"
#include "PrmSourceRockType.h"
#include "PrmSourceRockPreAsphaltStartAct.h"

#include "PrmOneCrustThinningEvent.h"

#include "PrmPorosityModel.h"
#include "PrmPermeabilityModel.h"
#include "PrmLithoSTPThermalCond.h"

#include "VarPrmTopCrustHeatProduction.h"

#include "VarPrmSourceRockTOC.h"
#include "VarPrmSourceRockHC.h"
#include "VarPrmSourceRockHI.h"
#include "VarPrmSourceRockType.h"
#include "VarPrmSourceRockPreAsphaltStartAct.h"

#include "VarPrmOneCrustThinningEvent.h"

#include "VarPrmPorosityModel.h"
#include "VarPrmPermeabilityModel.h"
#include "VarPrmLithoSTPThermalCond.h"

// Utilities lib
#include <NumericFunctions.h>

// Standard C lib
#include <cmath>
#include <sstream>

namespace casa {

///////////////////////////////////////////////////////////////////////////////
// Set of business logic rules functions to convert one request to set of parameters
namespace BusinessLogicRulesSet
{
// Add a parameter to variate layer thickness value [m] in given range
ErrorHandler::ReturnCode VaryLayerThickness( ScenarioAnalysis & sa
                                           , const char * layerName
                                           , double minVal
                                           , double maxVal
                                           , VarPrmContinuous::PDF rangeShape
                                           )
{
   return ErrorHandler::NotImplementedAPI;
}

// Add a parameter to variate top crust heat production value @f$ [\mu W/m^3] @f$ in given range
ErrorHandler::ReturnCode VaryTopCrustHeatProduction( ScenarioAnalysis & sa
                                                   , double minVal
                                                   , double maxVal
                                                   , VarPrmContinuous::PDF rangeShape
                                                   )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();
      
      casa::PrmTopCrustHeatProduction prm( mdl );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmTopCrustHeatProduction( baseValue[0], minVal, maxVal, rangeShape ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

// Add a parameter to variate source rock lithology TOC value @f$ [%%] @f$ in given range
ErrorHandler::ReturnCode VarySourceRockTOC( ScenarioAnalysis & sa
                                          , const char * layerName
                                          , double minVal
                                          , double maxVal
                                          , VarPrmContinuous::PDF rangeShape
                                          )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockTOC prm( mdl, layerName );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmSourceRockTOC( layerName, baseValue[0], minVal, maxVal, rangeShape ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

// Add a parameter to variate source rock lithology HI value [kg/tonne] in given range
ErrorHandler::ReturnCode VarySourceRockHI( ScenarioAnalysis & sa
                                         , const char * layerName
                                         , double minVal
                                         , double maxVal
                                         , VarPrmContinuous::PDF rangeShape
                                         )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockHI prm( mdl, layerName );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      // check if H/C or HI variation for the same level is already in the list
      for ( size_t i = 0; i < varPrmsSet.numberOfContPrms(); ++i )
      {
         const VarPrmContinuous * prm = varPrmsSet.continuousParameter( i );

         const VarPrmSourceRockHC * hcPrm = dynamic_cast<const VarPrmSourceRockHC *>( prm );
         if ( hcPrm && hcPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of source rock H/C parameter is already defined for the layer " << layerName <<
               ", H/C and HI variation can not be defined together";
         }

         const VarPrmSourceRockHI * hiPrm = dynamic_cast<const VarPrmSourceRockHI *>( prm );
         if ( hiPrm && hiPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of source rock HI parameter is already defined for the layer " << layerName;
         }
      }

      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmSourceRockHI( layerName, baseValue[0], minVal, maxVal, rangeShape ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

// Add a parameter to variate source rock lithology HC value [kg/tonne C] in given range
ErrorHandler::ReturnCode VarySourceRockHC( ScenarioAnalysis & sa
                                         , const char * layerName
                                         , double minVal
                                         , double maxVal
                                         , VarPrmContinuous::PDF rangeShape
                                         )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockHC prm( mdl, layerName );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      // check if H/C or HI variation for the same level is already in the list
      for ( size_t i = 0; i < varPrmsSet.numberOfContPrms(); ++i )
      {
         const VarPrmContinuous * prm = varPrmsSet.continuousParameter( i );

         const VarPrmSourceRockHC * hcPrm = dynamic_cast<const VarPrmSourceRockHC *>( prm );
         if ( hcPrm && hcPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of source rock H/C parameter is already defined for the layer " << layerName;
         }
         
         const VarPrmSourceRockHI * hiPrm = dynamic_cast<const VarPrmSourceRockHI *>( prm );
         if ( hiPrm && hiPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of source rock HI parameter is already defined for the layer " << layerName <<
               ", H/C and HI variation can not be defined together";
         }
      }

      // add variable parameter to VarSpace
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmSourceRockHC( layerName, baseValue[0], minVal, maxVal, rangeShape ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}


// Add a parameter to variate source rock lithology pre-asphaltene activation energy value @f$ [kJ/mol] @f$ in given range
ErrorHandler::ReturnCode VarySourceRockPreAsphaltActEnergy( ScenarioAnalysis & sa
                                                          , const char * layerName
                                                          , double minVal
                                                          , double maxVal
                                                          , VarPrmContinuous::PDF rangeShape
                                                          )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockPreAsphaltStartAct prm( mdl, layerName );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmSourceRockPreAsphaltStartAct( layerName, baseValue[0], minVal, maxVal, rangeShape ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}


// Add parameter to variate source rock type for the specified layer
ErrorHandler::ReturnCode VarySourceRockType( ScenarioAnalysis               & sa
                                           , const char                     * layerName
                                           , const std::vector<std::string> & stVariation
                                           , const std::vector<double>      & weights
                                           )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();
      
      PrmSourceRockType prm( mdl, layerName );

      bool found = false;

      // check if base case exist in given variation
      for ( size_t i = 0; i < stVariation.size() && !found; ++i )
      {
         if ( stVariation[i] == prm.sourceRockTypeName() ) found = true;
      }

      if ( !found )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Scenario base case has source rock type name for layer " << 
            layerName << " which is not in source rock types variation list";
      }

      // we need to check are source rock lithology table keeps records for all given source rock types
      mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();

      for ( size_t i = 0; i < stVariation.size(); ++i )
      {
         mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( layerName, stVariation[i] );
   
         if ( IsValueUndefined( sid ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Source rock lithology variation " << 
               layerName << "->" << stVariation[i] <<  " does not exist in source rock lithology table";
         }
      }

      // check if TOC, H/C, HI or PreAsphaltenActEnergy variation defined before source rock type for this layer
      for ( size_t i = 0; i < varPrmsSet.size(); ++i )
      {
         const VarParameter * prm = varPrmsSet.parameter( i );

         const VarPrmSourceRockTOC * tocPrm = dynamic_cast<const VarPrmSourceRockTOC *>( prm );
         if ( tocPrm && tocPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of TOC for the layer: " << layerName <<
               " is defined before source rock type variation for the same layer";
         }

         const VarPrmSourceRockHC * hcPrm = dynamic_cast<const VarPrmSourceRockHC *>( prm );
         if ( hcPrm && hcPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of H/C for the layer: " << layerName <<
               " is defined before source rock type variation for the same layer";
         }

         const VarPrmSourceRockHI * hiPrm = dynamic_cast<const VarPrmSourceRockHI *>( prm );
         if ( hiPrm && hiPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of HI for the layer: " << layerName <<
               " is defined before source rock type variation for the same layer";
         }

         const VarPrmSourceRockPreAsphaltStartAct * aaPrm = dynamic_cast<const VarPrmSourceRockPreAsphaltStartAct *>( prm );
         if ( aaPrm && aaPrm->layerName() == layerName )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of pre-asphalt activation energy for the layer: " << layerName <<
               " is defined before source rock type variation for the same layer";
         }
      }
     
      // add variable parameter to VarSpace
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmSourceRockType( layerName, prm.sourceRockTypeName(), stVariation, weights ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}
 

// Add 4 parameters to variate one crust thinning event.
ErrorHandler::ReturnCode VaryOneCrustThinningEvent( casa::ScenarioAnalysis & sa, double minThickIni,    double maxThickIni,
                                                                                 double minT0,          double maxT0,       
                                                                                 double minDeltaT,      double maxDeltaT,   
                                                                                 double minThinningFct, double maxThinningFct, VarPrmContinuous::PDF thingFctPDF )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmOneCrustThinningEvent prm( mdl );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      std::vector<double> baseValues = prm.asDoubleArray();

      for ( size_t i = 0; i < 4; ++i ) // replace undefined base value with middle of value range
      // crust thickness profile shape in base project file could not match what we need : 
      // *--------*
      //          \
      //           *-----------*
      // in this case, constructor of parameter could pick up some of base values from the base project file
      // for others - we will use avarage from min/max
      {
         if ( IsValueUndefined( baseValues[i] ) )
         {
            switch ( i )
            {
               case 0: baseValues[i] = 0.5 * ( minThickIni    + maxThickIni    ); break;
               case 1: baseValues[i] = 0.5 * ( minT0          + maxT0          ); break;
               case 2: baseValues[i] = 0.5 * ( minDeltaT      + maxDeltaT      ); break;
               case 3: baseValues[i] = 0.5 * ( minThinningFct + maxThinningFct ); break;
            }
         }
      }

      if ( baseValues[0] < minThickIni || baseValues[0] > maxThickIni )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of initial thickness parameter in base case is outside of the given range";
      }

      if ( baseValues[1] < minT0 || baseValues[1] > maxT0 )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of start time for crust thinning parameter in base case is outside of the given range";
      }

      if ( baseValues[2] < minDeltaT || baseValues[2] > maxDeltaT )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of duration of crust thinning parameter in base case is outside of the given range";
      }

      if ( baseValues[3] < minThinningFct || baseValues[3] > maxThinningFct )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of crust thinning factor parameter in base case is outside of the given range";
      }
      
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmOneCrustThinningEvent( baseValues[0], minThickIni,    maxThickIni,
                                                                                              baseValues[1], minT0,          maxT0,
                                                                                              baseValues[2], minDeltaT,      maxDeltaT,
                                                                                              baseValues[3], minThinningFct, maxThinningFct,
                                                                                              thingFctPDF ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

// Add variation of porosity model parameters 
ErrorHandler::ReturnCode VaryPorosityModelParameters( ScenarioAnalysis    & sa
                                                    , const char          * litName
                                                    , const char          * modelName
                                                    , double                minSurfPor
                                                    , double                maxSurfPor
                                                    , double                minCompCoef
                                                    , double                maxCompCoef
                                                    , double                minMinPor
                                                    , double                maxMinPor
                                                    , double                minCompCoef1
                                                    , double                maxCompCoef1
                                                    , VarPrmContinuous::PDF pdfType
                                                    )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // convert porosity model name to enum value
      PrmPorosityModel::PorosityModelType mdlType = PrmPorosityModel::UndefinedModel;
      if (      !strcmp( modelName, "Exponential"        ) ) { mdlType = PrmPorosityModel::Exponential;       }
      else if ( !strcmp( modelName, "Soil_Mechanics"     ) ) { mdlType = PrmPorosityModel::SoilMechanics;     }
      else if ( !strcmp( modelName, "Double_Exponential" ) ) { mdlType = PrmPorosityModel::DoubleExponential; }
      else { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unsupported porosity model: " << modelName; }

      // calculate base value as middle of range first

      double baseSurfPor   = 0.5 * ( minSurfPor   + maxSurfPor   );
      double baseCompCoef  = 0.5 * ( minCompCoef  + maxCompCoef  );
      double baseMinPor    = 0.5 * ( minMinPor    + maxMinPor    );
      double baseCompCoef1 = 0.5 * ( minCompCoef1 + maxCompCoef1 );

      if ( PrmPorosityModel::SoilMechanics == mdlType )
      {
         bool surfPorIsDef = IsValueUndefined( minSurfPor  ) || IsValueUndefined( maxSurfPor  ) ? false : true;
         bool compCofIsDef = IsValueUndefined( minCompCoef ) || IsValueUndefined( maxCompCoef ) ? false : true;

         if ( ! surfPorIsDef && ! compCofIsDef ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << 
            "For soil mechanics porosity model the surface porosity or compaction coefficient value range must be specified";

         // surface porosity has some preference 
         else if ( !surfPorIsDef &&  compCofIsDef ) baseSurfPor  = SMcc2sp( baseCompCoef );
         else if (  surfPorIsDef && !compCofIsDef ) baseCompCoef = SMsp2cc( baseSurfPor );
      }

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmPorosityModel prm( mdl, litName );
      if ( mdl.errorCode() == ErrorHandler::NoError )
      {
         std::vector<double> baseValues = prm.asDoubleArray();

         // base case has the same model - use it values as base values
         if ( static_cast<int>( mdlType ) == prm.asInteger() )
         {
            switch ( mdlType )
            {
               case PrmPorosityModel::Exponential:
                  baseSurfPor  = baseValues[0];
                  baseCompCoef = baseValues[1];
                  break;

               case PrmPorosityModel::SoilMechanics:
                  baseSurfPor  = SMcf2sp( baseValues[0] );
                  baseCompCoef = SMcf2cc( baseValues[0] );
                  break;

               case PrmPorosityModel::DoubleExponential:
                  baseSurfPor   = baseValues[0];
                  baseCompCoef  = baseValues[1];
                  baseMinPor    = baseValues[2];
                  baseCompCoef1 = baseValues[3];
                  break;                     
            }
         }
      }
      
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmPorosityModel( litName,       mdlType, 
                                                                                      baseSurfPor,   minSurfPor,   maxSurfPor, 
                                                                                      baseMinPor,    minMinPor,    maxMinPor,
                                                                                      baseCompCoef,  minCompCoef,  maxCompCoef,
                                                                                      baseCompCoef1, minCompCoef1, maxCompCoef1,
                                                                                      pdfType
                                                                                    ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

/// @brief Add permeability model parameters variation
ErrorHandler::ReturnCode VaryPermeabilityModelParameters( ScenarioAnalysis      & sa
                                                        , const char            * layerName
                                                        , const char            * lithoName
                                                        , const char            * modelName
                                                        , std::vector<double>   & minModelPrms
                                                        , std::vector<double>   & maxModelPrms
                                                        , VarPrmContinuous::PDF   pdfType
                                                        )
{
   try
   {
      // convert model name to enum value
      PrmPermeabilityModel::PermeabilityModelType mdlType = PrmPermeabilityModel::Unknown;

      if (      !strcmp( modelName, "Sandstone"  ) || !strcmp( modelName, "Sands"  ) ) { mdlType = PrmPermeabilityModel::Sandstone;  }
      else if ( !strcmp( modelName, "Mudstone"   ) || !strcmp( modelName, "Shales" ) ) { mdlType = PrmPermeabilityModel::Mudstone;   }
      else if ( !strcmp( modelName, "Multipoint" )                                   ) { mdlType = PrmPermeabilityModel::Multipoint; }
      else { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unsupported permeability model: " << modelName; }

      if ( PrmPermeabilityModel::Multipoint != mdlType && minModelPrms.size() != maxModelPrms.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << 
            "Different number parameters for minimal (" << minModelPrms.size() << 
                                      ") and maximal (" << maxModelPrms.size() << 
                                      ") values in varying of lithology permeability model parameters";
      }
      std::vector<double> basModelPrms;

      // set base value as middle one
      if ( PrmPermeabilityModel::Multipoint == mdlType ) 
      {  // create base case curve as a middle curve between min/max
         basModelPrms = VarPrmPermeabilityModel::createBaseCaseMPModelPrms( minModelPrms, maxModelPrms );
      }
      else
      {
         for ( size_t i = 0; i < minModelPrms.size(); ++i ) basModelPrms.push_back( (minModelPrms[i] + maxModelPrms[i]) * 0.5 );
      }
      
      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();
      
      mbapi::StratigraphyManager & smgr = mdl.stratigraphyManager();
      mbapi::LithologyManager    & lmgr = mdl.lithologyManager();
      
      mbapi::StratigraphyManager::LayerID lid = smgr.layerID( layerName );

      if ( UndefinedIDValue == lid )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No layer with name: " << layerName << " in stratigraphy table";
      }
      // do checking if we need to copy lithology
      std::vector<std::string> lithNames;
      std::vector<double>      lithPerc;
      smgr.layerLithologiesList( lid, lithNames, lithPerc );

      int found = -1;
      for ( size_t i = 0; i < lithNames.size() && found < 0; ++i )
      {
         if ( lithNames[i] == lithoName ) found = static_cast<int>( i );
      }
      if ( found < 0 ) throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Layer " << layerName << " has no lithology type: " << lithoName;

      // get model parameters from project file
      std::vector<double> litMdlPrms;
      std::vector<double> litMdlMPPor;
      std::vector<double> litMdlMPPerm;
      mbapi::LithologyManager::PermeabilityModel litMdl;

      // get base value
      mbapi::LithologyManager::LithologyID ltid = lmgr.findID( lithoName );
      if ( UndefinedIDValue == ltid ) 
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No lithology with name: " << lithoName << " in lithologies type table";
      }
      if ( ErrorHandler::NoError != lmgr.permeabilityModel( ltid, litMdl, litMdlPrms, litMdlMPPor, litMdlMPPerm ) ) { return sa.moveError( lmgr ); }

      // check if model in project file is the same
      if ( litMdl == mdlType )
      {
         basModelPrms = litMdlPrms;
         if ( mbapi::LithologyManager::PermMultipoint == litMdl )
         {
            basModelPrms.push_back( litMdlMPPor.size() );
            for ( size_t i = 0; i < litMdlMPPor.size(); ++i )
            {
               basModelPrms.push_back( litMdlMPPor[i]  );
               basModelPrms.push_back( litMdlMPPerm[i] );
            }
         }
      }
      // check parameters for undefined values and replace them with base value
      for ( size_t i = 0; i < minModelPrms.size(); ++i )
      {
         if ( IsValueUndefined( minModelPrms[i] ) || IsValueUndefined( maxModelPrms[i] ) )
         {
            if ( litMdl == mdlType ) // if one of the range value is undefined assign min/max to the base value
            {
               minModelPrms[i] = maxModelPrms[i] = basModelPrms[i];
            }
            else // if in the project file the model is different - we can't get base value from it and replace undefined values with it
            {
               // if one of the value is defined - assign it to another one and base value
               if (      !IsValueUndefined( minModelPrms[i] ) ) maxModelPrms[i] = basModelPrms[i] = minModelPrms[i];
               else if ( !IsValueUndefined( maxModelPrms[i] ) ) minModelPrms[i] = basModelPrms[i] = maxModelPrms[i];
               else
               {
                  throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Type of permeability model is changed for lithology: " << lithoName <<
                     ", but not all model parameters are defined";
               }
            }
         }
      }
      
      const std::vector<mbapi::StratigraphyManager::LayerID> & layersWithSameLith = smgr.findLayersForLithology( lithoName );
      bool copyLithology = layersWithSameLith.size() > 1 ? true : false;
      
      // create lithology copy if needed
      if ( copyLithology )
      {
         smgr.layerLithologiesList( lid, lithNames, lithPerc );
         mbapi::LithologyManager::LithologyID lithID = lmgr.findID( lithoName );
         if ( UndefinedIDValue == lithID ) return sa.moveError( lmgr );

         // construct new lithology name as oldName_layerName_CASA_copy
         std::ostringstream oss;
         oss << lithoName << "_" << layerName << "_CASA_copy";

         mbapi::LithologyManager::LithologyID newLithID = lmgr.copyLithology( lithID, oss.str() );
         if ( UndefinedIDValue == newLithID ) return sa.moveError( lmgr );

         lithNames[found] = lmgr.lithologyName( newLithID );
         smgr.setLayerLithologiesList( lid, lithNames, lithPerc );
      }

      VarSpace & varPrmsSet = sa.varSpace();
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmPermeabilityModel( lithNames[found].c_str(), mdlType, basModelPrms, minModelPrms, maxModelPrms, pdfType ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}


// Add STP thermal conductivity parameter variation for lithology
ErrorHandler::ReturnCode VaryLithoSTPThermalCondCoeffParameter( ScenarioAnalysis & sa, const char * litName, double minVal, double maxVal, VarPrmContinuous::PDF pdfType )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      double baseVal = 0.5 * ( minVal + maxVal );

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      PrmLithoSTPThermalCond prm( mdl, litName );

      if ( mdl.errorCode() == ErrorHandler::NoError )
      {
         baseVal = prm.asDoubleArray()[0];
      }
      
      if ( baseVal < minVal || baseVal > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of STP thermal conductivity in base case is outside of the given range";
      }


      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmLithoSTPThermalCond( litName, baseVal, minVal, maxVal, pdfType ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

} // namespace BusinessLogicRulesSet
} // namespace casa
