//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultElementFormationMapCalculator.h"

#include "DerivedFormationMapProperty.h"
#include "GeoPhysicsFormation.h"
#include "Surface.h"
#include "Snapshot.h"

using namespace AbstractDerivedProperties;

DerivedProperties::FaultElementFormationMapCalculator::FaultElementFormationMapCalculator () {
   addPropertyName ( "FaultElements" );
}

void DerivedProperties::FaultElementFormationMapCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation,
                                                                              FormationMapPropertyList&     derivedProperties ) const {


   const GeoPhysics::GeoPhysicsFormation* geophysicsFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   if( not geophysicsFormation->getContainsFaults () ) {
      return;
   }
   if ( snapshot->getTime() > geophysicsFormation->getTopSurface()->getSnapshot()->getTime() ) {
      return;
   }

   const DataModel::AbstractProperty* faultElementProperty = propertyManager.getProperty ( "FaultElements" );

   derivedProperties.clear ();

   DerivedFormationMapPropertyPtr faultElements = DerivedFormationMapPropertyPtr ( new DerivedProperties::DerivedFormationMapProperty ( faultElementProperty,
                                                                                                                                        snapshot,
                                                                                                                                        formation,
                                                                                                                                        propertyManager.getMapGrid ()));

   double undefinedValue = faultElements->getUndefinedValue ();

   for ( unsigned int i = faultElements->firstI ( true ); i <= faultElements->lastI ( true ); ++i ) {

      for ( unsigned int j = faultElements->firstJ ( true ); j <= faultElements->lastJ ( true ); ++j ) {

         if ( propertyManager.getNodeIsValid ( i, j )) {

            if ( geophysicsFormation->getCompoundLithologyArray()( i, j, snapshot->getTime() )->isFault ()) {
               faultElements->set ( i, j, 1.0 );
            } else {
               faultElements->set ( i, j, 0.0 );
            }

         } else {
            faultElements->set ( i, j, undefinedValue );
         }

      }

   }

   derivedProperties.push_back ( faultElements );
}
