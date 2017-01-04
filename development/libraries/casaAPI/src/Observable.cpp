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
#include "ObsTrapProp.h"
#include "ObsTrapDerivedProp.h"

#include "UndefinedValues.h"

std::vector<bool> casa::Observable::isValid( const casa::ObsValue * obv ) const
{
   std::vector<bool> ret;

   if ( ! obv ) { return ret; }

   const std::vector<double> & vals = obv->asDoubleArray();
   for ( auto v : vals ) { ret.push_back( !IsValueUndefined( v ) ); }
 
   return ret;
}

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
   else if ( ot == "ObsTrapProp"          ) { return new ObsTrapProp(          dz, vr ); }
   else if ( ot == "ObsTrapDerivedProp"   ) { return new ObsTrapDerivedProp(   dz, vr ); }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Observable deserialization error: Unknown type: " << ot;
   }

   return 0;
}
