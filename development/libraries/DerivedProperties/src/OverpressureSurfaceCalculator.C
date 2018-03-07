//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"
#include "DerivedPropertyManager.h"
#include "GeoPhysicalConstants.h"

#include "OverpressureSurfaceCalculator.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::OverpressureSurfaceCalculator::OverpressureSurfaceCalculator () {
   addPropertyName ( "OverPressure" );
}

void DerivedProperties::OverpressureSurfaceCalculator::calculate (       AbstractPropertyManager&     propertyManager,
                                                                   const DataModel::AbstractSnapshot* snapshot,
                                                                   const DataModel::AbstractSurface*  surface,
                                                                         SurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( "HydroStaticPressure" );
   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* overpressureProperty = propertyManager.getProperty ( "OverPressure" );
   
   const SurfacePropertyPtr hydrostaticPressure = propertyManager.getSurfaceProperty ( hydrostaticPressureProperty, snapshot, surface );
   const SurfacePropertyPtr porePressure = propertyManager.getSurfaceProperty ( porePressureProperty, snapshot, surface );
   
   PropertyRetriever lpRetriever ( hydrostaticPressure );
   PropertyRetriever ppRetriever ( porePressure );

   derivedProperties.clear ();
   
   if( hydrostaticPressure != 0 and porePressure != 0 ) {
              
      DerivedSurfacePropertyPtr overpressure = DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( overpressureProperty,
                                                                                                                           snapshot,
                                                                                                                           surface,
                                                                                                                           propertyManager.getMapGrid ()));
      double undefinedValue = hydrostaticPressure->getUndefinedValue ();

      for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {

            if( hydrostaticPressure->get ( i, j ) != undefinedValue && porePressure->get ( i, j ) != porePressure->getUndefinedValue () ) {
               overpressure->set ( i, j, porePressure->get ( i, j ) - hydrostaticPressure->get ( i, j ));
            } else {
               overpressure->set ( i, j, undefinedValue );
            }
         }
      }

      derivedProperties.push_back ( overpressure );
   }

}
