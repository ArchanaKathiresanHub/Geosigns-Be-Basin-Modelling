//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DomainFormationProperty.h"

#include "DomainPropertyCollection.h"
#include "ElementFunctions.h"

using namespace AbstractDerivedProperties;

DataAccess::Mining::DomainFormationProperty::DomainFormationProperty ( const DomainPropertyCollection*            collection,
                                                                       DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                       const Interface::Snapshot*                 snapshot,
                                                                       const Interface::Property*                 property ) :
   DomainProperty ( collection, propertyManager, snapshot, property )
{

   FormationPropertyList values = propertyManager.getFormationProperties ( getProperty (), getSnapshot (), true );

   for ( size_t i = 0; i < values.size (); ++i ) {
      m_values [ values [ i ]->getFormation ()] = values [ i ];
   }

}

DataAccess::Mining::DomainFormationProperty::~DomainFormationProperty () {
   m_values.clear ();
}

double DataAccess::Mining::DomainFormationProperty::compute ( const ElementPosition& position ) const {

   if ( position.getFormation () != 0 ) {
      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         FormationPropertyPtr grid = propIter->second;

         if ( position.getSurface () == 0 ) {
            return interpolate3D ( position, grid );
         } else {
            return interpolate2D ( position, grid );
         }

      } else {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

void DataAccess::Mining::DomainFormationProperty::extractCoefficients ( const ElementPosition& position,
                                                                              FiniteElementMethod::ElementVector& coefficients ) const {

   if ( position.getFormation () != 0 ) {
      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         FormationPropertyPtr grid = propIter->second;

         getElementCoefficients ( position, grid, coefficients );
      } else {
         coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
      }

   } else {
      coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
   }

}


DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainFormationPropertyAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                     const Interface::Snapshot*                 snapshot,
                                                                                                     const Interface::Property*                 property ) const {
   return new DomainFormationProperty ( collection, propertyManager, snapshot, property );
}
