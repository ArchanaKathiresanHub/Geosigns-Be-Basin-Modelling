//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsSpaceImpl.h
/// @brief This file keeps declaration of the implementation part of observables set manager

#ifndef CASA_API_OBS_SPACE_IMPL_H
#define CASA_API_OBS_SPACE_IMPL_H

// CASA
#include "ObsSpace.h"

// STL
#include <vector>

namespace casa
{
// Observables manager. It keeps a unique set of targets (observables)
class ObsSpaceImpl : public ObsSpace
{
public:  
   // Constructor/Destructor
   ObsSpaceImpl() { ; }

   virtual ~ObsSpaceImpl()
   {
      for ( size_t i = 0; i < m_obsSet.size(); ++i ) delete m_obsSet[i];
      m_obsSet.clear();
   }

   // Add a new observable
   virtual ErrorHandler::ReturnCode addObservable( Observable * prm );

   // Get number of variable parameters defined in ObsSpace
   virtual size_t size() const { return m_obsSet.size(); } 

   // get i-th observable
   Observable * operator[] ( size_t i ) const { return m_obsSet[ i ]; }

   // get i-th observable
   Observable * at( size_t i ) const { return m_obsSet[i]; }

   // Get i-th observable
   virtual const Observable * observable( size_t i ) const { return i < size() ? at( i ) : 0; }

   // Serialization / Deserialization
  
   // version of serialized object representation
   virtual unsigned int version() const { return 0; }

   // Serialize object to the given stream
   virtual bool save( CasaSerializer & sz, unsigned int version ) const;

   // Get type name of the serialaizable object, used in deserialization to create object with correct type
   virtual const char * typeName() const { return "ObsSpaceImpl"; }

   // Create a new instance and deserialize it from the given stream
   ObsSpaceImpl( CasaDeserializer & dz, const char * objName );

private:
   std::vector< Observable*> m_obsSet;

   // disable copy constructor and copy operator
   ObsSpaceImpl( const ObsSpaceImpl & );
   ObsSpaceImpl & operator = ( const ObsSpaceImpl & );
};

}

#endif // CASA_API_OBS_SPACE_IMPL_H
