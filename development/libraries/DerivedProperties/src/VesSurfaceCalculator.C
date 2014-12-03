#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"
#include "DerivedPropertyManager.h"
#include "GeoPhysicalConstants.h"

#include "VesSurfaceCalculator.h"

#include "PropertyRetriever.h"

DerivedProperties::VesSurfaceCalculator::VesSurfaceCalculator () {
   addPropertyName ( "Ves" );
}

void DerivedProperties::VesSurfaceCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
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
   
   if( lithostaticPressure != 0 and porePressure != 0 ) {
              
      DerivedSurfacePropertyPtr ves = DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( aVesProperty, snapshot, surface, propertyManager.getMapGrid () ));
      double undefinedValue = lithostaticPressure->getUndefinedValue ();

      for ( unsigned int i = lithostaticPressure->firstI ( true ); i <= lithostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = lithostaticPressure->firstJ ( true ); j <= lithostaticPressure->lastJ ( true ); ++j ) {

            if( lithostaticPressure->get ( i, j ) != undefinedValue && porePressure->get ( i, j ) != porePressure->getUndefinedValue () ) {
               ves->set ( i, j, ( lithostaticPressure->get ( i, j ) - porePressure->get ( i, j )) * GeoPhysics::MPa_To_Pa );
            } else {
               ves->set ( i, j, undefinedValue );
            }
         }
      }

      derivedProperties.push_back ( ves );
   }

}
