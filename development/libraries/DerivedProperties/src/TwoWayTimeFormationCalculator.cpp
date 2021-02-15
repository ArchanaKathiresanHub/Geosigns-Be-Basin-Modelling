//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "TwoWayTimeFormationCalculator.h"

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"
#include "PropertyRetriever.h"

// DataAccess library
#include "Surface.h"

// Geophysics library
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsProjectHandle.h"

// Utility library
#include "FormattingException.h"
#include "LogHandler.h"
#include "ConstantsMathematics.h"


using namespace AbstractDerivedProperties;

typedef formattingexception::GeneralException TwoWayTimeException;

DerivedProperties::TwoWayTimeFormationCalculator::TwoWayTimeFormationCalculator() {
   addPropertyName ( "TwoWayTime" );

   addDependentPropertyName ( "Depth" );
   addDependentPropertyName ( "Pressure" );
   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Velocity" );
}

void DerivedProperties::TwoWayTimeFormationCalculator::calculate(      AbstractPropertyManager&      propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   ///I. Get dependent properties
   DataModel::AbstractProperty const * const depthProperty         = propertyManager.getProperty ( "Depth" );
   DataModel::AbstractProperty const * const pressureProperty      = propertyManager.getProperty ( "Pressure" );
   DataModel::AbstractProperty const * const temperatureProperty   = propertyManager.getProperty ( "Temperature" );
   DataModel::AbstractProperty const * const velocityProperty      = propertyManager.getProperty ( "Velocity" );

   DataModel::AbstractProperty const * const twoWayTimeProperty = propertyManager.getProperty ( "TwoWayTime" );

   const FormationPropertyPtr   depth       = propertyManager.getFormationProperty ( depthProperty,       snapshot, formation );
   const FormationPropertyPtr   pressure    = propertyManager.getFormationProperty ( pressureProperty,    snapshot, formation );
   const FormationPropertyPtr   temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );
   const FormationPropertyPtr   velocity    = propertyManager.getFormationProperty ( velocityProperty,    snapshot, formation );

   ///II. Find the two-way-travel-time at the top of the current formation if there is a such formation
   GeoPhysics::GeoPhysicsFormation const * const geophysicsFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   const DataAccess::Interface::Surface* surfaceTop = geophysicsFormation->getTopSurface();
   const GeoPhysics::GeoPhysicsFormation* formationTop = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(surfaceTop->getTopFormation());
   const GeoPhysics::GeoPhysicsFormation* formationBottom = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(surfaceTop->getBottomFormation());
   FormationPropertyPtr twoWayTimeTop = 0;
   if (formationTop){
      DataModel::AbstractProperty const * const twoWayTimeTopProperty = propertyManager.getProperty( "TwoWayTime" );
      twoWayTimeTop = propertyManager.getFormationProperty( twoWayTimeTopProperty, snapshot, formationTop );
      if (!twoWayTimeTop){
         throw TwoWayTimeException() << "Cannot get the two-way-time of the formation above '" << formation->getName() << "'.";
      }
   }


   derivedProperties.clear ();

   ///III. Compute two-way-(travel)-time
   if (depth != 0 and pressure != 0 and temperature != 0 and velocity != 0 and geophysicsFormation != 0) {

      PropertyRetriever depthRetriever         ( depth );
      PropertyRetriever pressureRetriever      ( pressure );
      PropertyRetriever temperatureRetriever   ( temperature );
      PropertyRetriever velocityRetriever      ( velocity );

      DerivedFormationPropertyPtr twoWayTime = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( twoWayTimeProperty,
                                                                                                                             snapshot,
                                                                                                                             formation,
                                                                                                                             propertyManager.getMapGrid (),
                                                                                                                             geophysicsFormation->getMaximumNumberOfElements() + 1 ));

      double undefinedValue = twoWayTime->getUndefinedValue();
      double twoWayTimeValue;
      double distance;
      double seismicVelocityNumerator;
      double seismicVelocityDenominator;
      double seismicVelocityBulk;
      double seismicVelocityFluid;
      double seaBottomDepth;

      for (unsigned int i = twoWayTime->firstI( true ); i <= twoWayTime->lastI( true ); ++i) {

         for (unsigned int j = twoWayTime->firstJ( true ); j <= twoWayTime->lastJ( true ); ++j) {

            if (geophysicsFormation->getProjectHandle().getNodeIsValid( i, j )) {

               ///1.a If we are not at the top, add the twoWayTime from the node above (first bottom node of the formation above)
               if (twoWayTimeTop){
                  twoWayTimeValue = twoWayTimeTop->get( i, j, 0 );
               }
               ///1.b If we are at the top, check for water column TwoWayTime
               else{
                  seaBottomDepth = depth->get( i, j, depth->lastK() );
                  ///1.b.i If there is water above the first formation, then the initial TwoWayTime is the one from the water above
                  if (seaBottomDepth > 0) {
                     // If there is no sediment yet, there is no TwoWayTime value
                     if (formationBottom->isCrust()) {
                        twoWayTimeValue = undefinedValue;
                     }
                     // If there is some sediments , the TwoWayTime value is computed (in ms) using the fluid of the last sediment deposited
                     else {
                        const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formationBottom->getFluidType());
                        if (!fluid){
                           throw TwoWayTimeException() << "The model is under water but there is no fluid avalaible to compute the water column two-way-time";
                        }
                        seismicVelocityFluid = fluid->seismicVelocity( temperature->get( i, j, temperature->lastK() ), pressure->get( i, j, temperature->lastK() ) );
                        if (seismicVelocityFluid==0){
                           // In case of weird fluids which are blocking the seismic waves (Vp=0), this should not be allowed by the UI
                           twoWayTimeValue = undefinedValue;
                           LogHandler( LogHandler::WARNING_SEVERITY ) << "Dividing by a 0 fluid seismic velocity during two-way-travel-time computation."
                              << " Two-way-time is set to undefined value for node (" << i << "," << j << "," << 0 << ").";
                        }
                        twoWayTimeValue = Utilities::Maths::KilometerToMeter * 2 * seaBottomDepth / seismicVelocityFluid;
                     }
                  }
                  ///1.b.ii If no water above, then the initial TwoWayTime is 0
                  else {
                     twoWayTimeValue = 0;
                  }
               }
               twoWayTime->set( i, j, twoWayTime->lastK(), twoWayTimeValue );

               ///2. For all the nodes in the formation compute the twoWayTime
               for (int k = twoWayTime->lastK() - 1; k >= 0; --k) {
                  ///2.1 Compute the distance between the nodes
                  distance = depth->get( i, j, k ) - depth->get( i, j, k + 1 );
                  ///2.2 Compute the harmonic mean of the velocity between the nodes
                  seismicVelocityNumerator = 2 * velocity->get( i, j, k ) * velocity->get( i, j, k + 1 );
                  seismicVelocityDenominator = velocity->get( i, j, k ) + velocity->get( i, j, k + 1 );
                  if (seismicVelocityDenominator == 0) {
                     throw TwoWayTimeException() << "Dividing by zero during harmonic mean computation of the seismic velocity.";
                  }
                  seismicVelocityBulk = seismicVelocityNumerator / seismicVelocityDenominator;
                  ///2.3 Compute the twoWayTime between the nodes (in ms)
                  if (seismicVelocityBulk == 0) {
                     // In case of weird lithologies which are blocking the seismic waves (Vp=0), this should not be allowed by the UI
                     twoWayTimeValue = undefinedValue;
                     LogHandler( LogHandler::WARNING_SEVERITY ) << "Dividing by a 0 bulk seismic velocity during two-way-travel-time computation."
                        << " Two-way-time is set to undefined value for node (" << i << "," << j << "," << k << ").";
                  }
                  else{
                     twoWayTimeValue += Utilities::Maths::KilometerToMeter * 2 * distance / seismicVelocityBulk;
                  }
                  twoWayTime->set( i, j, k, twoWayTimeValue );
               } //for each k

            } // if valid node

            else {
               for (unsigned int k = velocity->firstK(); k <= velocity->lastK(); ++k) {
                  twoWayTime->set( i, j, k, undefinedValue );
               }
            } // if not valid node

         } // for each j

      } // for each i

      derivedProperties.push_back( twoWayTime );
   } // if properties and formation are found
   else{
      throw TwoWayTimeException() << "Cannot get one of the two-way-time dependent properties or formation.";
   }

}

bool DerivedProperties::TwoWayTimeFormationCalculator::isComputable( const AbstractPropertyManager&      propManager,
                                                                     const DataModel::AbstractSnapshot*  snapshot,
                                                                     const DataModel::AbstractFormation* formation ) const
{
   if (formation && snapshot->getTime() != 0.0)
   {
     return false;
   }

   return AbstractDerivedProperties::FormationPropertyCalculator::isComputable( propManager, snapshot, formation );
}
