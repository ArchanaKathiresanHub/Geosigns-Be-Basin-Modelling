#include "ConstantDomainFormationProperty.h"

#include "DomainPropertyCollection.h"
#include "ElementFunctions.h"

#include "GeoPhysicsProjectHandle.h"

DataAccess::Mining::ConstantDomainFormationProperty::ConstantDomainFormationProperty ( const DomainPropertyCollection*  collection,
                                                                                       const Interface::Snapshot* snapshot,
                                                                                       const Interface::Property* property ) :
   DomainProperty ( collection, snapshot, property )
{

   Interface::PropertyValueList* values;
   Interface::PropertyValueList::const_iterator valueIter;

   values = getProjectHandle ()->getPropertyValues ( Interface::FORMATION,// | Interface::FORMATIONSURFACE,
                                                     getProperty (),
                                                     getSnapshot ());

   for ( valueIter = values->begin (); valueIter != values->end (); ++valueIter ) {
      const Interface::PropertyValue* value = *valueIter;

      m_values [ value->getFormation ()] = value;
   }

   delete values;
}

DataAccess::Mining::ConstantDomainFormationProperty::~ConstantDomainFormationProperty () {

   m_values.clear ();
}

double DataAccess::Mining::ConstantDomainFormationProperty::compute ( const ElementPosition& position ) const {

   if ( position.getFormation () != 0 ) {
      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         const Interface::GridMap* grid = propIter->second->getGridMap ();

         return grid->getValue ( position.getI (), position.getJ (), position.getLocalK ());
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
         const Interface::GridMap* grid = propIter->second->getGridMap ();

         // Since the property is constant every where on the element setting all coeffs to the same value
         // returns the same constant value in the 3d interpolation functions.
         coefficients.fill ( grid->getValue ( position.getI (), position.getJ (), position.getLocalK ()));
      } else {
         coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
      }

   } else {
      coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
   }

}

DataAccess::Mining::DomainProperty* DataAccess::Mining::ConstantDomainFormationPropertyAllocator::allocate ( const DomainPropertyCollection* collection,
                                                                                                             const Interface::Snapshot*      snapshot,
                                                                                                             const Interface::Property*      property ) const {
   return new ConstantDomainFormationProperty ( collection, snapshot, property );
}
