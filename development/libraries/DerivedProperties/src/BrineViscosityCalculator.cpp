//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineViscosityCalculator.h"

#include "RunParameters.h"
#include "Surface.h"
#include "Snapshot.h"
#include "SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"

#include "DerivedFormationProperty.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::BrineViscosityCalculator::BrineViscosityCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) :
   m_projectHandle ( projectHandle )
{
   addPropertyName ( "BrineViscosity" );
   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Pressure" );
}


double DerivedProperties::BrineViscosityCalculator::calculateBrineViscosity(const GeoPhysics::FluidType* fluid, double temperature, double pressure) const
{
  return fluid->viscosity ( temperature, pressure);
}

void DerivedProperties::BrineViscosityCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                              const DataModel::AbstractSnapshot*  snapshot,
                                                              const DataModel::AbstractFormation* formation,
                                                                    FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::GeoPhysicsFormation* currentFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(currentFormation->getFluidType ());

   derivedProperties.clear();

   if( fluid == 0 ) {
      return;
   }

   const DataModel::AbstractProperty* brineViscosityProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr brineViscosity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( brineViscosityProperty, snapshot, formation,
                                                                                                                              propertyManager.getMapGrid (),
                                                                                                                              currentFormation->getMaximumNumberOfElements() + 1 ));

   const DataModel::AbstractProperty* temperatureProperty = propertyManager.getProperty ( "Temperature" );
   FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );

   PropertyRetriever temperatureRetriever ( temperature );

   const DataModel::AbstractProperty* pressureProperty = propertyManager.getProperty ( "Pressure" );
   FormationPropertyPtr pressure = propertyManager.getFormationProperty ( pressureProperty, snapshot, formation );

   PropertyRetriever pressureRetriever ( pressure );

   const double undefinedValue = brineViscosity->getUndefinedValue ();

   for ( unsigned int i = brineViscosity->firstI ( true ); i <= brineViscosity->lastI ( true ); ++i ) {

      for ( unsigned int j = brineViscosity->firstJ ( true ); j <= brineViscosity->lastJ ( true ); ++j ) {
         if ( m_projectHandle.getNodeIsValid ( i, j )) {

            for ( unsigned int k = brineViscosity->firstK (); k <= brineViscosity->lastK (); ++k ) {
               brineViscosity->set ( i, j, k, calculateBrineViscosity(fluid, temperature->get(i,j,k), pressure->get(i,j,k)));
            }
         } else {
            for ( unsigned int k = brineViscosity->firstK (); k <= brineViscosity->lastK (); ++k ) {
               brineViscosity->set ( i, j, k, undefinedValue );
            }

         }

      }

   }

   derivedProperties.push_back ( brineViscosity );
}

double DerivedProperties::BrineViscosityCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                                                         const GeoPhysics::CompoundLithology* /*lithology*/,
                                                                         const std::map<std::string, double>& dependentProperties ) const
{
  const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>( formation->getFluidType() );
  if ( !fluid )
  {
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }

  return calculateBrineViscosity( fluid, dependentProperties.at("Temperature"), dependentProperties.at("Pressure") );
}
