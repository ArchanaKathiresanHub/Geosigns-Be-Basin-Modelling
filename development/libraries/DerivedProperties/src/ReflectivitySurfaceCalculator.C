//                                                                      
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ReflectivitySurfaceCalculator.h"

#include "Formation.h"
#include "Surface.h"
#include "Snapshot.h"

#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"
#include "DerivedPropertyManager.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsFormation.h"

#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

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
   const GeoPhysics::GeoPhysicsFormation* geoFormationBelow = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formationBelow );
   if( geoFormationBelow->kind() == DataAccess::Interface::BASEMENT_FORMATION ) {
      return;
   }

   const DataModel::AbstractProperty* reflectivityProperty = propManager.getProperty ( "Reflectivity" );
   const DataModel::AbstractProperty* thicknessProperty    = propManager.getProperty ( "Thickness" );
   const DataModel::AbstractProperty* bulkDensityProperty  = propManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* velocityProperty     = propManager.getProperty ( "Velocity" );

   bool formationAboveFound = formationAbove != 0 and 
      ( formationAbove->getBottomSurface ()->getSnapshot () != 0 ?
        formationAbove->getBottomSurface ()->getSnapshot ()->getTime () > snapshot->getTime () : true );

   FormationMapPropertyPtr     layerThickness   = propManager.getFormationMapProperty     ( thicknessProperty,   snapshot, formationBelow );
   FormationSurfacePropertyPtr layerBulkDensity = propManager.getFormationSurfaceProperty ( bulkDensityProperty, snapshot, formationBelow, surface );
   FormationSurfacePropertyPtr layerVelocity    = propManager.getFormationSurfaceProperty ( velocityProperty,    snapshot, formationBelow, surface );

   if ( layerThickness == 0 or layerBulkDensity == 0 or layerVelocity == 0 ) {
      return;
   }
   PropertyRetriever layerThicknessRetriever  ( layerThickness );
   PropertyRetriever layerBulkDensityRetriever( layerBulkDensity );
   PropertyRetriever layerVelocityRetriever   ( layerVelocity );


   FormationMapPropertyList     thicknesses;
   FormationSurfacePropertyList bulkDensities;
   FormationSurfacePropertyList velocities;

   // Get thickness, bulk-density and velocity from all surfaces above.
   while ( formationAboveFound ) {
      
      surfaceAbove = formationAbove->getTopSurface ();

      FormationMapPropertyPtr     thickness   = propManager.getFormationMapProperty     ( thicknessProperty,   snapshot, formationAbove );
      FormationSurfacePropertyPtr bulkDensity = propManager.getFormationSurfaceProperty ( bulkDensityProperty, snapshot, formationAbove, formationAbove->getBottomSurface ());
      FormationSurfacePropertyPtr velocity    = propManager.getFormationSurfaceProperty ( velocityProperty,    snapshot, formationAbove, formationAbove->getBottomSurface ());

      PropertyRetriever thicknessRetriever  ( thickness );
      PropertyRetriever bulkDensityRetriever( bulkDensity );
      PropertyRetriever velocityRetriever   ( velocity );

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

         if ( m_projectHandle->getNodeIsValid ( i, j ) and layerThickness->get ( i, j ) > 0.0 ) {
            unsigned int surfaceIndex = 0;

            // Find first layer that is not 0 thickness at the i, j position.
            while ( surfaceIndex < thicknesses.size ()) {

               if ( thicknesses [ surfaceIndex ]->get ( i, j ) > 0.0 ) {
                  break;
               }

               ++surfaceIndex;
            }

            double reflectivityValue;

            if ( surfaceIndex == thicknesses.size ()) {
               reflectivityValue = undefinedValue;
            } else {
               const double bulkDensity = layerBulkDensity->get ( i, j );
               const double bulkDensityAbove = bulkDensities [ surfaceIndex ]->get ( i, j );

               const double velocity = layerVelocity->get ( i, j );
               const double velocityAbove = velocities [ surfaceIndex ]->get ( i, j );

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
