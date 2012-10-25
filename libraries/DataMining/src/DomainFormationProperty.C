#include "DomainFormationProperty.h"

#include "DomainPropertyCollection.h"
#include "ElementFunctions.h"


DataAccess::Mining::DomainFormationProperty::DomainFormationProperty ( const DomainPropertyCollection*  collection,
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

#if 0
      value->printOn ( std::cout );
#endif

      m_values [ value->getFormation ()] = value;
   }

   delete values;
}

DataAccess::Mining::DomainFormationProperty::~DomainFormationProperty () {

#if 0
   FormationToPropertyValueMapping::iterator gridIter;

   for ( gridIter = m_values.begin (); gridIter != m_values.end (); ++gridIter ) {
      const Interface::GridMap* grid = gridIter->second;
      grid->release ();
   }
#endif

   m_values.clear ();
}

double DataAccess::Mining::DomainFormationProperty::compute ( const ElementPosition& position ) const {

   if ( position.getFormation () != 0 ) {
      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         const Interface::GridMap* grid = propIter->second->getGridMap ();

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
                                                                  InterpolatedPropertyValues& evaluations ) {

   if ( position.getFormation () != 0 ) {

      FormationToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getFormation ());

      if ( propIter != m_values.end ()) {
         const Interface::GridMap* grid = propIter->second->getGridMap ();

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
         const Interface::GridMap* grid = propIter->second->getGridMap ();

         getElementCoefficients ( position, grid, coefficients );
      } else {
         coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
      }

   } else {
      coefficients.fill ( DataAccess::Interface::DefaultUndefinedMapValue );
   }

}


DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainFormationPropertyAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                     const Interface::Snapshot*     snapshot,
                                                                                                     const Interface::Property*     property ) const {
   return new DomainFormationProperty ( collection, snapshot, property );
}
