// 
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "GammaRayFormationCalculator.h"

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "PropertyRetriever.h"

// Utility library
#include "FormattingException.h"
#include "LogHandler.h"
#include "ConstantsMathematics.h"

#include "CompoundLithology.h"
#include "GeoPhysicsFormation.h"

using namespace AbstractDerivedProperties;

DerivedProperties::GammaRayFormationCalculator::GammaRayFormationCalculator( ) {
   addPropertyName ( "GammaRay" );

   addDependentPropertyName ( "Porosity" );
}

double DerivedProperties::GammaRayFormationCalculator::calculateGammaRay(const double porosity, const double solidRadiogenicHeatProduction) const
{
  const double actualPorosity = Utilities::Maths::PercentageToFraction * porosity;
  const double bulkRadiogenicHeatProduction = ((1.0 - actualPorosity) * solidRadiogenicHeatProduction);
  return (bulkRadiogenicHeatProduction / m_gammaRayScaleFactor) + m_gammaRayOffset;
}

void DerivedProperties::GammaRayFormationCalculator::calculate(        AbstractPropertyManager&      propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   ///I. Get dependent properties
   DataModel::AbstractProperty const * const porosityProperty = propertyManager.getProperty ( "Porosity" );

   DataModel::AbstractProperty const * const gammaRayProperty = propertyManager.getProperty ( "GammaRay" );
 
   const FormationPropertyPtr porosity = propertyManager.getFormationProperty ( porosityProperty, snapshot, formation );

   derivedProperties.clear ();

   ///II. Compute the gamma ray value
   if ( porosity != nullptr and formation != nullptr) {
      const DataModel::AbstractCompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
      const double currentTime = snapshot->getTime ();

      PropertyRetriever porosityRetriever( porosity );
      DerivedFormationPropertyPtr gammaRay = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( gammaRayProperty,
                                                                                                                           snapshot,
                                                                                                                           formation, 
                                                                                                                           propertyManager.getMapGrid (),
                                                                                                                           formation->getMaximumNumberOfElements() + 1 ));


      for (unsigned int i = gammaRay->firstI( true ); i <= gammaRay->lastI( true ); ++i) {

         for (unsigned int j = gammaRay->firstJ( true ); j <= gammaRay->lastJ( true ); ++j) {

            if (propertyManager.getNodeIsValid( i, j )) {
               const double solidRadiogenicHeatProduction = lithologies ( i, j, currentTime )-> heatproduction ();
               for (int k = gammaRay->firstK(); k <= gammaRay->lastK(); ++k) {

                  gammaRay->set( i, j, k, calculateGammaRay( porosity->get( i, j, k ), solidRadiogenicHeatProduction ) );
               }
            } // if valid node
            else {
               for (unsigned int k = gammaRay->firstK(); k <= gammaRay->lastK(); ++k) {
                  const double undefinedValue = gammaRay->getUndefinedValue();
                  gammaRay->set( i, j, k, undefinedValue );
               }
            } // if not valid node

         } // for each j

      } // for each i

      derivedProperties.push_back( gammaRay );
   }
}

bool DerivedProperties::GammaRayFormationCalculator::isComputable( const AbstractPropertyManager&      propManager,
                                                                   const DataModel::AbstractSnapshot*  snapshot,
                                                                   const DataModel::AbstractFormation* formation ) const {

   DataModel::AbstractProperty const * const porosityProperty = propManager.getProperty( "Porosity" );
   return  propManager.formationPropertyIsComputable( porosityProperty, snapshot, formation );
}

double DerivedProperties::GammaRayFormationCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* /*formation*/,
                                                                            const GeoPhysics::CompoundLithology* lithology,
                                                                            const std::map<std::string, double>& dependentProperties) const
{
  const double solidRadiogenicHeatProduction = lithology->heatproduction();
  return calculateGammaRay(dependentProperties.at("Porosity"), solidRadiogenicHeatProduction);
}
