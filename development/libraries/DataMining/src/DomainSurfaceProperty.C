#include "DomainSurfaceProperty.h"

#include "Interface/PropertyValue.h"

DataAccess::Mining::DomainSurfaceProperty::DomainSurfaceProperty ( const DomainPropertyCollection*  collection,
                                                                   const Interface::Snapshot* snapshot,
                                                                   const Interface::Property* property ) :
   DomainProperty ( collection, snapshot, property )
{

   Interface::PropertyValueList* values;
   Interface::PropertyValueList::const_iterator valueIter;

   values = getProjectHandle ()->getPropertyValues ( Interface::SURFACE,
                                                     getProperty (),
                                                     getSnapshot ());

   for ( valueIter = values->begin (); valueIter != values->end (); ++valueIter ) {
      const Interface::PropertyValue* value = *valueIter;

      m_values [ value->getSurface ()] = value;
   }

   delete values;
}

DataAccess::Mining::DomainSurfaceProperty::~DomainSurfaceProperty () {

   SurfaceToPropertyValueMapping::iterator gridIter;

#if 0
   for ( gridIter = m_values.begin (); gridIter != m_values.end (); ++gridIter ) {
      const Interface::GridMap* grid = gridIter->second;
      grid->release ();
   }
#endif

   m_values.clear ();
}

void DataAccess::Mining::DomainSurfaceProperty::compute ( const ElementPosition&            position,
                                                                InterpolatedPropertyValues& evaluations ) {

   if ( position.getSurface () != 0 ) {
      SurfaceToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getSurface ());

      if ( propIter != m_values.end ()) {
         const Interface::GridMap* grid = propIter->second->getGridMap ();
         evaluations.setValue ( getProperty (), interpolate2D ( position, grid ));
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


double DataAccess::Mining::DomainSurfaceProperty::compute ( const ElementPosition& position ) const {

   if ( position.getSurface () != 0 ) {
      SurfaceToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getSurface ());

      if ( propIter != m_values.end ()) {
         const Interface::GridMap* grid = propIter->second->getGridMap ();
         return interpolate2D ( position, grid );
      } else {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}



DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainSurfacePropertyAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                   const Interface::Snapshot* snapshot,
                                                                                                   const Interface::Property* property ) const {
   return new DomainSurfaceProperty ( collection, snapshot, property );
}

