#include "HydrostaticPressureSurfaceCalculator.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"


DerivedProperties::HydrostaticPressureSurfaceCalculator::HydrostaticPressureSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) :
   m_projectHandle ( projectHandle )
{
   addPropertyName ( "HydroStaticPressure" );
}
 
void DerivedProperties::HydrostaticPressureSurfaceCalculator::calculate ( AbstractPropertyManager&           propertyManager,
                                                                          const DataModel::AbstractSnapshot* snapshot,
                                                                          const DataModel::AbstractSurface*  surface,
                                                                                SurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);

   const DataAccess::Interface::Surface* currentSurface = dynamic_cast<const DataAccess::Interface::Surface*>( surface );
   const DataAccess::Interface::Formation* formationBelow = currentSurface->getBottomFormation ();

   DerivedSurfacePropertyPtr hydrostaticPressure = DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( hydrostaticPressureProperty,
                                                                                                                               snapshot,
                                                                                                                               surface, 
                                                                                                                               propertyManager.getMapGrid ()));

   if ( currentSurface->getTopFormation () == 0 or
        snapshot->getTime () == currentSurface->getSnapshot ()->getTime () or 
        ( snapshot->getTime () >= currentSurface->getSnapshot ()->getTime () and
          snapshot->getTime () < formationBelow->getBottomSurface ()->getSnapshot ()->getTime ())) {

      computeHydrostaticPressureAtSeaBottom ( propertyManager, snapshot->getTime (), currentSurface, hydrostaticPressure );
   } else {
      copyHydrostaticPressureFromLayerAbove ( propertyManager, hydrostaticPressureProperty, snapshot, currentSurface, hydrostaticPressure );
   }

   derivedProperties.push_back ( hydrostaticPressure );
}

void DerivedProperties::HydrostaticPressureSurfaceCalculator::computeHydrostaticPressureAtSeaBottom ( const AbstractPropertyManager&        propertyManager,
                                                                                                      const double                          snapshotAge,
                                                                                                      const DataAccess::Interface::Surface* surface,
                                                                                                            DerivedSurfacePropertyPtr&      hydrostaticPressure ) const {

   (void) propertyManager;
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>( surface->getBottomFormation ()->getFluidType ());

   double pressure;

   for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {

      for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            GeoPhysics::computeHydrostaticPressure ( fluid,
                                                     m_projectHandle->getSeaBottomTemperature ( i, j, snapshotAge ),
                                                     m_projectHandle->getSeaBottomDepth ( i, j, snapshotAge ),
                                                     pressure );
            hydrostaticPressure->set ( i, j, pressure );
         }

      }

   }

}

void DerivedProperties::HydrostaticPressureSurfaceCalculator::copyHydrostaticPressureFromLayerAbove ( AbstractPropertyManager&              propertyManager,
                                                                                                      const DataModel::AbstractProperty*    hydrostaticPressureProperty,
                                                                                                      const DataModel::AbstractSnapshot*    snapshot,
                                                                                                      const DataAccess::Interface::Surface* surface,
                                                                                                            DerivedSurfacePropertyPtr&      hydrostaticPressure ) const {

   const DataModel::AbstractFormation* formationAbove = surface->getTopFormation ();

   const FormationPropertyPtr hydrostaticPressureAbove = propertyManager.getFormationProperty ( hydrostaticPressureProperty, snapshot, formationAbove );
   double undefinedValue = hydrostaticPressureAbove->getUndefinedValue ();

   for ( unsigned int i = hydrostaticPressureAbove->firstI ( true ); i <= hydrostaticPressureAbove->lastI ( true ); ++i ) {

      for ( unsigned int j = hydrostaticPressureAbove->firstJ ( true ); j <= hydrostaticPressureAbove->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            hydrostaticPressure->set ( i, j, hydrostaticPressureAbove->get ( i, j, 0 ));
         } else {
            hydrostaticPressure->set ( i, j, undefinedValue );
         }

      }

   }

}
