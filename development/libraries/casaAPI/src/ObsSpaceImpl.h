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

   // Get number of influential parameters defined in ObsSpace
   virtual size_t size() const { return m_obsSet.size(); }

   // Dimension of observables space
   virtual size_t dimension() const { size_t dim = 0; for ( const auto obs : m_obsSet ) dim += obs->dimension(); return dim; };

   // get i-th observable
   virtual Observable * operator[] ( size_t i ) const { return m_obsSet[ i ]; }

   // Get i-th observable
   virtual const Observable * observable( size_t i ) const { return i < size() ? m_obsSet[i] : 0; }

   // Is the given observable valid at least for one case?
   virtual bool isValid( size_t obId, size_t obSubId ) const;

   // Add observable validity status. This function is called for each run case on the stage of extracting observables
   // in data digger.
   virtual void updateObsValueValidateStatus( size_t ob, const std::vector<bool> & valFlags );

   // Serialization / Deserialization

   // version of serialized object representation
   virtual unsigned int version() const { return 1; }

   // Serialize object to the given stream
   virtual bool save(CasaSerializer & sz) const;

   // Get type name of the serialaizable object, used in deserialization to create object with correct type
   virtual std::string typeName() const { return "ObsSpaceImpl"; }

   // Create a new instance and deserialize it from the given stream
   ObsSpaceImpl( CasaDeserializer & dz, const char * objName );

private:
   std::vector<Observable*>       m_obsSet;          // list of observable definitions
   std::vector<std::vector<bool>> m_obsIsValidFlags; // list of arrays of is observable valid flags.
                                                     // It keeps flags accumulation over all run cases.

   // disable copy constructor and copy operator
   ObsSpaceImpl( const ObsSpaceImpl & );
   ObsSpaceImpl & operator = ( const ObsSpaceImpl & );
};

}

#endif // CASA_API_OBS_SPACE_IMPL_H
