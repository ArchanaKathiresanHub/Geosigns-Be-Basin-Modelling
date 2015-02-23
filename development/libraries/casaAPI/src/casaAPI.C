//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file cusaAPI.C 
/// This file keeps API definitions for Business Logic Rules Set functions

// CASA
#include "casaAPI.h"


#include "PrmTopCrustHeatProduction.h"
#include "PrmSourceRockTOC.h"
#include "PrmSourceRockHI.h"
#include "PrmOneCrustThinningEvent.h"
#include "PrmPorosityModel.h"

#include "VarPrmTopCrustHeatProduction.h"
#include "VarPrmSourceRockTOC.h"
#include "VarPrmSourceRockHI.h"
#include "VarPrmOneCrustThinningEvent.h"
#include "VarPrmPorosityModel.h"

// Standard C lib
#include <cmath>

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
      if ( mdl.errorCode() != ErrorHandler::NoError ) return mdl.errorCode();

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of parameter in base case is outside of the given range" );
      }

      return varPrmsSet.addParameter( new VarPrmTopCrustHeatProduction( baseValue[0], minVal, maxVal, rangeShape ) );
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }
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
      if ( mdl.errorCode() != ErrorHandler::NoError ) throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of parameter in base case is outside of the given range" );
      }

      return varPrmsSet.addParameter( new VarPrmSourceRockTOC( layerName, baseValue[0], minVal, maxVal, rangeShape ) );
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }
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
      if ( mdl.errorCode() != ErrorHandler::NoError ) throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of parameter in base case is outside of the given range" );
      }

      return varPrmsSet.addParameter( new VarPrmSourceRockHI( layerName, baseValue[0], minVal, maxVal, rangeShape ) );
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }
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
      if ( mdl.errorCode() != ErrorHandler::NoError ) return mdl.errorCode();

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
         return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of initial thickness parameter in base case is outside of the given range" );
      }

      if ( baseValues[1] < minT0 || baseValues[1] > maxT0 )
      {
         return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of start time for crust thinning parameter in base case is outside of the given range" );
      }

      if ( baseValues[2] < minDeltaT || baseValues[2] > maxDeltaT )
      {
         return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of duration of crust thinning parameter in base case is outside of the given range" );
      }

      if ( baseValues[3] < minThinningFct || baseValues[3] > maxThinningFct )
      {
         return mdl.reportError( ErrorHandler::OutOfRangeValue, "Value of crust thinning factor parameter in base case is outside of the given range" );
      }
      
      return varPrmsSet.addParameter( new VarPrmOneCrustThinningEvent( baseValues[0], minThickIni,    maxThickIni,
                                                                       baseValues[1], minT0,          maxT0,
                                                                       baseValues[2], minDeltaT,      maxDeltaT,
                                                                       baseValues[3], minThinningFct, maxThinningFct,
                                                                       thingFctPDF ) );
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }
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

      double baseSurfPor   = 0.5 * ( minSurfPor + maxSurfPor );
      double baseCompCoef  = 0.5 * ( minCompCoef  + maxCompCoef );
      double baseMinPor    = 0.5 * ( minMinPor    + maxMinPor );
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
      return varPrmsSet.addParameter( new VarPrmPorosityModel( litName,       mdlType, 
                                                               baseSurfPor,   minSurfPor,   maxSurfPor, 
                                                               baseMinPor,    minMinPor,    maxMinPor,
                                                               baseCompCoef,  minCompCoef,  maxCompCoef,
                                                               baseCompCoef1, minCompCoef1, maxCompCoef1,
                                                               pdfType
                                                             ) );
                                                                       
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }
}
 
} // namespace BusinessLogicRulesSet
} // namespace casa
