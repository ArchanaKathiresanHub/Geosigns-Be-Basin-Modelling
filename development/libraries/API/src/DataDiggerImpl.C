//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataDiggerImpl.h
/// @brief This file keeps API implementation for data digger. 


#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "DataDiggerImpl.h"
#include "ObsSpaceImpl.h"
#include "ObsGridPropertyXYZ.h"
#include "cmbAPI.h"

#include <cassert>

namespace casa
{

   // Create an observable object which will keep given property value for given XYZ coordinates
   Observable * DataDigger::newObsPropertyXYZ( double x, double y, double z, const char * propName, double simTime )
   {
      return new ObsGridPropertyXYZ( x, y, z, propName, simTime );
   }



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
   return reportError( NotImplementedAPI, "DataDiggerImpl::requestObservables not implemented yet" );
}

// Add Case to set
ErrorHandler::ReturnCode DataDiggerImpl::collectRunResults( ObsSpace & obs, RunCaseSet & rcs )
{
   RunCaseSetImpl & runCaseSet = dynamic_cast<RunCaseSetImpl&>( rcs );
   ObsSpaceImpl   & observSpace = dynamic_cast<ObsSpaceImpl&>( obs );

   for ( size_t c = 0; c < runCaseSet.size( ); ++c )
   {
      RunCaseImpl * runCase = dynamic_cast<RunCaseImpl*>( runCaseSet.at( c ) );
      assert( runCase->observablesNumber() == 0 );

      mbapi::Model * caseModel = runCase->caseModel();
      if ( !caseModel )
      {
         caseModel = runCase->loadProject();
         if ( caseModel->errorCode() == NoError ) { return moveError( *caseModel ); }
      }

      for ( size_t ob = 0; ob < observSpace.size( ); ++ob )
      {
         Observable * obDef = observSpace[ ob ];
         runCase->addObservableValue( obDef->getFromModel( *caseModel ) );
      }
   }
   return NoError;
}

}
