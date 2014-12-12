//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Observable.C
/// @brief This file keeps loaders for all possible types of Observable

#include "ObsGridPropertyXYZ.h"
#include "ObsGridPropertyWell.h"
#include "ObsSourceRockMapProp.h"


casa::Observable * casa::Observable::load( CasaDeserializer & dz, const char * objName )
{
   std::string  ot; // object type name
   std::string  on; // object name
   unsigned int vr; // object version

   dz.loadObjectDescription( ot, on, vr );
   if ( on.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Observable deserialization error, expected observable with name: " << objName
         << ", but stream gave object with name: " << on;
   }
   if (      ot == "ObsGridPropertyXYZ"   ) { return new ObsGridPropertyXYZ(   dz, vr ); }
   else if ( ot == "ObsGridPropertyWell"  ) { return new ObsGridPropertyWell(  dz, vr ); }
   else if ( ot == "ObsSourceRockMapProp" ) { return new ObsSourceRockMapProp( dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Observable deserialization error: Unknown type: " << ot;
   }

   return 0;
}
