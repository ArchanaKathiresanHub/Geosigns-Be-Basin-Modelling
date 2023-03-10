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

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsFluidType.h"
#include "CompoundLithologyArray.h"
#include "GeoPhysicsProjectHandle.h"

#include "PropertyRetriever.h"
#include "ConstantsMathematics.h"
#include "VelocityFormationCalculator.h"

using namespace AbstractDerivedProperties;
using namespace std;

DerivedProperties::VelocityFormationCalculator::VelocityFormationCalculator () {
   addPropertyName ( "Velocity" );

   addDependentPropertyName ( "Porosity" );
   addDependentPropertyName ( "BulkDensity" );
   addDependentPropertyName ( "Pressure" );
   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "MaxVes" );
}

double DerivedProperties::VelocityFormationCalculator::calculateVelocity(const GeoPhysics::FluidType* const geophysicsFluid, const GeoPhysics::CompoundLithology* lithology,
                                                                         const double temperature, const double pressure, const double bulkDensity, const double porosity,
                                                                         const double ves, const double maxVes) const
{
  double seismicVelocityFluid;
  double densityFluid;
  if ( geophysicsFluid != nullptr ) {
     seismicVelocityFluid = geophysicsFluid->seismicVelocity(temperature, pressure);
     densityFluid = geophysicsFluid->density(temperature, pressure);
  } else {
     seismicVelocityFluid = -1;
     densityFluid = -1;
  }

  return lithology->seismicVelocity().calculate ( seismicVelocityFluid,
                                                  densityFluid,
                                                  bulkDensity,
                                                  Utilities::Maths::PercentageToFraction * porosity,
                                                  ves,
                                                  maxVes);
}

void DerivedProperties::VelocityFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   GeoPhysics::GeoPhysicsFormation const * const geophysicsFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   if( geophysicsFormation != 0 and geophysicsFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION ) {
      return calculateForBasement ( propertyManager, snapshot, formation, derivedProperties );
   }

   DataModel::AbstractProperty const * const porosityProperty    = propertyManager.getProperty( "Porosity" );
   DataModel::AbstractProperty const * const bulkDensityProperty = propertyManager.getProperty( "BulkDensity" );
   DataModel::AbstractProperty const * const pressureProperty    = propertyManager.getProperty( "Pressure" );
   DataModel::AbstractProperty const * const temperatureProperty = propertyManager.getProperty( "Temperature" );
   DataModel::AbstractProperty const * const vesProperty         = propertyManager.getProperty( "Ves" );
   DataModel::AbstractProperty const * const maxVesProperty      = propertyManager.getProperty( "MaxVes" );

   DataModel::AbstractProperty const * const velocityProperty = propertyManager.getProperty ( "Velocity" );

   const FormationPropertyPtr porosity    = propertyManager.getFormationProperty ( porosityProperty, snapshot, formation );
   const FormationPropertyPtr bulkDensity = propertyManager.getFormationProperty ( bulkDensityProperty, snapshot, formation );
   const FormationPropertyPtr pressure    = propertyManager.getFormationProperty ( pressureProperty, snapshot, formation );
   const FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );
   const FormationPropertyPtr ves         = propertyManager.getFormationProperty ( vesProperty, snapshot, formation );
   const FormationPropertyPtr maxVes      = propertyManager.getFormationProperty ( maxVesProperty, snapshot, formation );

   derivedProperties.clear ();

   if ( porosity != 0 and bulkDensity != 0 and pressure != 0 and temperature != 0 and ves!=0 and maxVes!=0 and geophysicsFormation != 0 ) {
      GeoPhysics::FluidType const * const geophysicsFluid = dynamic_cast<const GeoPhysics::FluidType*>( geophysicsFormation->getFluidType ());

      const GeoPhysics::CompoundLithologyArray& lithologies = geophysicsFormation->getCompoundLithologyArray ();

      PropertyRetriever porosityRetriever ( porosity );
      PropertyRetriever bulkDensityRetriever ( bulkDensity );
      PropertyRetriever pressureRetriever ( pressure );
      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever vesRetriever( ves );
      PropertyRetriever maxVesRetriever( maxVes );

      DerivedFormationPropertyPtr velocity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( velocityProperty,
                                                                                                                             snapshot,
                                                                                                                             formation,
                                                                                                                             propertyManager.getMapGrid (),
                                                                                                                             geophysicsFormation->getMaximumNumberOfElements() + 1 ));

      double currentTime = snapshot->getTime ();
      double undefinedValue = velocity->getUndefinedValue ();
      double velocityValue;
      double seismicVelocityFluid;
      double densityFluid;

      for ( unsigned int i = velocity->firstI ( true ); i <= velocity->lastI ( true ); ++i ) {

         for ( unsigned int j = velocity->firstJ ( true ); j <= velocity->lastJ ( true ); ++j ) {

            if ( geophysicsFormation->getProjectHandle().getNodeIsValid ( i, j )) {

               for ( unsigned int k = velocity->firstK (); k <= velocity->lastK (); ++k ) {


                  velocity->set ( i, j, k, calculateVelocity(geophysicsFluid, lithologies (i,j,currentTime), temperature->get(i,j,k),
                                                             pressure->get(i,j,k), bulkDensity->get(i,j,k), porosity->get(i,j,k),
                                                             ves->get(i,j,k), maxVes->get(i,j,k)));
               }

            } else {

               for ( unsigned int k = velocity->firstK (); k <= velocity->lastK (); ++k ) {
                  velocity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( velocity );
   }

}

void DerivedProperties::VelocityFormationCalculator::calculateForBasement (       AbstractPropertyManager&      propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const {

   GeoPhysics::GeoPhysicsFormation const * const geophysicsFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   DataModel::AbstractProperty const * const bulkDensityProperty = propertyManager.getProperty( "BulkDensity" );
   DataModel::AbstractProperty const * const temperatureProperty = propertyManager.getProperty( "Temperature" );

   DataModel::AbstractProperty const * const velocityProperty = propertyManager.getProperty ( "Velocity" );

   const FormationPropertyPtr bulkDensity = propertyManager.getFormationProperty ( bulkDensityProperty, snapshot, formation );
   const FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );

   derivedProperties.clear ();

   if ( bulkDensity != 0 and temperature != 0 and geophysicsFormation != 0 ) {

      const GeoPhysics::CompoundLithologyArray& lithologies = geophysicsFormation->getCompoundLithologyArray ();

      PropertyRetriever bulkDensityRetriever ( bulkDensity );
      PropertyRetriever temperatureRetriever ( temperature );

      DerivedFormationPropertyPtr velocity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( velocityProperty,
                                                                                                                             snapshot,
                                                                                                                             formation,
                                                                                                                             propertyManager.getMapGrid (),
                                                                                                                             geophysicsFormation->getMaximumNumberOfElements() + 1 ));

      double currentTime = snapshot->getTime ();
      double undefinedValue = velocity->getUndefinedValue ();
      double velocityValue;
      double seismciVelocityFluid;
      double densityFluid;

      for ( unsigned int i = velocity->firstI ( true ); i <= velocity->lastI ( true ); ++i ) {

         for ( unsigned int j = velocity->firstJ ( true ); j <= velocity->lastJ ( true ); ++j ) {

            if ( geophysicsFormation->getProjectHandle().getNodeIsValid ( i, j )) {

               for ( unsigned int k = velocity->firstK (); k <= velocity->lastK (); ++k ) {

                  seismciVelocityFluid = -1;
                  densityFluid = -1;

                  velocityValue = lithologies ( i, j, currentTime )->seismicVelocity().calculate ( seismciVelocityFluid,
                                                                                                   densityFluid,
                                                                                                   bulkDensity->get(i, j, k),
                                                                                                   0.0, 0.0, 0.0 );

                  velocity->set ( i, j, k, velocityValue );
               }

            } else {

               for ( unsigned int k = velocity->firstK (); k <= velocity->lastK (); ++k ) {
                  velocity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( velocity );
   }

}

bool DerivedProperties::VelocityFormationCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                    const DataModel::AbstractSnapshot*  snapshot,
                                                                    const DataModel::AbstractFormation* formation ) const {

   bool basementFormation = ( dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation ) != 0 and dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( basementFormation and ( dependentProperties [ i ] != "Temperature" and dependentProperties [ i ] != "BulkDensity" )) {
         propertyIsComputable = true;
      } else {
         const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

         if ( property == 0 ) {
            propertyIsComputable = false;
         } else {
            propertyIsComputable = propertyIsComputable and propManager.formationPropertyIsComputable ( property, snapshot, formation );
         }

      }
   }

   return propertyIsComputable;
}

double DerivedProperties::VelocityFormationCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                                                            const GeoPhysics::CompoundLithology* lithology,
                                                                            const std::map<string, double>& dependentProperties ) const
{
  const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

  return DerivedProperties::VelocityFormationCalculator::calculateVelocity(fluid, lithology,
                                                                           dependentProperties.at("Temperature"),
                                                                           dependentProperties.at("Pressure"),
                                                                           dependentProperties.at("BulkDensity"),
                                                                           dependentProperties.at("Porosity"),
                                                                           dependentProperties.at("Ves"),
                                                                           dependentProperties.at("MaxVes"));
}
