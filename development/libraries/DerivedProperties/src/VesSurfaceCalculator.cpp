//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
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

#include "VesSurfaceCalculator.h"

#include "PropertyRetriever.h"

// utilities library
#include "ConstantsMathematics.h"

using namespace AbstractDerivedProperties;

DerivedProperties::VesSurfaceCalculator::VesSurfaceCalculator () {
   addPropertyName ( "Ves" );
   addDependentPropertyName("LithoStaticPressure");
   addDependentPropertyName("Pressure");
}

void DerivedProperties::VesSurfaceCalculator::calculate (       AbstractPropertyManager&     propertyManager,
                                                          const DataModel::AbstractSnapshot* snapshot,
                                                          const DataModel::AbstractSurface*  surface,
                                                                SurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* aPorePressureProperty = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );

   const SurfacePropertyPtr lithostaticPressure = propertyManager.getSurfaceProperty ( aLithostaticPressureProperty, snapshot, surface );
   const SurfacePropertyPtr porePressure = propertyManager.getSurfaceProperty ( aPorePressureProperty, snapshot, surface );

   PropertyRetriever lpRetriever ( lithostaticPressure );
   PropertyRetriever ppRetriever ( porePressure );

   derivedProperties.clear ();

   if( lithostaticPressure != nullptr and porePressure != nullptr ) {

      DerivedSurfacePropertyPtr ves = DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( aVesProperty, snapshot, surface, propertyManager.getMapGrid () ));
      double undefinedValue = lithostaticPressure->getUndefinedValue ();

      for ( unsigned int i = lithostaticPressure->firstI ( true ); i <= lithostaticPressure->lastI ( true ); ++i ) {

         for ( unsigned int j = lithostaticPressure->firstJ ( true ); j <= lithostaticPressure->lastJ ( true ); ++j ) {

            if( lithostaticPressure->get ( i, j ) != undefinedValue && porePressure->get ( i, j ) != porePressure->getUndefinedValue () ) {
               ves->set ( i, j, ( lithostaticPressure->get ( i, j ) - porePressure->get ( i, j )) * Utilities::Maths::MegaPaToPa );
            } else {
               ves->set ( i, j, undefinedValue );
            }
         }
      }

      derivedProperties.push_back ( ves );
   }

}
