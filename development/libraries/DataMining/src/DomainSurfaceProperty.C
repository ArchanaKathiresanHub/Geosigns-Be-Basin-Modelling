//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DomainSurfaceProperty.h"

#include "PropertyValue.h"

using namespace AbstractDerivedProperties;

DataAccess::Mining::DomainSurfaceProperty::DomainSurfaceProperty ( const DomainPropertyCollection*            collection,
                                                                   DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                   const Interface::Snapshot*                 snapshot,
                                                                   const Interface::Property*                 property ) :
   DomainProperty ( collection, propertyManager, snapshot, property )
{

   SurfacePropertyList values = propertyManager.getSurfaceProperties ( getProperty (), getSnapshot (), true );

   for ( size_t i = 0; i < values.size (); ++i ) {
      m_values [ values [ i ]->getSurface ()] = values [ i ];
   }

}

DataAccess::Mining::DomainSurfaceProperty::~DomainSurfaceProperty () {
   m_values.clear ();
}

double DataAccess::Mining::DomainSurfaceProperty::compute ( const ElementPosition& position ) const {

   if ( position.getSurface () != 0 ) {
      SurfaceToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getSurface ());

      if ( propIter != m_values.end ()) {
         SurfacePropertyPtr grid = propIter->second;
         return interpolate2D ( position, grid );
      } else {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}



DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainSurfacePropertyAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                   DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                   const Interface::Snapshot*                 snapshot,
                                                                                                   const Interface::Property*                 property ) const {
   return new DomainSurfaceProperty ( collection, propertyManager, snapshot, property );
}
