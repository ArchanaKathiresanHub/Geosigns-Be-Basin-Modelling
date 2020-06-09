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

#include "RunParameters.h"
#include "SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"
#include "FracturePressureCalculator.h"

#include "FracturePressureFormationCalculator.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::FracturePressureFormationCalculator::FracturePressureFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "FracturePressure" );

   addDependentPropertyName ( "Depth" );
   addDependentPropertyName ( "HydroStaticPressure" );
   addDependentPropertyName ( "LithoStaticPressure" );
}

void DerivedProperties::FracturePressureFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                                         const DataModel::AbstractFormation* formation,
                                                                               FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* depthProperty = propertyManager.getProperty ( "Depth" );
   const DataModel::AbstractProperty* fracturePressureProperty = propertyManager.getProperty ( "FracturePressure" );
   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( "HydroStaticPressure" );
   const DataModel::AbstractProperty* lithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );

   const FormationPropertyPtr depth               = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );
   const FormationPropertyPtr hydrostaticPressure = propertyManager.getFormationProperty ( hydrostaticPressureProperty, snapshot, formation );
   const FormationPropertyPtr lithostaticPressure = propertyManager.getFormationProperty ( lithostaticPressureProperty, snapshot, formation );

   const GeoPhysics::GeoPhysicsFormation* geoFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle.getDetailsOfLastFastcauldron();
   bool hydrostaticMode = ( lastFastcauldronRun != 0 and
                            ( lastFastcauldronRun->getSimulatorMode () == "HydrostaticDecompaction" or
                              lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" ));

   if ( depth != 0 and hydrostaticPressure != 0 and lithostaticPressure != 0 and geoFormation != 0 ) {
      const double age = snapshot->getTime ();
      const GeoPhysics::FracturePressureCalculator& fracturePressureCalculator = m_projectHandle.getFracturePressureCalculator();

      DerivedFormationPropertyPtr fracturePressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( fracturePressureProperty, snapshot, formation,
                                                                                                                                     propertyManager.getMapGrid (),
                                                                                                                                     geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever depthRetriever ( depth );
      PropertyRetriever hydrostaticPressureRetriever ( hydrostaticPressure );
      PropertyRetriever lithostaticPressureRetriever ( lithostaticPressure );

      const GeoPhysics::CompoundLithologyArray * lithologies = &geoFormation->getCompoundLithologyArray ();
      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(geoFormation->getFluidType ());

      if( hydrostaticMode ) {
         ( (GeoPhysics::FluidType*) fluid )->setDensityToConstant ();
      }

      // We could use any of the formation-properties here to get the undefined value.
      double undefinedValue = depth->getUndefinedValue ();
      double pressureValue;

      for ( unsigned int i = fracturePressure->firstI ( true ); i <= fracturePressure->lastI ( true ); ++i ) {

         for ( unsigned int j = fracturePressure->firstJ ( true ); j <= fracturePressure->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {
               const GeoPhysics::CompoundLithology* lithology = (*lithologies)( i, j, age );

               double seaTemperature = m_projectHandle.getSeaBottomTemperature ( i, j, age );
               double surfaceDepth = m_projectHandle.getSeaBottomDepth ( i, j, age );

               for ( unsigned int k = fracturePressure->firstK (); k <= fracturePressure->lastK (); ++k ) {
                  pressureValue = fracturePressureCalculator.fracturePressure ( lithology,
                                                                                fluid,
                                                                                seaTemperature,
                                                                                surfaceDepth,
                                                                                depth->get ( i, j, k ),
                                                                                hydrostaticPressure->get ( i, j, k ),
                                                                                lithostaticPressure->get ( i, j, k ));
                  fracturePressure->set ( i, j, k, pressureValue );
               }

            } else {

               for ( unsigned int k = fracturePressure->firstK (); k <= fracturePressure->lastK (); ++k ) {
                  fracturePressure->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( fracturePressure );
   }

}
