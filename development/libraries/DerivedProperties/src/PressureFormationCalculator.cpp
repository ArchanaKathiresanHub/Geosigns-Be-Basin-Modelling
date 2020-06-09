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
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "AbstractProperty.h"

#include "Interface.h"
#include "SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "PressureFormationCalculator.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PressureFormationCalculator::PressureFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "Pressure" );

   addDependentPropertyName ( "HydroStaticPressure" );
}

void DerivedProperties::PressureFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( "HydroStaticPressure" );
   const FormationPropertyPtr hydrostaticPressure  = propertyManager.getFormationProperty ( hydrostaticPressureProperty,  snapshot, formation );

   if ( hydrostaticPressure != 0 ) {
      DerivedFormationPropertyPtr porePressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( porePressureProperty, snapshot, formation,
                                                                                                                                 propertyManager.getMapGrid (),
                                                                                                                                 hydrostaticPressure->lengthK ()));

      PropertyRetriever hydrostaticPressureRetriever ( hydrostaticPressure );

      derivedProperties.clear ();

      // now copy the hydrostatic pore pressure to the pore pressure

      for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {

         for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {

            for ( unsigned int k = hydrostaticPressure->firstK (); k <= hydrostaticPressure->lastK (); ++k ) {
               porePressure->set ( i, j, k, hydrostaticPressure->get ( i, j, k ));
            }
         }
      }
      derivedProperties.push_back ( porePressure );
   }

}
