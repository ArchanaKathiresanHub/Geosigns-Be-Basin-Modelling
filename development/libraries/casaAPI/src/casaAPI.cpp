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
#include "PrmCrustThinning.h"

#include "PrmPorosityModel.h"
#include "PrmSurfacePorosity.h"
#include "PrmPermeabilityModel.h"
#include "PrmLithoSTPThermalCond.h"

#include "VarPrmTopCrustHeatProduction.h"

#include "VarPrmSourceRockTOC.h"
#include "VarPrmSourceRockHC.h"
#include "VarPrmSourceRockHI.h"
#include "VarPrmSourceRockType.h"
#include "VarPrmSourceRockPreAsphaltStartAct.h"

#include "VarSpaceImpl.h"

#include "VarPrmOneCrustThinningEvent.h"
#include "VarPrmCrustThinning.h"

#include "VarPrmPorosityModel.h"
#include "VarPrmSurfacePorosity.h"
#include "VarPrmPermeabilityModel.h"
#include "VarPrmLithoSTPThermalCond.h"
#include "VarPrmLithoFraction.h"

// Standard C lib
#include <cmath>
#include <sstream>
#include <utility>


namespace casa {

// Auxillary function to search for the position of lithology in layer lithologies list
static size_t findMixingIDForLithologyInLayer( const char * layerName, const std::string & lithoName, ScenarioAnalysis & sa )
{
   size_t mixID = 0;
   if ( layerName && strlen( layerName ) > 0 )
   {
      mbapi::StratigraphyManager & smgr = sa.baseCase().stratigraphyManager();
      mbapi::StratigraphyManager::LayerID lyd = smgr.layerID( layerName );
      if ( UndefinedIDValue == lyd )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No such layer: " << layerName << " in stratigraphy table";
      }
      std::vector<std::string> lithLst;
      std::vector<double>      percLst;
      if ( ErrorHandler::NoError != smgr.layerLithologiesList( lyd, lithLst, percLst ) )
      {
         throw ErrorHandler::Exception( smgr.errorCode() ) << smgr.errorMessage();
      }
      for ( size_t i = 0; i < lithLst.size(); ++i )
      {
         if ( lithLst[i] == lithoName )
         {
            mixID = i;
            break;
         }
      }
   }
   return mixID;
}

///////////////////////////////////////////////////////////////////////////////
// Set of business logic rules functions to convert one request to set of parameters
namespace BusinessLogicRulesSet
{
// Add a parameter to variate layer thickness value [m] in given range
ErrorHandler::ReturnCode VaryLayerThickness( ScenarioAnalysis    & // sa
                                           , const char          * // name
                                           , const char          * // layerName
                                           , double                // minVal
                                           , double                // maxVal
                                           , VarPrmContinuous::PDF // rangeShape
                                           )
{
   return ErrorHandler::NotImplementedAPI;
}

// Add a parameter to variate top crust heat production value @f$ [\mu W/m^3] @f$ in given range
ErrorHandler::ReturnCode VaryTopCrustHeatProduction( ScenarioAnalysis               & sa
                                                   , const char                     * name
                                                   , const std::vector<double>      & dblRngIn
                                                   , const std::vector<std::string> & mapRngIn
                                                   , VarPrmContinuous::PDF            rangeShape
                                                   )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      std::vector<double>      dblRng( dblRngIn.begin(), dblRngIn.end() );
      std::vector<std::string> mapRng( mapRngIn.begin(), mapRngIn.end() );

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();
      
      casa::PrmTopCrustHeatProduction prm( mdl );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      if ( dblRng.size() == 2 )
      {
         double baseValue = prm.value();

         if ( baseValue < dblRng[0] || baseValue > dblRng[1] )
         {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
         }

         dblRng.push_back( baseValue );
      }
      else if ( mapRng.size() == 2 )
      {
         std::string baseValue = prm.mapName();
         if ( baseValue.empty() )
         {
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Base case has no map defined for radiogenic heat production";
         }
         mapRng.push_back( baseValue );

         // check are other 2 maps exist in maps list
         mbapi::MapsManager & mm = mdl.mapsManager();
         for ( size_t k = 0; k < 2; ++k )
         {
            mbapi::MapsManager::MapID mid = mm.findID( mapRng[k] );
            if ( UndefinedIDValue == mid )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Base case project has no map " << mapRng[k] << " defined";
            }
         }
      }
      else
      {
         ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Radiogenic heat production IP must be defined by simple range of" <<
            " two double values or by map range with two map names";
      }

      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmTopCrustHeatProduction( dblRng, mapRng, rangeShape, name ) ) )
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
ErrorHandler::ReturnCode VarySourceRockTOC( ScenarioAnalysis    & sa
                                          , const char          * name
                                          , const char          * layerName
                                          , int                   mixID
                                          , const char          * srTypeName
                                          , double                minVal
                                          , double                maxVal
                                          , VarPrmContinuous::PDF rangeShape
                                          )
{
   try
   {
      std::string srtName = srTypeName ? srTypeName : "";
      VarSpaceImpl & varPrmsSet = dynamic_cast< VarSpaceImpl & >( sa.varSpace() );

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockTOC prm( mdl, layerName, srTypeName, mixID );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      bool alreadyAdded = false;

      // check is the variable parameters set already has TOC parameter
      if ( !srtName.empty() )
      {
         for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
         {
            VarPrmSourceRockTOC * prm = dynamic_cast<VarPrmSourceRockTOC *>( varPrmsSet[ i ] );
            if ( !prm ) continue;
            if ( !prm->layerName().compare( layerName ) && prm->mixID() == mixID ) // already exists such TOC object
            {
               if ( name && prm->name()[0].compare( name ) ) // if name is given and name is different - error
               {
                  throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Different name for the same TOC variable parameter. Given: " 
                     << name << ", expected : " << prm->name()[0];
               }
               alreadyAdded = true;
               prm->addSourceRockTypeRange( srTypeName, baseValue[0], minVal, maxVal, rangeShape );
            }
         }
      }
      if ( !alreadyAdded )
      {
         std::unique_ptr<VarPrmSourceRockTOC> newPrm( new VarPrmSourceRockTOC( layerName
                                                                       , baseValue[0]
                                                                       , minVal
                                                                       , maxVal
                                                                       , rangeShape
                                                                       , name
                                                                       , srTypeName
                                                                       , mixID
                                                                       ) );
         // check if there is SourceRockType category parameter
         if ( !srtName.empty() )
         {
            for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
            {
               VarPrmSourceRockType * prm = dynamic_cast<VarPrmSourceRockType *>( varPrmsSet[ i ] );
               if ( !prm ) continue;
               if ( prm->mixID() == mixID && !prm->layerName().compare( layerName ) )
               {
                  prm->addDependent( newPrm.get() );
                  alreadyAdded = true;
               }
            }

            if ( !alreadyAdded ) // didn't find SourceRockType parameter defined
            {
               casa::PrmSourceRockTOC prm( mdl, layerName, 0, mixID );
               if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );
               if ( prm.sourceRockTypeName() != srtName ) // source rock lithology for this name do assigned to layer in StratIoTbl
               {
                  throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "There is no SourceRockType category parameter " <<
                     "defined and layer : " << layerName << " has different source rock type: " << prm.sourceRockTypeName() <<
                     " for mixing ID " << mixID << " then provided: " << srtName;
               }
            }
         }
         if ( ErrorHandler::NoError != varPrmsSet.addParameter( newPrm.release() ) )
         {
            return sa.moveError( varPrmsSet );
         }
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }
   return ErrorHandler::NoError;
}

// Add a parameter to variate source rock lithology HI value [kg/tonne] in given range
ErrorHandler::ReturnCode VarySourceRockHI( ScenarioAnalysis    & sa
                                         , const char          * name
                                         , const char          * layerName
                                         , int                   mixID
                                         , const char          * srTypeName
                                         , double                minVal
                                         , double                maxVal
                                         , VarPrmContinuous::PDF rangeShape
                                         )
{
   try
   {
      std::string srtName = srTypeName ? srTypeName : "";
      VarSpaceImpl & varPrmsSet = dynamic_cast< VarSpaceImpl & >(sa.varSpace());

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockHI prm( mdl, layerName, srTypeName, mixID );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      bool alreadyAdded = false;

      // check if H/C or HI variation for the same level is already in the list
      for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
      {
         const VarPrmSourceRockHC * hcPrm = dynamic_cast<const VarPrmSourceRockHC *>( varPrmsSet[i] );

         if ( hcPrm && hcPrm->layerName() == layerName && hcPrm->mixID() == mixID )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of source rock H/C parameter is already defined " << 
               "for the layer " << layerName << " with mixing ID: " << mixID << ", H/C and HI variation can not be defined together";
         }

         VarPrmSourceRockHI * hiPrm = dynamic_cast<VarPrmSourceRockHI *>( varPrmsSet[i] );
         if ( hiPrm && hiPrm->layerName() == layerName && hiPrm->mixID() == mixID )
         {
            if ( name && hiPrm->name()[0].compare( name ) ) // if name is given and name is different - error
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Different name for the same HI variable parameter. Given: " 
                  << name << ", expected : " << hiPrm->name()[0];
            }

            alreadyAdded = true;
            hiPrm->addSourceRockTypeRange( srTypeName, baseValue[0], minVal, maxVal, rangeShape );
         }
      }

      if ( !alreadyAdded )
      {
         std::unique_ptr<VarPrmSourceRockHI> newPrm( new VarPrmSourceRockHI( layerName
                                                                         , baseValue[0]
                                                                         , minVal
                                                                         , maxVal
                                                                         , rangeShape
                                                                         , name
                                                                         , srTypeName
                                                                         , mixID
                                                                         ) );
         // check if there is SourceRockType category parameter
         if ( !srtName.empty() )
         {
            for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
            {
               VarPrmSourceRockType * prm = dynamic_cast<VarPrmSourceRockType *>( varPrmsSet[ i ] );
               if ( !prm ) continue;
               if ( prm->mixID() == mixID && !prm->layerName().compare( layerName ) )
               {
                  prm->addDependent( newPrm.get() );
                  alreadyAdded = true;
               }
            }

            if ( !alreadyAdded ) // didn't find SourceRockType parameter defined
            {
               casa::PrmSourceRockHI prm( mdl, layerName, 0, mixID );
               if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );
               if ( prm.sourceRockTypeName() != srtName ) // source rock lithology for this name do assigned to layer in StratIoTbl
               {
                  throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "There is no SourceRockType category parameter " <<
                     "defined and layer : " << layerName << " has different source rock type: " << prm.sourceRockTypeName() <<
                     " for mixing ID " << mixID << " then provided: " << srtName;
               }
            }
         }
         if ( ErrorHandler::NoError != varPrmsSet.addParameter( newPrm.release() ) )
         {
            return sa.moveError( varPrmsSet );
         }
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }
   return ErrorHandler::NoError;
}

// Add a parameter to variate source rock lithology HC value [kg/tonne C] in given range
ErrorHandler::ReturnCode VarySourceRockHC( ScenarioAnalysis    & sa
                                         , const char          * name
                                         , const char          * layerName
                                         , int                   mixID
                                         , const char          * srTypeName
                                         , double                minVal
                                         , double                maxVal
                                         , VarPrmContinuous::PDF rangeShape
                                         )
{
   try
   {
      std::string srtName = srTypeName ? srTypeName : "";
      VarSpaceImpl & varPrmsSet = dynamic_cast< VarSpaceImpl & >(sa.varSpace());

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockHC prm( mdl, layerName, srTypeName, mixID  );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      bool alreadyAdded = false;

      // check if H/C or HI variation for the same level is already in the list
      for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
      {
         const VarPrmSourceRockHI * hiPrm = dynamic_cast<const VarPrmSourceRockHI *>( varPrmsSet[i] );
         if ( hiPrm && hiPrm->layerName() == layerName && hiPrm->mixID() == mixID )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of source rock HI parameter is already defined " << 
               " for the layer " << layerName << " with mixing ID: " << mixID << ", H/C and HI variation can not be defined together";
         }
         
         VarPrmSourceRockHC * hcPrm = dynamic_cast<VarPrmSourceRockHC *>( varPrmsSet[i] );
         if ( hcPrm && hcPrm->layerName() == layerName && hcPrm->mixID() == mixID )
         {
            if ( name && hcPrm->name()[0].compare( name ) ) // if name is given and name is different - error
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Different name for the same H/C variable arameter. Given: " 
                  << name << ", expected : " << hcPrm->name()[0];
            }

            alreadyAdded = true;
            hcPrm->addSourceRockTypeRange( srTypeName, baseValue[0], minVal, maxVal, rangeShape );
         }
      }
      // add variable parameter to VarSpace
      if ( !alreadyAdded )
      {
         std::unique_ptr<VarPrmSourceRockHC> newPrm( new VarPrmSourceRockHC( layerName
                                                                         , baseValue[0]
                                                                         , minVal
                                                                         , maxVal
                                                                         , rangeShape
                                                                         , name
                                                                         , srTypeName
                                                                         , mixID
                                                                         ) );
         // check if there is SourceRockType category parameter
         if ( !srtName.empty() )
         {
            for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
            {
               VarPrmSourceRockType * prm = dynamic_cast<VarPrmSourceRockType *>( varPrmsSet[ i ] );
               if ( !prm ) continue;
               if ( prm->mixID() == mixID && !prm->layerName().compare( layerName ) )
               {
                  prm->addDependent( newPrm.get() );
                  alreadyAdded = true;
               }
            }

            if ( !alreadyAdded ) // didn't find SourceRockType parameter defined
            {
               casa::PrmSourceRockHC prm( mdl, layerName, 0, mixID );
               if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );
               if ( prm.sourceRockTypeName() != srtName ) // source rock lithology for this name do assigned to layer in StratIoTbl
               {
                  throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "There is no SourceRockType category parameter " <<
                     "defined and layer : " << layerName << " has different source rock type: " << prm.sourceRockTypeName() <<
                     " for mixing ID " << mixID << " then provided: " << srtName;
               }
            }
         }
         if ( ErrorHandler::NoError != varPrmsSet.addParameter( newPrm.release() ) )
         {
            return sa.moveError( varPrmsSet );
         }
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}


// Add a parameter to variate source rock lithology pre-asphaltene activation energy value @f$ [kJ/mol] @f$ in given range
ErrorHandler::ReturnCode VarySourceRockPreAsphaltActEnergy( ScenarioAnalysis    & sa
                                                          , const char          * name
                                                          , const char          * layerName
                                                          , int                   mixID
                                                          , const char          * srTypeName
                                                          , double                minVal
                                                          , double                maxVal
                                                          , VarPrmContinuous::PDF rangeShape
                                                          )
{
   try
   {
      std::string srtName = srTypeName ? srTypeName : "";
      VarSpaceImpl & varPrmsSet = dynamic_cast< VarSpaceImpl & >( sa.varSpace() );

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSourceRockPreAsphaltStartAct prm( mdl, layerName, srTypeName, mixID );
      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );

      const std::vector<double> & baseValue = prm.asDoubleArray();
      assert( baseValue.size() == 1 );

      if ( baseValue[0] < minVal || baseValue[0] > maxVal )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Value of parameter in base case is outside of the given range";
      }

      bool alreadyAdded = false;

      // check is the variable parameters set already has PreAsphaltenActEnergy parameter
      if ( !srtName.empty() )
      {
         for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
         {
            VarPrmSourceRockPreAsphaltStartAct * prm = dynamic_cast<VarPrmSourceRockPreAsphaltStartAct *>( varPrmsSet[ i ] );
            if ( !prm ) continue;
            if ( !prm->layerName().compare( layerName ) && prm->mixID() == mixID ) // already exists such object
            {
               if ( name && prm->name()[0].compare( name ) ) // if name is given and name is different - error
               {
                  throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Different name for the same PreAsphaltenActEnergy " <<
                     "variable parameter. Given: " << name << ", expected : " << prm->name()[0];
               }
               alreadyAdded = true;
               prm->addSourceRockTypeRange( srTypeName, baseValue[0], minVal, maxVal, rangeShape );
            }
         }
      }
      if ( !alreadyAdded )
      {
         std::unique_ptr<VarPrmSourceRockPreAsphaltStartAct> newPrm( new VarPrmSourceRockPreAsphaltStartAct( layerName
                                                                                                         , baseValue[0]
                                                                                                         , minVal
                                                                                                         , maxVal
                                                                                                         , rangeShape
                                                                                                         , name
                                                                                                         , srTypeName
                                                                                                         , mixID
                                                                                                         ) );
         // check if there is SourceRockType category parameter
         if ( !srtName.empty() )
         {
            for ( size_t i = 0; i < varPrmsSet.size() && !alreadyAdded; ++i )
            {
               VarPrmSourceRockType * prm = dynamic_cast<VarPrmSourceRockType *>( varPrmsSet[ i ] );
               if ( !prm ) continue;
               if ( prm->mixID() == mixID && !prm->layerName().compare( layerName ) )
               {
                  prm->addDependent( newPrm.get() );
                  alreadyAdded = true;
               }
            }
            if ( !alreadyAdded ) // didn't find SourceRockType parameter defined
            {
               casa::PrmSourceRockPreAsphaltStartAct prm( mdl, layerName, 0, mixID );
               if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );
               if ( prm.sourceRockTypeName() != srtName ) // source rock lithology for this name do assigned to layer in StratIoTbl
               {
                  throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "There is no SourceRockType category parameter " <<
                     "defined and layer : " << layerName << " has different source rock type: " << prm.sourceRockTypeName() <<
                     " for mixing ID " << mixID << " then provided: " << srtName;
               }
            }
         }

         if ( ErrorHandler::NoError != varPrmsSet.addParameter( newPrm.release() ) )
         {
            return sa.moveError( varPrmsSet );
         }
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}


// Add parameter to variate source rock type for the specified layer
ErrorHandler::ReturnCode VarySourceRockType( ScenarioAnalysis               & sa
                                           , const char                     * name
                                           , const char                     * layerName
                                           , int                              mixingID
                                           , const std::vector<std::string> & stVariation
                                           , const std::vector<double>      & weights
                                           )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();
      
      PrmSourceRockType prm( mdl, layerName, mixingID );

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
         if ( tocPrm && tocPrm->layerName() == layerName && tocPrm->mixID() == mixingID )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of TOC for the layer: " << layerName <<
               " is defined before source rock type variation for the same layer";
         }

         const VarPrmSourceRockHC * hcPrm = dynamic_cast<const VarPrmSourceRockHC *>( prm );
         if ( hcPrm && hcPrm->layerName() == layerName && hcPrm->mixID() == mixingID )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of H/C for the layer: " << layerName <<
               " is defined before source rock type variation for the same layer";
         }

         const VarPrmSourceRockHI * hiPrm = dynamic_cast<const VarPrmSourceRockHI *>( prm );
         if ( hiPrm && hiPrm->layerName() == layerName && hiPrm->mixID() == mixingID )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of HI for the layer: " << layerName <<
               " is defined before source rock type variation for the same layer";
         }

         const VarPrmSourceRockPreAsphaltStartAct * aaPrm = dynamic_cast<const VarPrmSourceRockPreAsphaltStartAct *>( prm );
         if ( aaPrm && aaPrm->layerName() == layerName /*&& hiPrm->mixID() == mixingID*/ )
         {
            throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Variation of pre-asphalt activation energy for the layer: " <<
               layerName << " is defined before source rock type variation for the same layer";
         }
      }
     
      // add variable parameter to VarSpace
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmSourceRockType( layerName
                                                                                     , prm.sourceRockTypeName()
                                                                                     , mixingID
                                                                                     , stVariation
                                                                                     , weights
                                                                                     , name
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
 

// Add 4 parameters to variate one crust thinning event.
ErrorHandler::ReturnCode VaryOneCrustThinningEvent( casa::ScenarioAnalysis & sa, const char * name,
                                                    double minThickIni,    double maxThickIni,
                                                    double minT0,          double maxT0,       
                                                    double minDeltaT,      double maxDeltaT,   
                                                    double minThinningFct, double maxThinningFct, VarPrmContinuous::PDF thingFctPDF
                                                  )
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
      /*
       crust thickness profile shape in base project file could not match what we need : 
       *--------*
                \
                 *-----------*
       in this case, constructor of parameter could pick up some of base values from the base project file
       for others - we will use avarage from min/max
      */
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
                                                                                              thingFctPDF, name ) ) )
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

// Add Multi-event crust thinning parameter with maps support
ErrorHandler::ReturnCode VaryCrustThinning( casa::ScenarioAnalysis & sa
                                           , const char                     * name
                                           , double                           minThickIni,    double                     maxThickIni    
                                           , const std::vector<double>      & minT0,          const std::vector<double> & maxT0          
                                           , const std::vector<double>      & minDeltaT,      const std::vector<double> & maxDeltaT      
                                           , const std::vector<double>      & minThinningFct, const std::vector<double> & maxThinningFct 
                                           , const std::vector<std::string> & mapsList  
                                           , VarPrmContinuous::PDF            pdfType
                                           )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();
      mbapi::Model & mdl = sa.baseCase();

      if ( mdl.errorCode() != ErrorHandler::NoError ) return sa.moveError( mdl );
      // check given arrays dimensions
      if ( minT0.size() != maxT0.size()    || minT0.size() != minDeltaT.size()      || minT0.size() != maxDeltaT.size()     || 
           minT0.size() != mapsList.size() || minT0.size() != minThinningFct.size() || minT0.size() != maxThinningFct.size()
         )

      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Crust thinning variation: wrong parameters number";
      }
           
      // Get base value of parameter from the Model
      casa::PrmCrustThinning prm( mdl );

      //        t0      t1  t2     t3 t4    t5  t6                                           T0  DeltaT   ThinningFct  MapName
      //    S1  *--------*  |       |  |     |  |    t0: S0 - ThickIni                       Ev1: t1, (t2-t1), f1,          "Map1"
      //Ev1               \ |       |  |     |  |    t1: S1 = S0                             Ev2: t3, (t4-t1), f2            ""             
      //    S2           Map1-------*  |     |  |    t2: S2 = Map1 * f2                      Ev3: t5, (t6-t5), f3,          "Map2"
      //                             \ |     |  |    t3: S2
      //Ev2                           \|     |  |    t4: S3 = S2 * f3 = (Map1 * f2)  * f3  
      //    S3                         *-----*  |    t5  S3                                
      //Ev3                                   \ |    t6  S4 = Map2 * f4
      //    S4                                Map2

      // create min/base/max arrays to keep all crust thinning history variation
      // parameter as: initial thickness, events sequence as (time, duration, new thickness) triplets list
      const std::vector<double> & prmBaseValues        = prm.asDoubleArray();
      const std::vector<std::string> & prmBaseMapsList = prm.getMapsList();
      
      // check, does the base case has the same thinning history
      bool samePattern = prm.numberOfEvents() == minT0.size() ? true : false;
      for ( size_t i = 0; i < mapsList.size() && samePattern; ++i ) samePattern = !mapsList[i].compare( prmBaseMapsList[i] ) ? true : false;

      std::vector<double> baseValues;
      // if base case does not have the same pattern - ignore it and generate base case as a middle between min/max
      if ( !samePattern ) { baseValues.assign( 3 * minT0.size() + 1, UndefinedDoubleValue ); }
      else                { baseValues.insert( baseValues.begin(), prmBaseValues.begin(), prmBaseValues.end() ); }

      std::vector<double> minValues( 3 * minT0.size() + 1, UndefinedDoubleValue );
      std::vector<double> maxValues( 3 * minT0.size() + 1, UndefinedDoubleValue );

      if ( IsValueUndefined( minThickIni ) ||
           IsValueUndefined( maxThickIni ) ) { throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Initial crust thickness is undefined"; }
         
      minValues[0] = minThickIni;
      maxValues[0] = maxThickIni;

      if ( IsValueUndefined( baseValues[0] ) ) { baseValues[0] = 0.5 * ( minThickIni + maxThickIni );  }
 
      for ( size_t i = 0, pos = 1; i < minT0.size(); ++i ) // replace undefined base value with middle of value range
      {
         // process one event
         // Event start time
         if ( IsValueUndefined( minT0[i] ) || IsValueUndefined( maxT0[i] ) )
         { 
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << 
                  "Crust thinning event: " << i+1 << ", has undefined range for start time";
         }         
         minValues[pos] = minT0[i];
         maxValues[pos] = maxT0[i];

         if ( IsValueUndefined( baseValues[pos] ) ) { baseValues[pos] = 0.5 * ( minT0[i] + maxT0[i] ); }
         ++pos;
         
         // Event duration
         if ( IsValueUndefined( minDeltaT[i] ) || IsValueUndefined( maxDeltaT[i] ) )
         { 
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Crust thinning event: " << i+1 << ", has undefined range for event duration";
         }
         minValues[pos] = minDeltaT[i];
         maxValues[pos] = maxDeltaT[i];
         
         if ( IsValueUndefined( baseValues[pos] ) ) { baseValues[pos] = 0.5 * ( minDeltaT[i] + maxDeltaT[i] ); }
         ++pos;
 
         // Thinning factor
         if ( IsValueUndefined( minThinningFct[i] ) || IsValueUndefined( maxThinningFct[i] ) )
         { 
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Crust thinning event: " << i+1 << ", has undefined range for event thinning factor";
         }
         minValues[pos] = minThinningFct[i];
         maxValues[pos] = maxThinningFct[i];
         
         if ( IsValueUndefined( baseValues[pos] ) ) { baseValues[pos] = 0.5 * ( minThinningFct[i] + maxThinningFct[i] ); }
         ++pos;
      }
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmCrustThinning( baseValues, minValues, maxValues, mapsList, pdfType, name ) ) )
      {
         return sa.moveError( varPrmsSet );
      }
   }
   catch( const ErrorHandler::Exception & ex ) { return sa.reportError( ex.errorCode(), ex.what() ); }

   return ErrorHandler::NoError;
}

// Add variation of porosity model parameters 
ErrorHandler::ReturnCode VarySurfacePorosity( ScenarioAnalysis & sa
                                            , const std::string                                      & name         
                                            , const std::vector<std::pair<std::string, size_t> >     & layersName   
                                            , const std::vector<std::string>                         & allochtLitName
                                            , const std::vector<std::pair<std::string,std::string> > & faultsName   
                                            , const std::string                                      & litName      
                                            , double                                                   minSurfPor   
                                            , double                                                   maxSurfPor   
                                            , VarPrmContinuous::PDF                                    pdfType
                                            )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // calculate base value as middle of range first

      double baseSurfPor = 0.5 * ( minSurfPor + maxSurfPor );

      // Get base value of parameter from the Model
      mbapi::Model & mdl = sa.baseCase();

      casa::PrmSurfacePorosity prm( mdl, litName );
      baseSurfPor = prm.asDoubleArray()[0];

      // check ranges and base value
      ErrorHandler::Exception ex( ErrorHandler::OutOfRangeValue );
      if ( baseSurfPor < minSurfPor || baseSurfPor > maxSurfPor ) { throw ex << "Surface porosity in the base case is outside of the given range"; }

      // check - if not only lithology was specified, create a copy of corresponded lithology and update all refernces to it
      if ( !layersName.empty() || !allochtLitName.empty() || !faultsName.empty() )
      {
         const std::vector<std::string> & newLitNames = mdl.copyLithology( litName, layersName, allochtLitName, faultsName );

         if ( newLitNames.empty() ) { throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage(); }

         if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmSurfacePorosity( newLitNames, baseSurfPor, minSurfPor, maxSurfPor, pdfType, name ) ) )
         {
            throw ErrorHandler::Exception( varPrmsSet.errorCode() ) << varPrmsSet.errorMessage();
         }
      }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

ErrorHandler::ReturnCode VaryLithoFraction(
   ScenarioAnalysis                                   & sa
   , const std::string                                & name
   , const std::string                                & layerName
   , std::vector<int>                                   lithoFractionsInds
   , std::vector<double>                                minLithoFrac
   , std::vector<double>                                maxLithoFrac
   , casa::VarPrmContinuous::PDF 	                    pdfType
   )
{
   try
   {
      VarSpace & varPrmsSet = sa.varSpace();

      // Get base value of parameter from the model
      mbapi::Model & mdl = sa.baseCase();
      // If only one lithofraction was given we need to calculate the ratio for one of the two remaining lithofractions
      if ( lithoFractionsInds.size() == 1 )
      {
         const int numPercentages = 3;
         for ( int i = 0; i != numPercentages; ++i )
         {
            if ( i != lithoFractionsInds[0] )
            {
               lithoFractionsInds.push_back( i );
               break;
            }    
         }
      }

      PrmLithoFraction prm( mdl, layerName, lithoFractionsInds );
      std::vector<double> baseLithoFractions = prm.asDoubleArray();

      // If only one lithofraction was given, the last lithofraction does not vary
      if ( minLithoFrac.size() == 1 )
      {
         minLithoFrac.push_back( baseLithoFractions.back() );
         maxLithoFrac.push_back( baseLithoFractions.back() );
      }

      // Check ranges and the base value
      ErrorHandler::Exception ex( ErrorHandler::OutOfRangeValue );

      if ( baseLithoFractions[0] < minLithoFrac[0] || baseLithoFractions[0] > maxLithoFrac[0] )
      {
         throw ex << "The percentage of the lithology " << lithoFractionsInds[0] << " for the layer " << layerName << " in the base case: " <<
                      baseLithoFractions[0] << " is outside of the given range : [" << minLithoFrac[0] << ":" << maxLithoFrac[0] << "]" ;
      }

      if ( baseLithoFractions[1] < minLithoFrac[1] || baseLithoFractions[1] > maxLithoFrac[1] )
      {
         throw ex << "The ratio of the lithology " << lithoFractionsInds[1] << " for the layer " << layerName << " in the base case: " <<
                     baseLithoFractions[1] << " is outside of the given range : [" << minLithoFrac[1] << ":" << maxLithoFrac[1] << "]" ;
      }

      // add the variable lithofraction parameter to varPrmsSet 
      if ( !layerName.empty() )
      {
         if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmLithoFraction( layerName, lithoFractionsInds, baseLithoFractions, minLithoFrac, maxLithoFrac, pdfType, name ) ) )
         {
            throw ErrorHandler::Exception( varPrmsSet.errorCode() ) << varPrmsSet.errorMessage();
         }
      }
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

// Add variation of porosity model parameters 
ErrorHandler::ReturnCode VaryPorosityModelParameters( ScenarioAnalysis    & sa
                                                    , const char          * name
                                                    , const char          * layerName
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

               default:
                  assert(0);
                  break;
            }
         }
      }

      // check ranges and base value
      ErrorHandler::Exception ex( ErrorHandler::OutOfRangeValue );

      switch ( mdlType )
      {
         case PrmPorosityModel::DoubleExponential:
            if ( baseMinPor < minMinPor || baseMinPor > maxMinPor ) { throw ex << "Minimal porosity in the base case is outside of the given range"; }
            if ( baseCompCoef1 < minCompCoef1 || baseCompCoef1 > maxCompCoef1 ) { throw ex << "Compaction coeff. (the second one) in the base case is outside of the given range"; }

         case PrmPorosityModel::Exponential:
           if ( baseSurfPor < minSurfPor || baseSurfPor > maxSurfPor ) { throw ex << "Surface porosity in the base case is outside of the given range"; }
           if ( baseCompCoef < minCompCoef || baseCompCoef > maxCompCoef ) { throw ex << "Value of comaction coeff. in the base case is outside of the given range"; }
           break;

         case PrmPorosityModel::SoilMechanics:
            {
               bool surfPorIsDef = IsValueUndefined( minSurfPor  ) || IsValueUndefined( maxSurfPor  ) ? false : true;
               bool compCofIsDef = IsValueUndefined( minCompCoef ) || IsValueUndefined( maxCompCoef ) ? false : true;
 
               if ( surfPorIsDef && ( baseSurfPor < minSurfPor || baseSurfPor > maxSurfPor ) ) { throw ex << "Surface porosity in the base case is outside of the given range"; }
               if ( compCofIsDef && ( baseCompCoef < minCompCoef || baseCompCoef > maxCompCoef ) ) { throw ex << "Compaction coeff. in the base case is outside of the given range"; }
            }
            break;

         default:
            assert(0);
            break;
      }

      // check - if layer was specified, create a copy of corresponded lithology for the given 
      // layer and change Porosity Model parameters only for this lithology
      size_t mixID = findMixingIDForLithologyInLayer( layerName, litName, sa );
      const std::vector<std::string> & newLithoNames = mdl.copyLithology(
                                                 litName
                                               , (layerName != NULL && strlen(layerName) > 0) ?
                                                  std::vector<std::pair<std::string, size_t> >(1, std::pair<std::string,size_t>(layerName, mixID)) :
                                                  std::vector<std::pair<std::string, size_t> >()
                                               , std::vector<std::string>()
                                               , std::vector<std::pair<std::string, std::string> >()
                                                                        );

      if ( newLithoNames.empty() ) { throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage(); }

      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmPorosityModel( newLithoNames.front().c_str(), mdlType, 
                                                                                      baseSurfPor,   minSurfPor,   maxSurfPor, 
                                                                                      baseMinPor,    minMinPor,    maxMinPor,
                                                                                      baseCompCoef,  minCompCoef,  maxCompCoef,
                                                                                      baseCompCoef1, minCompCoef1, maxCompCoef1,
                                                                                      pdfType, name
                                                                                    ) )
         ) {  throw ErrorHandler::Exception( varPrmsSet.errorCode() ) << varPrmsSet.errorMessage(); }
   }
   catch( const ErrorHandler::Exception & ex )
   {
      return sa.reportError( ex.errorCode(), ex.what() );
   }

   return ErrorHandler::NoError;
}

/// @brief Add permeability model parameters variation
ErrorHandler::ReturnCode VaryPermeabilityModelParameters( ScenarioAnalysis      & sa
                                                        , const char            * name
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
     
      // get model parameters from project file
      std::vector<double>                        litMdlPrms;
      std::vector<double>                        litMdlMPPor;
      std::vector<double>                        litMdlMPPerm;
      mbapi::LithologyManager::PermeabilityModel litMdl;

      // get base value
      mbapi::LithologyManager & lmgr = sa.baseCase().lithologyManager();
      mbapi::LithologyManager::LithologyID ltid = lmgr.findID( lithoName );
      if ( UndefinedIDValue == ltid ) 
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No lithology with name: " << lithoName << " in lithologies type table";
      }
      if ( ErrorHandler::NoError != lmgr.permeabilityModel( ltid, litMdl, litMdlPrms, litMdlMPPor, litMdlMPPerm ) ) { return sa.moveError( lmgr ); }

      // check if model in project file is the same
      if ( static_cast<int>(litMdl) == static_cast<int>(mdlType) )
      {
         basModelPrms = litMdlPrms;
         if ( mbapi::LithologyManager::PermMultipoint == litMdl )
         {
            basModelPrms.push_back( static_cast<double>( litMdlMPPor.size() ) );
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
            if ( static_cast<int>(litMdl) == static_cast<int>(mdlType) ) // if one of the range value is undefined assign min/max to the base value
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
                  throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Type of permeability model is changed for lithology: " 
                                                                                << lithoName << ", but not all model parameters are defined";
               }
            }
         }
      }

      // check - if layer was specified, create a copy of corresponded lithology for the given 
      // layer and change Permeability Model parameters only for this lithology
      size_t mixID = findMixingIDForLithologyInLayer( layerName, lithoName, sa );
      const std::vector<std::string> & newLithoNames = sa.baseCase().copyLithology(
                                            lithoName
                                          , (layerName != NULL && strlen( layerName ) > 0 ) ? 
                                             std::vector<std::pair<std::string, size_t> >( 1, std::pair<std::string,size_t>( layerName, mixID ) ) :
                                             std::vector<std::pair<std::string, size_t> >()
                                          , std::vector<std::string>()
                                          , std::vector<std::pair<std::string, std::string> >() );

      if ( newLithoNames.empty() ) { throw ErrorHandler::Exception( sa.baseCase().errorCode() ) << sa.baseCase().errorMessage(); }

      VarSpace & varPrmsSet = sa.varSpace();
      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmPermeabilityModel( newLithoNames.front().c_str()
                                                                                        , mdlType
                                                                                        , basModelPrms
                                                                                        , minModelPrms
                                                                                        , maxModelPrms
                                                                                        , pdfType
                                                                                        , name
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


// Add STP thermal conductivity parameter variation for lithology
ErrorHandler::ReturnCode VaryLithoSTPThermalCondCoeffParameter( ScenarioAnalysis    & sa
                                                              , const char          * name
                                                              , const char          * layerName
                                                              , const char          * litName
                                                              , double                minVal
                                                              , double                maxVal
                                                              , VarPrmContinuous::PDF pdfType
                                                              )
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

      // check - if layer was specified, create a copy of corresponded lithology for the given 
      // layer and change STP thermal cond. parameter only for this lithology
      size_t mixID = findMixingIDForLithologyInLayer( layerName, litName, sa );
      const std::vector<std::string> & newLithoNames = mdl.copyLithology(
                                              litName
                                            , (layerName != NULL && strlen( layerName ) > 0 ) ? 
                                              std::vector<std::pair<std::string, size_t> >( 1, std::pair<std::string,size_t>( layerName, mixID ) ) :
                                              std::vector<std::pair<std::string, size_t> >()
                                            , std::vector<std::string>()
                                            , std::vector<std::pair<std::string, std::string> >() );

      if ( newLithoNames.empty() ) { throw ErrorHandler::Exception( sa.baseCase().errorCode() ) << sa.baseCase().errorMessage(); }

      if ( ErrorHandler::NoError != varPrmsSet.addParameter( new VarPrmLithoSTPThermalCond( newLithoNames.front().c_str()
                                                                                          , baseVal
                                                                                          , minVal
                                                                                          , maxVal
                                                                                          , pdfType
                                                                                          , name
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

} // namespace BusinessLogicRulesSet
} // namespace casa
