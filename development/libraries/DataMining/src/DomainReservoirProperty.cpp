//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DomainReservoirProperty.h"

#include "Reservoir.h"
#include "Formation.h"
#include "Surface.h"
#include "PropertyValue.h"

using namespace AbstractDerivedProperties;

DataAccess::Mining::DomainReservoirProperty::DomainReservoirProperty ( const DomainPropertyCollection*            collection,
                                                                       DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                       const Interface::Snapshot*                 snapshot,
                                                                       const Interface::Property*                 property ) :
   DomainProperty ( collection, propertyManager, snapshot, property )
{

   if ( property->getType () == Interface::RESERVOIRPROPERTY ) {
      FormationMapPropertyList values = propertyManager.getFormationMapProperties ( getProperty (), getSnapshot ());

      for ( size_t i = 0; i < values.size (); ++i ) {
         const Interface::Surface* topSurface = getProjectHandle ()->findSurface ( values [ i ]->getFormation ()->getTopSurfaceName ());

         if ( topSurface != 0 ) {
            m_values [ surface ] = values [ i ];
         } else {
            std::cerr << " Cannot find top surface for formation: " << values [ i ]->getFormation ()->getName () << endl;
         }

      }

   }

}


DataAccess::Mining::DomainReservoirProperty::~DomainReservoirProperty () {
   m_values.clear ();
}

double DataAccess::Mining::DomainReservoirProperty::compute ( const ElementPosition& position ) const {

   if ( position.getSurface () != 0 ) {
      SurfaceToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getSurface ());

      if ( propIter != m_values.end ()) {
         FormationMapPropertyPtr grid = propIter->second;
         return interpolate2D ( position, grid );
      } else {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}



DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainReservoirPropertyAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                     const Interface::Snapshot*                 snapshot,
                                                                                                     const Interface::Property*                 property ) const {
   return new DomainReservoirProperty ( collection, propertyManager, snapshot, property );
}
