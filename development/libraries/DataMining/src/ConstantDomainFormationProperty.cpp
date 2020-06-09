//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ConstantDomainFormationProperty.h"

#include "DomainPropertyCollection.h"
#include "ElementFunctions.h"

#include "GeoPhysicsProjectHandle.h"

using namespace AbstractDerivedProperties;

DataAccess::Mining::ConstantDomainFormationProperty::ConstantDomainFormationProperty ( const DomainPropertyCollection*            collection,
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

DataAccess::Mining::ConstantDomainFormationProperty::~ConstantDomainFormationProperty () {
   m_values.clear ();
}

double DataAccess::Mining::ConstantDomainFormationProperty::compute ( const ElementPosition& position ) const {

   if ( position.getFormation () != 0 ) {
      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         FormationPropertyPtr grid = propIter->second;

         return grid->get ( position.getI (), position.getJ (), position.getLocalK ());
      } else {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

void DataAccess::Mining::ConstantDomainFormationProperty::extractCoefficients ( const ElementPosition& position,
                                                                                FiniteElementMethod::ElementVector& coefficients ) const {

   if ( position.getFormation () != 0 ) {
      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         FormationPropertyPtr grid = propIter->second;

         // Since the property is constant every where on the element setting all coeffs to the same value
         // returns the same constant value in the 3d interpolation functions.
         coefficients.fill ( grid->get ( position.getI (), position.getJ (), position.getLocalK ()));
      } else {
         coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
      }

   } else {
      coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
   }

}

DataAccess::Mining::DomainProperty* DataAccess::Mining::ConstantDomainFormationPropertyAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                             DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                             const Interface::Snapshot*                 snapshot,
                                                                                                             const Interface::Property*                 property ) const {
   return new ConstantDomainFormationProperty ( collection, propertyManager, snapshot, property );
}
