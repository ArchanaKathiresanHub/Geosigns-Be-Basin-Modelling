#include "DomainReservoirProperty.h"

#include "Interface/Reservoir.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/PropertyValue.h"

DataAccess::Mining::DomainReservoirProperty::DomainReservoirProperty ( const DomainPropertyCollection*            collection,
                                                                       DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                       const Interface::Snapshot*                 snapshot,
                                                                       const Interface::Property*                 property ) : 
   DomainProperty ( collection, propertyManager, snapshot, property )
{

   if ( property->getType () == Interface::RESERVOIRPROPERTY ) {
      DerivedProperties::FormationMapPropertyList values = propertyManager.getFormationMapProperties ( getProperty (), getSnapshot ());

      for ( size_t i = 0; i < values.size (); ++i ) {
         const Interface::Surface* topSurface = getProjectHandle ()->findSurface ( values [ i ]->getFormation ()->getTopSurfaceName ());

         if ( topSurface != 0 ) {
            m_values [ surface ] = values [ i ];
         } else {
            std::cerr << " Cannot find top surface for formation: " << values [ i ]->getFormation ()->getName () << endl;
         }

      }

   }

}


DataAccess::Mining::DomainReservoirProperty::~DomainReservoirProperty () {
   m_values.clear ();
}


void DataAccess::Mining::DomainReservoirProperty::compute ( const ElementPosition&            position,
                                                                  InterpolatedPropertyValues& evaluations ) {

   if ( position.getSurface () != 0 ) {
      SurfaceToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getSurface ());

      if ( propIter != m_values.end ()) {
         DerivedProperties::FormationMapPropertyPtr grid = propIter->second;
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


double DataAccess::Mining::DomainReservoirProperty::compute ( const ElementPosition& position ) const {

   if ( position.getSurface () != 0 ) {
      SurfaceToPropertyValueMapping::const_iterator propIter = m_values.find ( position.getSurface ());

      if ( propIter != m_values.end ()) {
         DerivedProperties::FormationMapPropertyPtr grid = propIter->second;
         return interpolate2D ( position, grid );
      } else {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}



DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainReservoirPropertyAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                     const Interface::Snapshot*                 snapshot,
                                                                                                     const Interface::Property*                 property ) const {
   return new DomainReservoirProperty ( collection, propertyManager, snapshot, property );
}

