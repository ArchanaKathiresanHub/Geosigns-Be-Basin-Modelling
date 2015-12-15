#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"
#include "DerivedPropertyManager.h"
#include "GeoPhysicalConstants.h"

#include "LithostaticPressureSurfaceCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::LithostaticPressureSurfaceCalculator::LithostaticPressureSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "LithoStaticPressure" );
}

void DerivedProperties::LithostaticPressureSurfaceCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                          const DataModel::AbstractSnapshot* snapshot,
                                                                          const DataModel::AbstractSurface*  surface,
                                                                                SurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aPorePressureProperty = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   
   const SurfacePropertyPtr ves          = propertyManager.getSurfaceProperty ( aVesProperty, snapshot, surface );
   const SurfacePropertyPtr porePressure = propertyManager.getSurfaceProperty ( aPorePressureProperty, snapshot, surface );

   PropertyRetriever vesRetriever ( ves );
   PropertyRetriever ppRetriever ( porePressure );
   
   derivedProperties.clear ();
   
   if( ves != 0 and porePressure != 0 ) {
              
      DerivedSurfacePropertyPtr lithostaticPressure =
         DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( aLithostaticPressureProperty, snapshot, surface, propertyManager.getMapGrid () ));

      double undefinedValue = ves->getUndefinedValue ();
      
      for ( unsigned int i = ves->firstI ( true ); i <= ves->lastI ( true ); ++i ) {
         
         for ( unsigned int j = ves->firstJ ( true ); j <= ves->lastJ ( true ); ++j ) {

            if( ves->get ( i, j ) != undefinedValue && porePressure->get ( i, j ) != porePressure->getUndefinedValue () ) {
               lithostaticPressure->set ( i, j, ( ves->get ( i, j ) * GeoPhysics::PascalsToMegaPascals + porePressure->get ( i, j )));
            } else {
               lithostaticPressure->set ( i, j, undefinedValue );
            }
         }
      }

      derivedProperties.push_back ( lithostaticPressure );
   }
}
