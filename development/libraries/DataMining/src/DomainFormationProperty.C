#include "DomainFormationProperty.h"

#include "DomainPropertyCollection.h"
#include "ElementFunctions.h"


DataAccess::Mining::DomainFormationProperty::DomainFormationProperty ( const DomainPropertyCollection*            collection,
                                                                       DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                       const Interface::Snapshot*                 snapshot,
                                                                       const Interface::Property*                 property ) :
   DomainProperty ( collection, propertyManager, snapshot, property )
{

   DerivedProperties::FormationPropertyList values = propertyManager.getFormationProperties ( getProperty (), getSnapshot (), true );

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
         DerivedProperties::FormationPropertyPtr grid = propIter->second;

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

void DataAccess::Mining::DomainFormationProperty::compute ( const ElementPosition&            position,
                                                                  InterpolatedPropertyValues& evaluations ) const {

   if ( position.getFormation () != 0 ) {

      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         DerivedProperties::FormationPropertyPtr grid = propIter->second;

         if ( position.getSurface () == 0 ) {
            evaluations.setValue ( getProperty (), interpolate3D ( position, grid ));
         } else {
            evaluations.setValue ( getProperty (), interpolate2D ( position, grid ));
         }

      } else {
         evaluations.setValue ( getProperty (), DataAccess::Interface::DefaultUndefinedMapValue );
      }

   } else {

      if ( not evaluations.contains ( getProperty ())) {
         // What kind of error is this?
         evaluations.setValue ( getProperty (), DataAccess::Interface::DefaultUndefinedMapValue );
      }

   }

}

void DataAccess::Mining::DomainFormationProperty::extractCoefficients ( const ElementPosition& position,
                                                                              FiniteElementMethod::ElementVector& coefficients ) const {

   if ( position.getFormation () != 0 ) {
      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         DerivedProperties::FormationPropertyPtr grid = propIter->second;

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
