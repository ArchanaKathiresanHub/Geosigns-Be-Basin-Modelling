//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file JobScheduler.C
/// @brief This file keeps loaders for all possible types of JobScheduler

// CMB
#include "ErrorHandler.h"

// CASA
#include "CasaDeserializer.h"
#include "JobSchedulerLocal.h"
#include "JobSchedulerLSF.h"

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
   if (      ot == typeid( casa::JobSchedulerLocal ).name() ) { return new JobSchedulerLocal(  dz, vr ); }
   else if ( ot == typeid( casa::JobSchedulerLSF   ).name() ) { return new JobSchedulerLSF(    dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "JobScheduler deserialization error: Unknown type: " << ot;
   }
   return 0;
}
