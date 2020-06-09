//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValue.cpp
/// @brief This file keeps loaders for all possible types of ObsValue

#include "ErrorHandler.h"

#include "ObsValueDoubleArray.h"
#include "ObsValueDoubleScalar.h"
#include "ObsValueTransformable.h"

casa::ObsValue * casa::ObsValue::load( CasaDeserializer & dz, const char * objName )
{
   std::string  ot; // object type name
   std::string  on; // object name
   unsigned int vr; // object version

   dz.loadObjectDescription( ot, on, vr );
   if ( on.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsValue deserialization error, expected observable with name: " << objName
         << ", but stream gave object with name: " << on;
   }
   if (      ot == "ObsValueDoubleArray"  ) { return new ObsValueDoubleArray(   dz, vr ); }
   else if ( ot == "ObsValueDoubleScalar" ) { return new ObsValueDoubleScalar(  dz, vr ); }
   else if ( ot == "ObsValueTransformable") { return new ObsValueTransformable( dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "ObsValue deserialization error: Unknown type: " << ot;
   }
   return 0;
}
