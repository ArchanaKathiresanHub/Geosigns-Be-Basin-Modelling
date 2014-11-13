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
      ObsSpaceImpl   & obSpace = dynamic_cast<ObsSpaceImpl &>(obs);
      RunCaseSetImpl & rcSet = dynamic_cast<RunCaseSetImpl &>(rcs);

      for ( size_t rc = 0; rc < rcSet.size(); ++rc )
      {
         // go through all run cases
         RunCaseImpl * cs = dynamic_cast<RunCaseImpl *>(rcSet[rc]);
         if ( !cs ) continue;

         mbapi::Model * mdl = cs->caseModel();

         if ( !mdl ) continue; // skip cases without model

         if ( NoError != mdl->clearTable( Observable::s_dataMinerTable ) ) return moveError( *mdl ); // clear the table

         // for each case go through all observables and update DataMiningIoTbl & SnapshotIoTbl
         for ( size_t ob = 0; ob < obSpace.size(); ++ob )
         {
            if ( NoError != obSpace[ob]->requestObservableInModel( *mdl ) ) return moveError( *mdl );
         }
         if ( NoError != mdl->saveModelToProjectFile( cs->projectPath() ) ) return moveError( *mdl );
      }
      return NoError;
   }

   // Add Case to set
   ErrorHandler::ReturnCode DataDiggerImpl::collectRunResults( ObsSpace & obs, RunCaseSet & rcs )
   {
      RunCaseSetImpl & runCaseSet = dynamic_cast<RunCaseSetImpl&>(rcs);
      ObsSpaceImpl   & observSpace = dynamic_cast<ObsSpaceImpl&>(obs);

      for ( size_t c = 0; c < runCaseSet.size(); ++c )
      {
         RunCaseImpl * runCase = dynamic_cast<RunCaseImpl*>(runCaseSet[c]);
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
      }
      return NoError;
   }

   // Serialize object to the given stream
   bool DataDiggerImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
   {
      bool ok = true;
      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   DataDiggerImpl::DataDiggerImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      std::string  objNameInFile;
      std::string  objType;
      unsigned int objVer;

      bool ok = dz.loadObjectDescription( objType, objNameInFile, objVer );
      if ( objType.compare( typeid(*this).name() ) || objNameInFile.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Deserialization error. Can not load object: " << objName;
      }

      if ( version() < objVer )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "Version of object in file is newer. No forward compatibility!";
      }

      if ( !ok ) throw Exception( DeserializationError ) << "DataDiggerImpl deserialization error";
   }

}

