//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimarySurfacePropertyCalculator.h"

#include "Snapshot.h"
#include "Surface.h"
#include "Property.h"
#include "PropertyValue.h"

#include "PrimarySurfaceProperty.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimarySurfacePropertyCalculator::PrimarySurfacePropertyCalculator ( const DataModel::AbstractProperty* property,
                                                                                        const DataAccess::Interface::PropertyValueList& propertyValues ) :
   m_property ( property ),
   m_surfacePropertyValues ( propertyValues )
{   
   addPropertyName ( property->getName ());
}

DerivedProperties::PrimarySurfacePropertyCalculator::~PrimarySurfacePropertyCalculator () {
   m_surfacePropertyValues.clear ();
}

void DerivedProperties::PrimarySurfacePropertyCalculator::calculate ( AbstractPropertyManager&           propManager,
                                                                      const DataModel::AbstractSnapshot* snapshot,
                                                                      const DataModel::AbstractSurface*  surface,
                                                                            SurfacePropertyList&         derivedProperties ) const {
   (void) propManager;

   derivedProperties.clear ();

   for ( size_t i = 0; i < m_surfacePropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_surfacePropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getSurface () == surface and propVal->getSnapshot () == snapshot and propVal->getGridMap() != nullptr ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary surface property calculator.
         derivedProperties.push_back ( SurfacePropertyPtr ( new PrimarySurfaceProperty ( propVal )));
         break;
      }

   }

}

bool DerivedProperties::PrimarySurfacePropertyCalculator::isComputable ( const AbstractPropertyManager&     propManager,
                                                                         const DataModel::AbstractSnapshot* snapshot,
                                                                         const DataModel::AbstractSurface*  surface ) const {

   (void) propManager;

   for ( size_t i = 0; i < m_surfacePropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_surfacePropertyValues [ i ];

      if ( propVal->getProperty () == m_property and ( surface == 0 or propVal->getSurface () == surface ) and ( snapshot == 0 or propVal->getSnapshot () == snapshot )) {
         return true;
      }

   }

   return false;
}

