//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file JobScheduler.cpp
/// @brief This file keeps loaders for all possible types of JobScheduler

// CMB
#include "ErrorHandler.h"

// FileSystem
#include "FilePath.h"

// CASA
#include "CasaDeserializer.h"
#include "JobSchedulerLocal.h"
#include "JobSchedulerCluster.h"

casa::JobScheduler * casa::JobScheduler::load( CasaDeserializer & dz, const char * objName )
{
   std::string  ot; // object type name
   std::string  on; // object name
   unsigned int vr; // object version

   dz.loadObjectDescription( ot, on, vr );
   if ( on.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "JobScheduler deserialization error, expected observable with name: " << objName
         << ", but stream gave object with name: " << on;
   }
   if (      ot == "JobSchedulerLocal" ) { return new JobSchedulerLocal(  dz, vr ); }
#ifndef WIN32
   else if ( ot == "JobSchedulerCluster" || ot == "JobSchedulerLSF" ) { return new JobSchedulerCluster( dz, vr ); }
#else
   else if (ot == "JobSchedulerCluster" || ot == "JobSchedulerLSF") { return new JobSchedulerLocal(); }
#endif // !WIN32
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "JobScheduler deserialization error: Unknown type: " << ot;
   }
   return 0;
}

casa::JobScheduler::JobState casa::JobScheduler::restoreJobState( const std::string & // cwd
                                                                , const std::string & scriptName
                                                                , const std::string & // jobName
                                                                )
{
   if (      ibs::FilePath ( scriptName + ".success" ).exists() ) { return JobScheduler::JobFinished; }
   else if ( ibs::FilePath ( scriptName + ".failed"  ).exists() ) { return JobScheduler::JobFailed; }

   return JobScheduler::NotSubmittedYet;
}
