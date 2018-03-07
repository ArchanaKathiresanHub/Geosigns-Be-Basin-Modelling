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
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicalConstants.h"
#include "OverpressureFormationCalculator.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::OverpressureFormationCalculator::OverpressureFormationCalculator () {
   addPropertyName ( "OverPressure" );

   addDependentPropertyName ( "HydroStaticPressure" );
   addDependentPropertyName ( "Pressure" );
}

void DerivedProperties::OverpressureFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                     const DataModel::AbstractSnapshot*  snapshot,
                                                                     const DataModel::AbstractFormation* formation,
                                                                           FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( "HydroStaticPressure" );
   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* overpressureProperty = propertyManager.getProperty ( "OverPressure" );
   
   const FormationPropertyPtr hydrostaticPressure = propertyManager.getFormationProperty ( hydrostaticPressureProperty, snapshot, formation );
   const FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
   
   PropertyRetriever hpRetriever ( hydrostaticPressure );
   PropertyRetriever ppRetriever ( porePressure );

   derivedProperties.clear ();
   
   if( hydrostaticPressure != 0 and porePressure != 0 ) {
              
      DerivedFormationPropertyPtr overpressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( overpressureProperty,
                                                                                                                                 snapshot,
                                                                                                                                 formation, 
                                                                                                                                 propertyManager.getMapGrid (),
                                                                                                                                 porePressure->lengthK () ));
      double undefinedValue = hydrostaticPressure->getUndefinedValue ();
      
      for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {
            
            if ( propertyManager.getNodeIsValid ( i, j )) {
                
               for ( unsigned int k = hydrostaticPressure->firstK (); k <= hydrostaticPressure->lastK (); ++k ) {
                  overpressure->set ( i, j, k, porePressure->get ( i, j, k ) - hydrostaticPressure->get ( i, j, k ));
               }

            } else {

               for ( unsigned int k = hydrostaticPressure->firstK (); k <= hydrostaticPressure->lastK (); ++k ) {
                   overpressure->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( overpressure );
   }

}
