#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"
#include "DerivedPropertyManager.h"
#include "GeoPhysicalConstants.h"

#include "OverpressureSurfaceCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::OverpressureSurfaceCalculator::OverpressureSurfaceCalculator () {
   addPropertyName ( "OverPressure" );
}

void DerivedProperties::OverpressureSurfaceCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
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

            if( hydrostaticPressure->getA ( i, j ) != undefinedValue && porePressure->getA ( i, j ) != porePressure->getUndefinedValue () ) {
               overpressure->set ( i, j, porePressure->getA ( i, j ) - hydrostaticPressure->getA ( i, j ));
            } else {
               overpressure->set ( i, j, undefinedValue );
            }
         }
      }

      derivedProperties.push_back ( overpressure );
   }

}
