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


#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "DataDiggerImpl.h"
#include "ObsSpaceImpl.h"
#include "cmbAPI.h"

#include <cassert>

namespace casa
{

   const std::string Observable::s_dataMinerTable = "DataMiningIoTbl"; //name of the table which keeps observable values after simulations


   DataDiggerImpl::DataDiggerImpl()
   {
      ;
   }

   DataDiggerImpl::~DataDiggerImpl()
   {
      ;
   }


   ErrorHandler::ReturnCode DataDiggerImpl::requestObservables( ObsSpace & obs, RunCaseSet & rcs )
   {
      try
      {
         RunCaseSetImpl & rcSet   = dynamic_cast<RunCaseSetImpl &>(rcs);

         // go through all run cases and request observables
         for ( size_t rc = 0; rc < rcSet.size(); ++rc )
         {
            if ( NoError != requestObservables( obs, rcSet[rc] ) ) throw Exception( errorCode() ) << errorMessage();
         }
      }
      catch ( const ErrorHandler::Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

      return NoError;
   }

   ErrorHandler::ReturnCode DataDiggerImpl::requestObservables( ObsSpace & obs, RunCase * rc )
   {
      if ( !rc ) return NoError;
      ObsSpaceImpl   & obSpace = dynamic_cast<ObsSpaceImpl &>(obs);

      RunCaseImpl * cs = dynamic_cast<RunCaseImpl *>( rc );
      if ( !cs ) return NoError;

      mbapi::Model * mdl = cs->caseModel();

      if ( !mdl ) return NoError; // skip cases without model

      if ( NoError != mdl->clearTable( Observable::s_dataMinerTable ) ) return moveError( *mdl ); // clear the table

      // for each case go through all observables and update DataMiningIoTbl & SnapshotIoTbl
      for ( size_t ob = 0; ob < obSpace.size(); ++ob )
      {
         const std::string & msg = obSpace[ob]->checkObservableForProject(*mdl);

         if (!msg.empty()) continue; //if the observation is outside the model window, do not request it

         if ( NoError != obSpace[ob]->requestObservableInModel( *mdl ) ) return moveError( *mdl );
      }
      if ( NoError != mdl->saveModelToProjectFile( cs->projectPath() ) ) return moveError( *mdl );

      return NoError;
   }

   // Collect observables for all cases in the given case set
   ErrorHandler::ReturnCode DataDiggerImpl::collectRunResults( ObsSpace & obs, RunCaseSet & rcs )
   {
      RunCaseSetImpl & runCaseSet = dynamic_cast<RunCaseSetImpl &>(rcs);

      rcs.filterByExperimentName( "" );
      for ( size_t c = 0; c < runCaseSet.size(); ++c )
      {
         if ( NoError != collectRunResults( obs, runCaseSet[c] ) ) return errorCode();
      }
      return NoError;
   }

   // Collect observables for the given RunCase
   ErrorHandler::ReturnCode DataDiggerImpl::collectRunResults( ObsSpace & obs, RunCase * cs )
   {
      ObsSpaceImpl & observSpace = dynamic_cast<ObsSpaceImpl&>(obs);
      RunCaseImpl  * runCase     = dynamic_cast<RunCaseImpl*>( cs );

      assert( runCase->observablesNumber() == 0 );

      mbapi::Model & caseModel = runCase->loadProject();
      if ( caseModel.errorCode() != NoError ) { return moveError( caseModel ); }

      for ( size_t ob = 0; ob < observSpace.size(); ++ob )
      {
         Observable * obDef = observSpace[ob];
         ObsValue   * obVal = obDef->getFromModel( caseModel );

         if ( !obVal ) return moveError( caseModel );
         runCase->addObsValue( obVal );
      }
      return NoError;
   }

   // Serialize object to the given stream
   bool DataDiggerImpl::save( CasaSerializer & /* sz */, unsigned int /* fileVersion */ ) const
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

