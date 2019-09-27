//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file DataDiggerImpl.C
/// @brief This file keeps API implementation for data digger.

#include "DataDiggerImpl.h"

#include "ObsSpace.h"
#include "RunCase.h"
#include "RunCaseSet.h"

#include "cmbAPI.h"

#include <cassert>

namespace casa
{

   const std::string Observable::s_dataMinerTable = "DataMiningIoTbl"; //name of the table which keeps observable values after simulations

   DataDiggerImpl::DataDiggerImpl() {}
   DataDiggerImpl::~DataDiggerImpl() {}

   ErrorHandler::ReturnCode DataDiggerImpl::requestObservables( ObsSpace & obs, RunCaseSet & rcs )
   {
      try
      {
         // go through all run cases and request observables
         for ( size_t rc = 0; rc < rcs.size(); ++rc )
         {
            if ( NoError != requestObservables( obs, rcs[rc].get() ) ) throw Exception( errorCode() ) << errorMessage();
         }
      }
      catch ( const ErrorHandler::Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

      return NoError;
   }

   ErrorHandler::ReturnCode DataDiggerImpl::requestObservables( ObsSpace & obSpace, RunCase * rc )
   {
      if ( !rc ) return NoError;
      mbapi::Model * mdl = rc->caseModel();

      if ( !mdl ) return NoError; // skip cases without model

      if ( NoError != mdl->clearTable( Observable::s_dataMinerTable ) ) return moveError( *mdl ); // clear the table

      // for each case go through all observables and update DataMiningIoTbl & SnapshotIoTbl
      for ( size_t ob = 0; ob < obSpace.size(); ++ob )
      {
         if (!obSpace.observable(ob)->checkObservableForProject(*mdl)) continue; //if the observation is outside the model window, do not request it

         if ( NoError != obSpace[ob]->requestObservableInModel( *mdl ) ) return moveError( *mdl );
      }
      if ( NoError != mdl->saveModelToProjectFile( rc->projectPath() ) ) return moveError( *mdl );

      return NoError;
   }

   ErrorHandler::ReturnCode DataDiggerImpl::requestObservablesInWindow( ObsSpace & obSpace, RunCase * rc )
   {
      if ( !rc ) return NoError;
      mbapi::Model * mdl = rc->caseModel();

      if ( !mdl ) return NoError; // skip cases without model

      if ( NoError != mdl->clearTable( Observable::s_dataMinerTable ) ) return moveError( *mdl ); // clear the table

      // for each case go through all observables and update DataMiningIoTbl & SnapshotIoTbl
      for ( size_t ob = 0; ob < obSpace.size(); ++ob )
      {
         if (!obSpace.observable(ob)->checkObservableOriginForProject(*mdl)) continue; //if the observation is outside the model window, do not request it

         if ( NoError != obSpace[ob]->requestObservableInModel( *mdl ) ) return moveError( *mdl );
      }
      if ( NoError != mdl->saveModelToProjectFile( rc->projectPath() ) ) return moveError( *mdl );

      return NoError;
   }

   // Collect observables for all cases in the given case set
   ErrorHandler::ReturnCode DataDiggerImpl::collectRunResults( ObsSpace & obs, RunCaseSet & runCaseSet )
   {
      runCaseSet.filterByExperimentName( "" );
      for ( size_t c = 0; c < runCaseSet.size(); ++c )
      {
         if ( NoError != collectRunResults( obs, runCaseSet[c].get() ) ) return errorCode();
      }
      return NoError;
   }

   // Collect observables for the given RunCase
   ErrorHandler::ReturnCode DataDiggerImpl::collectRunResults( ObsSpace & obs, RunCase * runCase )
   {
      assert( runCase->observablesNumber() == 0 );

      mbapi::Model & caseModel = runCase->loadProject();
      if ( caseModel.errorCode() != NoError ) { return moveError( caseModel ); }

      for ( size_t ob = 0; ob < obs.size(); ++ob )
      {
         const ObsValue * obVal = obs[ob]->getFromModel( caseModel );

         if ( !obVal ) return moveError( caseModel );

         obs.updateObsValueValidateStatus( ob, obs[ob]->isValid( obVal ) );

         runCase->addObsValue( obVal );
      }
      return NoError;
   }

   // Serialize object to the given stream
   bool DataDiggerImpl::save(CasaSerializer & /* sz */) const
   {
      bool ok = true;
      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   DataDiggerImpl::DataDiggerImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "DataDiggerImpl deserialization error";
      }
   }
}

