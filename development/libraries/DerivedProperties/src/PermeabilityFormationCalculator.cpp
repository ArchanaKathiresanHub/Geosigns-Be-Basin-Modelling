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

#include "RunParameters.h"
#include "SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "PermeabilityFormationCalculator.h"
#include "PropertyRetriever.h"

// utilities library
#include "ConstantsMathematics.h"

using namespace std;

using namespace AbstractDerivedProperties;

DerivedProperties::PermeabilityFormationCalculator::PermeabilityFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) : m_projectHandle ( projectHandle ) {

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle.getDetailsOfLastFastcauldron();

   m_chemicalCompactionRequired = lastFastcauldronRun != 0 and lastFastcauldronRun->getSimulatorMode () != "HydrostaticDecompaction" and
      m_projectHandle.getRunParameters()->getChemicalCompaction ();

   addPropertyName ( "Permeability" );
   addPropertyName ( "HorizontalPermeability" );

   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "MaxVes" );

   if ( m_chemicalCompactionRequired ) {
      addDependentPropertyName ( "ChemicalCompaction" );
   }

}

double DerivedProperties::PermeabilityFormationCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                                                                const GeoPhysics::CompoundLithology* lithology,
                                                                                const std::map<string, double>& dependentProperties ) const
{
  double permNorm;
  double permPlane;
  GeoPhysics::CompoundProperty porosity;
  bool chemicalCompactionRequired = m_chemicalCompactionRequired and formation->hasChemicalCompaction ();
  const double chemicalCompactionValue = ( chemicalCompactionRequired ? dependentProperties.at("ChemicalCompaction") : 0.0 );

  calculatePermeability(lithology,
                        dependentProperties.at("Ves"),
                        dependentProperties.at("MaxVes"),
                        chemicalCompactionRequired, chemicalCompactionValue,
                        permNorm, permPlane, porosity);
  return permPlane;
}

void DerivedProperties::PermeabilityFormationCalculator::calculatePermeability(const GeoPhysics::CompoundLithology* lithology, double ves, double maxVes,
                                                                               bool chemicalCompactionRequired, double chemicalCompactionValue,
                                                                               double& permNorm, double& permPlane, GeoPhysics::CompoundProperty& porosity) const
{
  lithology->getPorosity ( ves, maxVes, chemicalCompactionRequired, chemicalCompactionValue, porosity );
  lithology->calcBulkPermeabilityNP ( ves, maxVes, porosity, permNorm, permPlane );
  permNorm  *= Utilities::Maths::M2ToMilliDarcy;
  permPlane *= Utilities::Maths::M2ToMilliDarcy;
}

void DerivedProperties::PermeabilityFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                     const DataModel::AbstractSnapshot*  snapshot,
                                                                     const DataModel::AbstractFormation* formation,
                                                                           FormationPropertyList&        derivedProperties ) const {


   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* aPermeabilityVProperty = propertyManager.getProperty ( "Permeability" );
   const DataModel::AbstractProperty* aPermeabilityHProperty = propertyManager.getProperty ( "HorizontalPermeability" );

   const FormationPropertyPtr ves    = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr maxVes = propertyManager.getFormationProperty ( aMaxVesProperty, snapshot, formation );

   const GeoPhysics::GeoPhysicsFormation* geoFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   PropertyRetriever vesRetriever ( ves );
   PropertyRetriever maxVesRetriever ( maxVes );

   derivedProperties.clear ();

   if( ves != 0 and maxVes != 0 and geoFormation != 0 ) {

      bool chemicalCompactionRequired = m_chemicalCompactionRequired and geoFormation->hasChemicalCompaction ();

      FormationPropertyPtr chemicalCompaction;

      if ( chemicalCompactionRequired ) {
         chemicalCompaction = propertyManager.getFormationProperty ( aChemicalCompactionProperty, snapshot, formation );
         // Just in case the property is not found.
         chemicalCompactionRequired = chemicalCompaction != 0;
      }
      PropertyRetriever chemicalCompactionRetriever;
      if ( chemicalCompactionRequired ) {
         chemicalCompactionRetriever.reset( chemicalCompaction );
      }

      const GeoPhysics::CompoundLithologyArray& lithologies = geoFormation->getCompoundLithologyArray ();

      DerivedFormationPropertyPtr verticalPermeability =
         DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPermeabilityVProperty, snapshot, formation,
                                                                                         propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));
      DerivedFormationPropertyPtr horizontalPermeability =
         DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPermeabilityHProperty, snapshot, formation,
                                                                                         propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));
      const double undefinedValue = ves->getUndefinedValue ();
      const double currentAge = snapshot->getTime();

      double chemicalCompactionValue, permNorm, permPlane;
      GeoPhysics::CompoundProperty porosity;

      for ( unsigned int i = verticalPermeability->firstI ( true ); i <= verticalPermeability->lastI ( true ); ++i ) {

         for ( unsigned int j = verticalPermeability->firstJ ( true ); j <= verticalPermeability->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {

               for ( unsigned int k = verticalPermeability->firstK (); k <= verticalPermeability->lastK (); ++k ) {
                  chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->get ( i, j, k ) : 0.0 );

                  calculatePermeability(lithologies( i, j, currentAge ), ves->get ( i, j, k ), maxVes->get ( i, j, k ), chemicalCompactionRequired,
                                        chemicalCompactionValue, permNorm, permPlane, porosity);
                  verticalPermeability->set ( i, j, k, permNorm );
                  horizontalPermeability->set ( i, j, k, permPlane );

               }
            } else {
               for ( unsigned int k = verticalPermeability->firstK (); k <= verticalPermeability->lastK (); ++k ) {
                  verticalPermeability->set ( i, j, k, undefinedValue );
                  horizontalPermeability->set ( i, j, k, undefinedValue );
               }
            }
         }
      }

      derivedProperties.push_back ( verticalPermeability );
      derivedProperties.push_back ( horizontalPermeability );
   }
}


