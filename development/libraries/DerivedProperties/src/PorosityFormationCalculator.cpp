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
#include "CompoundLithologyArray.h"

#include "PropertyRetriever.h"
#include "PorosityFormationCalculator.h"

#include "ConstantsMathematics.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PorosityFormationCalculator::PorosityFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "Porosity" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle.getDetailsOfLastFastcauldron();

   // It could be that a particular formation does not have chemical-compaction
   // enabled butit is not possible to determine this here.
   m_chemicalCompactionRequired = lastFastcauldronRun != 0 and lastFastcauldronRun->getSimulatorMode () != "HydrostaticDecompaction" and
      m_projectHandle.getRunParameters()->getChemicalCompaction ();

   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "MaxVes" );

   if ( m_chemicalCompactionRequired ) {
      addDependentPropertyName ( "ChemicalCompaction" );
   }

}

double DerivedProperties::PorosityFormationCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                                                            const GeoPhysics::CompoundLithology* lithology,
                                                                            const std::map<string, double>& dependentProperties ) const
{
  bool chemicalCompactionRequired = m_chemicalCompactionRequired and formation->hasChemicalCompaction ();
  const double chemicalCompactionValue = ( chemicalCompactionRequired ? dependentProperties.at("ChemicalCompaction") : 0.0 );

  return calculatePorosity(lithology,
                           dependentProperties.at("Ves"),
                           dependentProperties.at("MaxVes"),
                           chemicalCompactionRequired,
                           chemicalCompactionValue);
}

double DerivedProperties::PorosityFormationCalculator::calculatePorosity(const GeoPhysics::CompoundLithology* lithology, double ves, double maxVes, bool chemicalCompactionRequired, double chemicalCompactionValue) const
{
  return Utilities::Maths::FractionToPercentage * lithology->porosity ( ves, maxVes, chemicalCompactionRequired, chemicalCompactionValue );
}

void DerivedProperties::PorosityFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   const std::string fname = formation->getName();

   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* aPorosityProperty = propertyManager.getProperty ( "Porosity" );

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


      const GeoPhysics::CompoundLithologyArray& lithologies = geoFormation->getCompoundLithologyArray ();

      DerivedFormationPropertyPtr porosityProp =
          DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPorosityProperty, snapshot, formation,
                                                                                          propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever chemicalCompactionRetriever;
      if( chemicalCompactionRequired ) {
         chemicalCompactionRetriever.reset( chemicalCompaction );
      }

      const double undefinedValue = ves->getUndefinedValue ();
      const double currentTime = snapshot->getTime();

      for ( unsigned int i = porosityProp->firstI ( true ); i <= porosityProp->lastI ( true ); ++i ) {

         for ( unsigned int j = porosityProp->firstJ ( true ); j <= porosityProp->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {

               for ( unsigned int k = porosityProp->firstK (); k <= porosityProp->lastK (); ++k ) {
                  const double chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->get ( i, j, k ) : 0.0 );
                  const double value = calculatePorosity(lithologies( i, j, currentTime ), ves->get(i,j,k), maxVes->get(i,j,k), chemicalCompactionRequired, chemicalCompactionValue);
                  porosityProp->set ( i, j, k, value );
               }
            } else {
               for ( unsigned int k = porosityProp->firstK (); k <= porosityProp->lastK (); ++k ) {
                  porosityProp->set ( i, j, k, undefinedValue );
               }
            }
         }
      }
      derivedProperties.push_back ( porosityProp );
   }
}
