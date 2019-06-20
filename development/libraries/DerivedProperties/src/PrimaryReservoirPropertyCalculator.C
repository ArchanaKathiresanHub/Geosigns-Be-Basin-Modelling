//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimaryReservoirPropertyCalculator.h"

#include "Snapshot.h"
#include "Reservoir.h"
#include "Property.h"
#include "PropertyValue.h"

#include "PrimaryReservoirProperty.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimaryReservoirPropertyCalculator::PrimaryReservoirPropertyCalculator ( const GeoPhysics::ProjectHandle*   projectHandle,
                                                                                            const DataModel::AbstractProperty* property ) :
   m_property ( property )
{

   DataAccess::Interface::PropertyValueList* reservoirProperties = projectHandle->getPropertyValues ( DataAccess::Interface::RESERVOIR, 0, 0, 0, 0, 0, DataAccess::Interface::MAP );
   addPropertyName ( property->getName ());

   for ( size_t i = 0; i < reservoirProperties->size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = (*reservoirProperties)[ i ];

      if ( propVal->getProperty () == m_property and propVal->getReservoir () != 0 and propVal->getFormation () == 0 ) {
         m_snapshots.insert ( propVal->getSnapshot ());
         m_reservoirPropertyValues.push_back ( propVal );
      }

   }

   delete reservoirProperties;
}

DerivedProperties::PrimaryReservoirPropertyCalculator::~PrimaryReservoirPropertyCalculator () {
   m_reservoirPropertyValues.clear ();
}

void DerivedProperties::PrimaryReservoirPropertyCalculator::calculate ( AbstractPropertyManager&            propManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractReservoir* reservoir,
                                                                              ReservoirPropertyList&        derivedProperties ) const {
   (void) propManager;

   derivedProperties.clear ();

   for ( size_t i = 0; i < m_reservoirPropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_reservoirPropertyValues [ i ];

      if ( propVal->getProperty () == m_property and propVal->getReservoir () == reservoir and propVal->getSnapshot () == snapshot ) {
         // Add the property and exit the loop, since there is only a single
         // property associated with the primary reservoir property calculator.
         derivedProperties.push_back ( ReservoirPropertyPtr ( new PrimaryReservoirProperty ( propVal )));
         break;
      }

   }

}

bool DerivedProperties::PrimaryReservoirPropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                           const DataModel::AbstractReservoir* reservoir ) const {

   (void) propManager;

   for ( size_t i = 0; i < m_reservoirPropertyValues.size (); ++i ) {
      const DataAccess::Interface::PropertyValue* propVal = m_reservoirPropertyValues [ i ];

      if ( propVal->getProperty () == m_property and ( reservoir == 0 or propVal->getReservoir () == reservoir ) and ( snapshot == 0 or propVal->getSnapshot () == snapshot )) {
         return true;
      }

   }

   return false;
}

const DataModel::AbstractSnapshotSet& DerivedProperties::PrimaryReservoirPropertyCalculator::getSnapshots () const {
   return m_snapshots;
} 
