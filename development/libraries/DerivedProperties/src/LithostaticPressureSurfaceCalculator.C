//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
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

#include "LithostaticPressureSurfaceCalculator.h"
#include "PropertyRetriever.h"

// utilities library
#include "ConstantsMathematics.h"

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

            if( ves->getA ( i, j ) != undefinedValue && porePressure->getA ( i, j ) != porePressure->getUndefinedValue () ) {
               lithostaticPressure->set ( i, j, ( ves->getA ( i, j ) * Utilities::Maths::PaToMegaPa + porePressure->getA ( i, j )));
            } else {
               lithostaticPressure->set ( i, j, undefinedValue );
            }
         }
      }

      derivedProperties.push_back ( lithostaticPressure );
   }
}
