#include "ErosionFactorFormationMapCalculator.h"

#include <string>

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedFormationMapProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/ProjectHandle.h"
#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"

#include "PropertyRetriever.h"

DerivedProperties::ErosionFactorFormationMapCalculator::ErosionFactorFormationMapCalculator () {
   addPropertyName ( "ErosionFactor" );
}

void DerivedProperties::ErosionFactorFormationMapCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                                         const DataModel::AbstractFormation* formation,
                                                                               FormationMapPropertyList&     derivedProperties ) const {

   const DataModel::AbstractProperty* erosionFactorProperty = propertyManager.getProperty ( "ErosionFactor" );
   const GeoPhysics::Formation* geophysicsFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   if ( geophysicsFormation != 0 ) {
      double snapshotAge = snapshot->getTime ();
      double depositionAge = geophysicsFormation->getTopSurface ()->getSnapshot ()->getTime ();
      bool isNonGeometricLoop = geophysicsFormation->getProjectHandle ()->getRunParameters ()->getNonGeometricLoop ();

      DataAccess::Interface::SimulationDetailsListPtr allSimulationDetails = geophysicsFormation->getProjectHandle ()->getSimulationDetails ();

      bool coupledMode = false;

      // Because the list is ordered by simulation sequence, i.e. the latest simulation 
      // is last on the list then we need only find the last fastcauldron simulation.
      for ( size_t i = allSimulationDetails->size (); i >= 1 ; --i ) {
         // Notice the i - 1 here. This is to simplify the loop range for an unsigned integer.
         const DataAccess::Interface::SimulationDetails* simulationDetails = (*allSimulationDetails)[ i - 1 ];

         if ( simulationDetails->getSimulatorName () == "fastcauldron" ) {

            // Search for the last fastcauldron simulation that was either hydrostatic or coupled
            if ( simulationDetails->getSimulatorMode ().find ( "Hydrostatic" ) != std::string::npos ) {
               // Possible values are:
               //    HydrostaticDecompaction
               //    HydrostaticHighResDecompaction
               //    HydrostaticTemperature
               //    HydrostaticDarcy
               //
               // If any of these values are present then this indicates that the last simulation was in hydrostatic mode.

               coupledMode = false;
               break;
            } else if ( simulationDetails->getSimulatorMode ().find ( "Coupled" ) != std::string::npos or
                        simulationDetails->getSimulatorMode () == "Overpressure" ) {
               // Possible values are:
               //    LooselyCoupledTemperature
               //    CoupledHighResDecompaction
               //    CoupledPressureAndTemperature
               //    CoupledDarcy
               //    Overpressure
               //
               // If any of these values are present then this indicates that the last simulation was in coupled or overpressure mode.

               coupledMode = true;
               break;
            }

         }

      }

      derivedProperties.clear ();

      DerivedFormationMapPropertyPtr erosionFactor = DerivedFormationMapPropertyPtr ( new DerivedProperties::DerivedFormationMapProperty ( erosionFactorProperty,
                                                                                                                                           snapshot,
                                                                                                                                           formation,
                                                                                                                                           propertyManager.getMapGrid ()));

      double undefinedValue = erosionFactor->getUndefinedValue ();
      double value;
      double depositionThickness;
      double currentThickness;

      for ( unsigned int i = erosionFactor->firstI ( true ); i <= erosionFactor->lastI ( true ); ++i ) {

         for ( unsigned int j = erosionFactor->firstJ ( true ); j <= erosionFactor->lastJ ( true ); ++j ) {

            if ( propertyManager.getNodeIsValid ( i, j )) {

               if ( snapshotAge < depositionAge ) {
                  depositionThickness = 0.0;
                  currentThickness = 0.0;

                  if ( coupledMode and isNonGeometricLoop ) {

                     for ( unsigned int k = 0; k <= geophysicsFormation->getMaximumNumberOfElements (); ++k ) {
                        depositionThickness += geophysicsFormation->getRealThickness ( i, j, k, depositionAge );
                        currentThickness += geophysicsFormation->getRealThickness ( i, j, k, snapshotAge );
                     }

                  } else {

                     for ( unsigned int k = 0; k <= geophysicsFormation->getMaximumNumberOfElements (); ++k ) {
                        depositionThickness += geophysicsFormation->getSolidThickness ( i, j, k, depositionAge );
                        currentThickness += geophysicsFormation->getSolidThickness ( i, j, k, snapshotAge );
                     }

                  }

                  if ( depositionThickness == 0.0 ) {
                     value = 1.0;
                  } else {
                     value = currentThickness / depositionThickness;
                  } 

               } else {
                  depositionThickness = 1.0;
                  currentThickness = 1.0;
                  value = 1.0;
               } 

               erosionFactor->set ( i, j, value );
            } else {
               erosionFactor->set ( i, j, undefinedValue );
            }

         }

      }

      derivedProperties.push_back ( erosionFactor );
   }

}
