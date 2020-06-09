//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file ObsSpaceImpl.cpp
/// @brief This file keeps API implementation for observables manager.

#include "ObsSpaceImpl.h"

#include <cassert>

ErrorHandler::ReturnCode casa::ObsSpaceImpl::addObservable( Observable * prm )
{
   if ( prm != nullptr )
   {
      m_obsSet.push_back( prm );
      m_obsIsValidFlags.push_back( std::vector<bool>( prm->dimension(), false ) );
   }
   else { return reportError( UndefinedValue, "ObsSpaceImpl::addObservable() no observable was given" ); }
   return NoError;
}

// Is the given observable valid at least for one case?
bool casa::ObsSpaceImpl::isValid( size_t obId, size_t obSubId ) const
{
   assert( obId < m_obsIsValidFlags.size() );
   assert( obSubId < m_obsIsValidFlags[obId].size() );

   return m_obsIsValidFlags[obId][obSubId];
}

// Add observable validity status. If at least on case has valid observable value - flag is set as valid
void casa::ObsSpaceImpl::updateObsValueValidateStatus( size_t ob, const std::vector<bool> & valFlags )
{
   assert( ob < m_obsIsValidFlags.size() );
   assert( valFlags.size() == m_obsIsValidFlags[ob].size() );

   for ( size_t i = 0; i < valFlags.size(); ++i )
   {
      m_obsIsValidFlags[ob][i] = m_obsIsValidFlags[ob][i] || valFlags[i];  // if at least one observable value is valid, mark observable as valid
   }
}

// Serialize object to the given stream
bool casa::ObsSpaceImpl::save( CasaSerializer & sz ) const
{
   bool ok = true;

   // initial implementation of serialization, must exist in all future versions of serialization
   // register ObsSpace object with serializer to allow other objects to keep reference to it after deserializtion
   CasaSerializer::ObjRefID obsID = sz.ptr2id( this );
   ok = ok ? sz.save( obsID, "ID" ) : ok;

   size_t setSize = m_obsSet.size();

   ok = sz.save( setSize, "ObservablesSetSize" );

   for ( size_t i = 0; i < setSize && ok; ++i )
   {
      ok = ok ? sz.save( *(m_obsSet[i]), "Observable" ) : ok;
   }

   // Version 1
   for ( size_t i = 0; i < setSize; ++i )
   {
      ok = ok ? sz.save( m_obsIsValidFlags[i], "ObsIsValidFlags" ) : ok;
   }

   return ok;
}

// Create a new instance and deserialize it from the given stream
casa::ObsSpaceImpl::ObsSpaceImpl(CasaDeserializer & dz, const char * objName) :
  m_obsSet{},
  m_obsIsValidFlags{}
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

   // Version 1
   if ( objVer > 0 )
   {  // load observable is valid status flags

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         ok = ok ? dz.load( m_obsIsValidFlags[i], "ObsIsValidFlags" ) : ok;
      }
   }
   else
   {  // set all observables as valid, because we can't check them for old version of casa state file
      for ( std::vector<bool>& obsIsValidFlag : m_obsIsValidFlags )
      {
         std::fill(obsIsValidFlag.begin(), obsIsValidFlag.end(), true);
      }
   }

   if ( !ok ) throw Exception( DeserializationError ) << "ObsSpaceImpl deserialization error";
}

