//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsSpaceImpl.C
/// @brief This file keeps API implementation for observables manager. 

#include "ObsSpaceImpl.h"

#include <cassert>

ErrorHandler::ReturnCode casa::ObsSpaceImpl::addObservable( Observable * prm )
{
   if ( prm ) { m_obsSet.push_back( prm ); }
   else       { return reportError( UndefinedValue, "ObsSpaceImpl::addObservable() no observable was given" ); }
   return NoError;
}

// Serialize object to the given stream
bool casa::ObsSpaceImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
{
   bool ok = true;
   
   // initial implementation of serialization, must exist in all future versions of serialization
   if ( fileVersion >= 0 )
   {
      // register ObsSpace object with serializer to allow other objects to keep reference to it after deserializtion
      CasaSerializer::ObjRefID obsID = sz.ptr2id( this );
      ok = ok ? sz.save( obsID, "ID" ) : ok;

      size_t setSize = m_obsSet.size();

      ok = sz.save( setSize, "ObservablesSetSize" );

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         ok = ok ? sz.save( *(m_obsSet[i]), "Observable" ) : ok;
      }
   }
   return ok;
}

// Create a new instance and deserialize it from the given stream
casa::ObsSpaceImpl::ObsSpaceImpl(CasaDeserializer & dz, const char * objName)
{  
   // read from file object name and version
   unsigned int objVer = version();

   bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

   CasaDeserializer::ObjRefID obsID;

   // load data necessary to create an object
   ok = ok ? dz.load( obsID, "ID" ) : ok;

   // register ObsSpace with deserializer under read ID to allow other objects to keep reference after deserialization
   if ( ok ) { dz.registerObjPtrUnderID( this, obsID ); }

   size_t setSize;

   ok = ok ? dz.load( setSize, "ObservablesSetSize" ) : ok;

   for ( size_t i = 0; i < setSize && ok; ++i )
   {     
      Observable * newObs = Observable::load( dz, "Observable" ); // load observable
      assert( newObs );

      ok = NoError == addObservable( newObs );
   }

   if ( !ok ) throw Exception( DeserializationError ) << "ObsSpaceImpl deserialization error";   
}

