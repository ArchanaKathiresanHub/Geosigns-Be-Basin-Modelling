//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimaryFormationPropertyCalculator.h"

#include "Snapshot.h"
#include "Formation.h"
#include "Property.h"
#include "PropertyValue.h"

#include "PrimaryFormationProperty.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimaryFormationPropertyCalculator::PrimaryFormationPropertyCalculator ( const DataModel::AbstractProperty* property,
                                                                                            const DataAccess::Interface::PropertyValueList& propertyValues ) :
   m_property ( property ),
   m_formationPropertyValues ( propertyValues )
{
   addPropertyName ( property->getName ());
}

DerivedProperties::PrimaryFormationPropertyCalculator::~PrimaryFormationPropertyCalculator () {
   m_formationPropertyValues.clear ();
}

void DerivedProperties::PrimaryFormationPropertyCalculator::calculate ( AbstractPropertyManager&            propManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation,
                                                                              FormationPropertyList&        derivedProperties ) const {

   (void) propManager;
   derivedProperties.clear ();

   for ( size_t i = 0; i < m_formationPropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_formationPropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getFormation () == formation and propVal->getSnapshot () == snapshot and propVal->getGridMap() != nullptr ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary formation property calculator.
         derivedProperties.push_back ( FormationPropertyPtr ( new PrimaryFormationProperty ( propVal )));
         break;
      }

   }

}

bool DerivedProperties::PrimaryFormationPropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                           const DataModel::AbstractFormation* formation ) const {

   (void) propManager;

   for ( size_t i = 0; i < m_formationPropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_formationPropertyValues [ i ];

      if ( propVal->getProperty () == m_property and ( formation == 0 or propVal->getFormation () == formation ) and ( snapshot == 0 or propVal->getSnapshot () == snapshot )) {
         return true;
      }

   }

   return false;
}

