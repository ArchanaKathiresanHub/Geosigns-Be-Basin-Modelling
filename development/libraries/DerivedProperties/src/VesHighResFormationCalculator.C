//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VesHighResFormationCalculator.h"

#include <assert.h>
#include "FormattingException.h"
#include "SimulationDetails.h"
#include "IndirectFormationProperty.h"
#include "CompoundLithology.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "Surface.h"
#include "Snapshot.h"

// utilities library
#include "ConstantsPhysics.h"

using namespace AbstractDerivedProperties;

DerivedProperties::VesHighResFormationCalculator::VesHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle ) :
   m_projectHandle( projectHandle ),
   m_isCoupledMode( false ),
   m_isSubsampled( ! ((*(m_projectHandle->getLowResolutionOutputGrid())) == (*(m_projectHandle->getHighResolutionOutputGrid()))) )
{
   try
   {
      addPropertyName( "VesHighRes" );

      const DataAccess::Interface::SimulationDetails * simulationDetails = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

      if( simulationDetails == 0 )
      {
         throw formattingexception::GeneralException() << "Last simulation is missing";
      }

      m_isCoupledMode = simulationDetails->getSimulatorMode () == "Overpressure" ||
                        simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" ||
                        simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" ||
                        simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" ||
                        simulationDetails->getSimulatorMode () == "CoupledDarcy";

      if( !m_isSubsampled || m_isCoupledMode )
      {
         addDependentPropertyName( "Ves" );
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}

void DerivedProperties::VesHighResFormationCalculator::calculate(       AbstractPropertyManager &      propertyManager,
                                                                  const DataModel::AbstractSnapshot *  snapshot,
                                                                  const DataModel::AbstractFormation * formation,
                                                                        FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const GeoPhysics::GeoPhysicsFormation * const currentFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation *>( formation );
      assert( currentFormation != 0 );

      if( currentFormation->getBottomSurface()->getSnapshot()->getTime() <= snapshot->getTime() ||
          currentFormation->kind() == DataAccess::Interface::BASEMENT_FORMATION )
      {
         // If the formation is the basement or at the provided snapshot the current formation has't
         // deposited yet or is just about to deposit we return an empty list of derived properties
         derivedProperties.clear();
      }
      else if( !m_isSubsampled || m_isCoupledMode )
      {
         computeIndirectly( propertyManager,
                            snapshot,
                            formation,
                            derivedProperties );
      }
      else
      {
         computeForSubsampledHydroRun( propertyManager,
                                       snapshot,
                                       formation,
                                       derivedProperties );
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::VesHighResFormationCalculator::computeIndirectly(       AbstractPropertyManager &      propertyManager,
                                                                          const DataModel::AbstractSnapshot *  snapshot,
                                                                          const DataModel::AbstractFormation * formation,
                                                                                FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const DataModel::AbstractProperty * vesHighResProperty = propertyManager.getProperty( getPropertyNames()[ 0 ] );
      assert( vesHighResProperty != 0 );
      
      const DataModel::AbstractProperty* vesProperty = propertyManager.getProperty( "Ves" );
      assert( vesProperty != 0 );
      FormationPropertyPtr ves = propertyManager.getFormationProperty( vesProperty, snapshot, formation );
      assert( ves != 0 );
      
      IndirectFormationPropertyPtr vesHighRes = IndirectFormationPropertyPtr( new DerivedProperties::IndirectFormationProperty( vesHighResProperty, ves) );
      derivedProperties.clear();
      derivedProperties.push_back( vesHighRes );
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::VesHighResFormationCalculator::computeForSubsampledHydroRun(       AbstractPropertyManager &      propertyManager,
                                                                                     const DataModel::AbstractSnapshot *  snapshot,
                                                                                     const DataModel::AbstractFormation * formation,
                                                                                           FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const GeoPhysics::GeoPhysicsFormation* currentFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

      const DataModel::AbstractProperty * vesHighResProperty = propertyManager.getProperty( getPropertyNames()[ 0 ] );
      assert( vesHighResProperty != 0 );

      DerivedFormationPropertyPtr vesHighRes = 
         DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( vesHighResProperty,
                                                                                       snapshot,
                                                                                       formation,
                                                                                       propertyManager.getMapGrid(),
                                                                                       currentFormation->getMaximumNumberOfElements() + 1 ) );

      const double time = snapshot->getTime();
      const GeoPhysics::GeoPhysicsFormation * formationAbove = 0;
      if( currentFormation->getTopSurface()->getSnapshot() == 0 ||
          currentFormation->getTopSurface()->getSnapshot()->getTime() > time )
      {
         formationAbove = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( currentFormation->getTopSurface()->getTopFormation());
      }

      // Initialize top surface nodes according to the formation above (if any)
      initializeTopSurface( propertyManager,
                            vesHighResProperty,
                            snapshot,
                            formationAbove,
                            vesHighRes );

      // Now that the top nodes of the property has been initialised 
      // the vesHighRes for the remaining nodes below them can be computed.
      const bool includeGhostNodes = true;
      const unsigned int firstI = vesHighRes->firstI( includeGhostNodes );
      const unsigned int lastI = vesHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = vesHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ = vesHighRes->lastJ( includeGhostNodes );
      const unsigned int firstK = vesHighRes->firstK();
      const unsigned int lastK = vesHighRes->lastK();
      
      double vesHighResValue = 0.0;
      double vesHighResAboveValue = 0.0;
      double densityDifference = 0.0;
      double solidThickness = 0.0;
      const GeoPhysics::FluidType * fluid = dynamic_cast<const GeoPhysics::FluidType*>(currentFormation->getFluidType());
      const double constFluidDensity = (fluid == 0) ? 0.0 : fluid->getConstantDensity();
      const bool switchPermafrost = ( fluid == 0 ) ? false : fluid->isPermafrostEnabled();

      for( unsigned int i = firstI; i <= lastI; ++i )
      {
         for( unsigned int j = firstJ; j <= lastJ; ++j )
         {
            if( m_projectHandle->getNodeIsValid(i, j) )
            {
               const GeoPhysics::CompoundLithology * lithology = currentFormation->getCompoundLithology(i, j);
               double surfacePorosity = lithology->surfacePorosity();
               assert( lithology != 0 );

               // the density difference cannot be smaller than 0
               if ( lithology->density() > constFluidDensity ) 
               {
                  densityDifference = lithology->density() - constFluidDensity;
               }
               else 
               {
                  densityDifference = 0.0;
               }
               vesHighResAboveValue = vesHighRes->get(i, j, lastK);

               // Loop index is shifted up by 1.
               for( unsigned int k = lastK; k > firstK; --k )
               {
                  // index k     is top node of segment
                  // index k - 1 is bottom node of segment
                  solidThickness = currentFormation->getSolidThickness( i, j, k - 1, time);
                  
                  // Fluid is denser than rock and the permafrost switch is on
                  if ( densityDifference <= 0 && solidThickness > 0.0 && switchPermafrost )
                  {
                     // mobile layer cannot compress, but permafrost lithology has certain porosity that must be accounted in the calculation of ves
                     vesHighResValue = vesHighResAboveValue + Utilities::Physics::AccelerationDueToGravity * lithology->density( ) * solidThickness * ( 1.0 - surfacePorosity);
                  }
                  else if( solidThickness > 0.0 )
                  {
                     vesHighResValue = vesHighResAboveValue + Utilities::Physics::AccelerationDueToGravity * densityDifference * solidThickness;
                  }
                  else
                  {
                     vesHighResValue = vesHighResAboveValue;
                  }
                  vesHighRes->set(i, j, k - 1, vesHighResValue);
                  vesHighResAboveValue = vesHighResValue;
               }
            }
            else
            {
               for( unsigned int k = firstK; k <= lastK; ++k )
               {
                  vesHighRes->set(i, j, k, DataAccess::Interface::DefaultUndefinedMapValue);
               }
            }
         }
      }

      derivedProperties.clear();
      derivedProperties.push_back(vesHighRes);

   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::VesHighResFormationCalculator::initializeTopSurface(       AbstractPropertyManager &      propertyManager,
                                                                             const DataModel::AbstractProperty *  vesHighResProperty,
                                                                             const DataModel::AbstractSnapshot *  snapshot,
                                                                             const DataModel::AbstractFormation * formationAbove,
                                                                                   DerivedFormationPropertyPtr &  vesHighRes ) const
{
   try
   {
      const bool includeGhostNodes = true;
      const unsigned int firstI = vesHighRes->firstI( includeGhostNodes );
      const unsigned int lastI = vesHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = vesHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ = vesHighRes->lastJ( includeGhostNodes );
      const unsigned int topNodeIndex = vesHighRes->lastK();

      if( formationAbove == 0 )
      {
         // No formation above is available. vesHighRes must be 0 or undefined for invalid nodes
         double propertyNodeValue = 0.0;
         for( unsigned int i = firstI; i <= lastI; ++i )
         {
            for( unsigned int j = firstJ; j <= lastJ; ++j )
            {
               propertyNodeValue = m_projectHandle->getNodeIsValid(i, j) ? 0.0 : DataAccess::Interface::DefaultUndefinedMapValue;
               vesHighRes->set( i, j, topNodeIndex, propertyNodeValue );
            }
         }
      }
      else
      {
         // Available formation above. vesHighRes retrieved from there
         // No check on node validity is required because it has been already done on the above formation
         const FormationPropertyPtr vesHighResAbove = propertyManager.getFormationProperty( vesHighResProperty,
                                                                                            snapshot,
                                                                                            formationAbove );

         for( unsigned int i = firstI; i <= lastI; ++i )
         {
            for( unsigned int j = firstJ; j <= lastJ; ++j )
            {
               vesHighRes->set( i, j, topNodeIndex, vesHighResAbove->get( i, j, 0 ) );
            }
         }
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}
