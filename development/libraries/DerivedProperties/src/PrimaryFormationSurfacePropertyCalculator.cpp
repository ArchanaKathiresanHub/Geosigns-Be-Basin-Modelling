//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimaryFormationSurfacePropertyCalculator.h"

#include "Formation.h"
#include "Property.h"
#include "PropertyValue.h"
#include "Snapshot.h"
#include "Surface.h"

#include "PrimaryFormationSurfaceProperty.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimaryFormationSurfacePropertyCalculator::PrimaryFormationSurfacePropertyCalculator ( const DataModel::AbstractProperty* property,
                                                                                                          const DataAccess::Interface::PropertyValueList& propertyValues ) :
   m_property ( property ),
   m_formationSurfacePropertyValues ( propertyValues )
{
   addPropertyName ( property->getName ());
}

DerivedProperties::PrimaryFormationSurfacePropertyCalculator::~PrimaryFormationSurfacePropertyCalculator () {
   m_formationSurfacePropertyValues.clear ();
}

void DerivedProperties::PrimaryFormationSurfacePropertyCalculator::calculate ( AbstractPropertyManager&      propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation,
                                                                               const DataModel::AbstractSurface*   surface,
                                                                               FormationSurfacePropertyList& derivedProperties ) const {

   (void) propManager;
   derivedProperties.clear ();

   for ( size_t i = 0; i < m_formationSurfacePropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_formationSurfacePropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getFormation () == formation and propVal->getSurface () == surface and propVal->getSnapshot () == snapshot
           and propVal->getGridMap() != nullptr ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary formation property calculator.
         derivedProperties.push_back ( FormationSurfacePropertyPtr ( new PrimaryFormationSurfaceProperty ( propVal )));
         break;
      }

   }

}

bool DerivedProperties::PrimaryFormationSurfacePropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                                  const DataModel::AbstractSnapshot*  snapshot,
                                                                                  const DataModel::AbstractFormation* formation,
                                                                                  const DataModel::AbstractSurface*   surface ) const {

   (void) propManager;

   for ( size_t i = 0; i < m_formationSurfacePropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_formationSurfacePropertyValues [ i ];

      if ( propVal->getProperty () == m_property and
           ( formation == 0 or propVal->getFormation () == formation ) and
           ( surface   == 0 or propVal->getSurface ()   == surface   ) and
           ( snapshot  == 0 or propVal->getSnapshot ()  == snapshot )) {
         return true;
      }

   }

   return false;
}


const DataModel::AbstractSnapshotSet& DerivedProperties::PrimaryFormationSurfacePropertyCalculator::getSnapshots () const {
   return m_snapshots;
}
