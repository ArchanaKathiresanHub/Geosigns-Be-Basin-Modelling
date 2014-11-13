//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Parameter.C
/// @brief This file keeps loaders for all possible types of Parameter

#include "ErrorHandler.h"

#include "PrmOneCrustThinningEvent.h"
#include "PrmSourceRockTOC.h"
#include "PrmTopCrustHeatProduction.h"

casa::Parameter * casa::Parameter::load( CasaDeserializer & dz, const char * objName )
{
   std::string  ot; // object type name
   std::string  on; // object name
   unsigned int vr; // object version

   dz.loadObjectDescription( ot, on, vr );
   if ( on.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Parameter deserialization error, expected parameter with name: " << objName
         << ", but stream gave object with name: " << on;
   }
   if (      ot == typeid( casa::PrmOneCrustThinningEvent  ).name() ) { return new PrmOneCrustThinningEvent(  dz, vr ); }
   else if ( ot == typeid( casa::PrmSourceRockTOC          ).name() ) { return new PrmSourceRockTOC(          dz, vr ); }
   else if ( ot == typeid( casa::PrmTopCrustHeatProduction ).name() ) { return new PrmTopCrustHeatProduction( dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Parameter deserialization error: Unknown type: " << ot;
   }
   return 0;
}
