#include "ReflectivitySurfaceCalculator.h"

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"

#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"
#include "DerivedPropertyManager.h"
#include "GeoPhysicalConstants.h"

#include "PropertyRetriever.h"

DerivedProperties::ReflectivitySurfaceCalculator::ReflectivitySurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "Reflectivity" );

   addDependentPropertyName ( "Thickness" );
   addDependentPropertyName ( "BulkDensity" );
   addDependentPropertyName ( "Velocity" );
}

void DerivedProperties::ReflectivitySurfaceCalculator::calculate ( AbstractPropertyManager&            propManager,
                                                                   const DataModel::AbstractSnapshot*  snapshot,
                                                                   const DataModel::AbstractSurface*   surface,
                                                                   SurfacePropertyList&                derivedProperties ) const {

   if ( surface == 0 or snapshot == 0 ) {
      return;
   }

   const DataAccess::Interface::Surface*   daSurface = dynamic_cast<const DataAccess::Interface::Surface*>( surface );
   const DataAccess::Interface::Formation* formationAbove = daSurface->getTopFormation ();
   const DataAccess::Interface::Formation* formationBelow = daSurface->getBottomFormation ();
   const DataAccess::Interface::Surface*   surfaceAbove;

   if ( formationBelow == 0 ) {
      return;
   }

   const DataModel::AbstractProperty* reflectivityProperty = propManager.getProperty ( "Reflectivity" );
   const DataModel::AbstractProperty* thicknessProperty   = propManager.getProperty ( "Thickness" );
   const DataModel::AbstractProperty* bulkDensityProperty = propManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* velocityProperty    = propManager.getProperty ( "Velocity" );

   bool formationAboveFound = formationAbove != 0 and formationAbove->getBottomSurface ()->getSnapshot ()->getTime () > snapshot->getTime ();

   FormationMapPropertyPtr     layerThickness = propManager.getFormationMapProperty ( thicknessProperty, snapshot, formationBelow );
   FormationSurfacePropertyPtr layerBulkDensity = propManager.getFormationSurfaceProperty ( bulkDensityProperty, snapshot, formationBelow, surface );
   FormationSurfacePropertyPtr layerVelocity = propManager.getFormationSurfaceProperty ( velocityProperty, snapshot, formationBelow, surface );

   if ( layerThickness == 0 or layerBulkDensity == 0 or layerVelocity == 0 ) {
      return;
   }

   FormationMapPropertyList     thicknesses;
   FormationSurfacePropertyList bulkDensities;
   FormationSurfacePropertyList velocities;

   // Get thickness, bulk-density and velocity from all surfaces above.
   while ( formationAboveFound ) {
      
      surfaceAbove = formationAbove->getTopSurface ();

      FormationMapPropertyPtr     thickness   = propManager.getFormationMapProperty     ( thicknessProperty,   snapshot, formationAbove );
      FormationSurfacePropertyPtr bulkDensity = propManager.getFormationSurfaceProperty ( bulkDensityProperty, snapshot, formationAbove, formationAbove->getBottomSurface ());
      FormationSurfacePropertyPtr velocity    = propManager.getFormationSurfaceProperty ( velocityProperty,    snapshot, formationAbove, formationAbove->getBottomSurface ());

      thicknesses.push_back ( thickness );
      bulkDensities.push_back ( bulkDensity );
      velocities.push_back ( velocity );

      formationAbove = surfaceAbove->getTopFormation ();

      formationAboveFound = formationAbove != 0 and formationAbove->getBottomSurface ()->getSnapshot ()->getTime () > snapshot->getTime ();
   }

   if ( thicknesses.size () == 0 ) {
      return;
   }

   DerivedSurfacePropertyPtr reflectivity = DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( reflectivityProperty, snapshot, surface, propManager.getMapGrid ()));
   double undefinedValue = reflectivity->getUndefinedValue ();

   for ( unsigned int i = reflectivity->firstI ( true ); i <= reflectivity->lastI ( true ); ++i ) {

      for ( unsigned int j = reflectivity->firstJ ( true ); j <= reflectivity->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j ) and layerThickness->getA ( i, j ) > 0.0 ) {
            unsigned int surfaceIndex = 0;

            // Find first layer that is not 0 thickness at the i, j position.
            while ( surfaceIndex < thicknesses.size ()) {

               if ( thicknesses [ surfaceIndex ]->getA ( i, j ) > 0.0 ) {
                  break;
               }

               ++surfaceIndex;
            }

            double reflectivityValue;

            if ( surfaceIndex == thicknesses.size ()) {
               reflectivityValue = undefinedValue;
            } else {
               const double bulkDensity = layerBulkDensity->getA ( i, j );
               const double bulkDensityAbove = bulkDensities [ surfaceIndex ]->getA ( i, j );

               const double velocity = layerVelocity->getA ( i, j );
               const double velocityAbove = velocities [ surfaceIndex ]->getA ( i, j );

               reflectivityValue = ( bulkDensityAbove * velocityAbove - bulkDensity * velocity ) /
                                   ( bulkDensityAbove * velocityAbove + bulkDensity * velocity );
            }

            reflectivity->set ( i, j, reflectivityValue );
         } else {
            reflectivity->set ( i, j, undefinedValue );
         }

      }

   }

   derivedProperties.push_back ( reflectivity );
}

bool DerivedProperties::ReflectivitySurfaceCalculator::isComputable ( const AbstractPropertyManager&     propManager,
                                                                      const DataModel::AbstractSnapshot* snapshot,
                                                                      const DataModel::AbstractSurface*  surface ) const {

   const DataModel::AbstractProperty* thickness   = propManager.getProperty ( "Thickness" );
   const DataModel::AbstractProperty* bulkDensity = propManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* velocity    = propManager.getProperty ( "Velocity" );

   if ( surface == 0 ) {
      return propManager.formationMapPropertyIsComputable ( thickness, snapshot, 0 ) and 
             propManager.formationPropertyIsComputable ( bulkDensity, snapshot, 0 ) and 
             propManager.formationPropertyIsComputable ( velocity, snapshot, 0 );
   } else {

      const DataModel::AbstractFormation* formationBelow = surface->getBottomFormation ();

      if ( formationBelow != 0 ) {
         return propManager.formationMapPropertyIsComputable ( thickness, snapshot, formationBelow ) and 
                propManager.formationSurfacePropertyIsComputable ( bulkDensity, snapshot, formationBelow, surface ) and 
                propManager.formationSurfacePropertyIsComputable ( velocity, snapshot, formationBelow, surface );
      } else {
         return false;
      }

   }

}
