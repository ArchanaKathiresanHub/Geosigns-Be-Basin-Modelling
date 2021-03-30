//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"
#include "PropertyRetriever.h"

// Geophysics library
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsProjectHandle.h"

// Utility library
#include "FormattingException.h"
#include "ConstantsMathematics.h"

#include "SonicFormationCalculator.h"

using namespace AbstractDerivedProperties;

DerivedProperties::SonicFormationCalculator::SonicFormationCalculator() {
   addPropertyName ( "SonicSlowness" );
   addDependentPropertyName ( "Velocity" );
}

double DerivedProperties::SonicFormationCalculator::calculateSonic(double velocity) const
{
  return Utilities::Maths::SecondToMicroSecond * (1.0 / velocity);
}

void DerivedProperties::SonicFormationCalculator::calculate(       AbstractPropertyManager&      propertyManager,
                                                             const DataModel::AbstractSnapshot*  snapshot,
                                                             const DataModel::AbstractFormation* formation,
                                                                   FormationPropertyList&        derivedProperties ) const {

   DataModel::AbstractProperty const * const velocityProperty = propertyManager.getProperty( "Velocity" );

   DataModel::AbstractProperty const * const sonicProperty = propertyManager.getProperty ( "SonicSlowness" );

   const FormationPropertyPtr velocity = propertyManager.getFormationProperty( velocityProperty, snapshot, formation );

   GeoPhysics::GeoPhysicsFormation const * const geophysicsFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(formation);

   derivedProperties.clear ();

   if (velocity != 0) {

      PropertyRetriever velocityRetriever( velocity );

      DerivedFormationPropertyPtr sonic = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( sonicProperty,
                                                                                                                        snapshot,
                                                                                                                        formation,
                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                        geophysicsFormation->getMaximumNumberOfElements() + 1 ));

      double undefinedValue = sonic->getUndefinedValue ();
      double sonicValue;

      for (unsigned int i = sonic->firstI( true ); i <= sonic->lastI( true ); ++i) {

         for (unsigned int j = sonic->firstJ( true ); j <= sonic->lastJ( true ); ++j) {

            if ( geophysicsFormation->getProjectHandle().getNodeIsValid ( i, j )) {

               for (unsigned int k = sonic->firstK(); k <= sonic->lastK(); ++k) {

                  sonicValue = calculateSonic(velocity->get(i,j,k));

                  sonic->set( i, j, k, sonicValue );
               }

            } else {

               for (unsigned int k = sonic->firstK(); k <= sonic->lastK(); ++k) {
                  sonic->set( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( sonic );
   } 
}

double DerivedProperties::SonicFormationCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* /*formation*/,
                                                                         const GeoPhysics::CompoundLithology* /*lithology*/,
                                                                         const std::map<string, double>& dependentProperties ) const
{
  return calculateSonic( dependentProperties.at( "Velocity" ) );
}
