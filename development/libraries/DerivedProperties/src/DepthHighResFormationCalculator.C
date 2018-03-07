//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DepthHighResFormationCalculator.h"

#include <assert.h>
#include "CompoundLithology.h"
#include "FormattingException.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsCrustFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsMantleFormation.h"
#include "IndirectFormationProperty.h"
#include "Interface/MantleFormation.h"
#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "NumericFunctions.h"

using namespace AbstractDerivedProperties;

DerivedProperties::DepthHighResFormationCalculator::DepthHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle ) :
   m_projectHandle( projectHandle ),
   m_isCoupledMode( false ),
   m_isSubsampled( ! ((*(m_projectHandle->getLowResolutionOutputGrid())) == (*(m_projectHandle->getHighResolutionOutputGrid()))) ),
   m_isNonGeometricLoopActive( m_projectHandle->getRunParameters()->getNonGeometricLoop() )
{
   try
   {
      addPropertyName( "DepthHighRes" );

      const DataAccess::Interface::SimulationDetails * simulationDetails = m_projectHandle->getDetailsOfLastFastcauldron ();

      if( simulationDetails == 0 )
      {
         throw formattingexception::GeneralException() << "Last simulation is missing";
      }

      m_isCoupledMode = simulationDetails->getSimulatorMode () == "Overpressure" ||
                        simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" ||
                        simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" ||
                        simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" ||
                        simulationDetails->getSimulatorMode () == "CoupledDarcy";

      if( !m_isSubsampled )
      {
         addDependentPropertyName( "Depth" );
      }
      else if( !(m_isCoupledMode && m_isNonGeometricLoopActive) )
      {
         addDependentPropertyName( "VesHighRes" );
         addDependentPropertyName( "MaxVesHighRes" );
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}

void DerivedProperties::DepthHighResFormationCalculator::calculate(       AbstractPropertyManager &      propertyManager,
                                                                    const DataModel::AbstractSnapshot *  snapshot,
                                                                    const DataModel::AbstractFormation * formation,
                                                                          FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const GeoPhysics::Formation * const currentFormation = dynamic_cast<const GeoPhysics::Formation *>( formation );
      assert( currentFormation != 0 );

      if( currentFormation->getBottomSurface()->getSnapshot()->getTime() <= snapshot->getTime() )
      {
         // If at the provided snapshot the current formation has't deposited yet
         // or is just about to deposit we return an empty list of derived properties
         derivedProperties.clear();
      }
      else if( !m_isSubsampled )
      {
         computeIndirectly( propertyManager,
                            snapshot,
                            formation,
                            derivedProperties );
      }
      else
      {
         const DataModel::AbstractProperty * depthHighResProperty = propertyManager.getProperty( getPropertyNames()[ 0 ] );
         assert( depthHighResProperty != 0 );

         DerivedFormationPropertyPtr depthHighRes = 
            DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( depthHighResProperty,
                                                                                          snapshot,
                                                                                          formation,
                                                                                          propertyManager.getMapGrid(),
                                                                                          currentFormation->getMaximumNumberOfElements() + 1 ) );
      
         // Initialize top surface nodes according to the formation above (if any)
         initializeTopSurface( propertyManager,
                               depthHighResProperty,
                               snapshot,
                               currentFormation,
                               depthHighRes );

         if( currentFormation->isMantle() )
         {
            computeForMantle( snapshot, depthHighRes );
         }
         else if( m_isCoupledMode && m_isNonGeometricLoopActive )
         {
            computeForCoupledRunWithNonGeometricLoop( currentFormation,
                                                      snapshot,
                                                      depthHighRes,
                                                      derivedProperties );
         }
         else
         {
            computeForSubsampledRun( propertyManager,
                                     currentFormation,
                                     snapshot,
                                     depthHighRes,
                                     derivedProperties );
         }
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::DepthHighResFormationCalculator::computeIndirectly(       AbstractPropertyManager &      propertyManager,
                                                                            const DataModel::AbstractSnapshot *  snapshot,
                                                                            const DataModel::AbstractFormation * formation,
                                                                                  FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const DataModel::AbstractProperty * const depthHighResProperty = propertyManager.getProperty( getPropertyNames()[ 0 ] );

      const DataModel::AbstractProperty * const depthProperty = propertyManager.getProperty( "Depth" );
      assert( depthProperty != 0 );
      FormationPropertyPtr depth = propertyManager.getFormationProperty( depthProperty, snapshot, formation );

      derivedProperties.clear();

      // Excluding the mantle
      if( depth != 0 )
      {
         IndirectFormationPropertyPtr depthHighRes = IndirectFormationPropertyPtr( new DerivedProperties::IndirectFormationProperty( depthHighResProperty, depth) );
         derivedProperties.push_back( depthHighRes );
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::DepthHighResFormationCalculator::initializeTopSurface(       AbstractPropertyManager &      propertyManager,
                                                                               const DataModel::AbstractProperty *  depthHighResProperty,
                                                                               const DataModel::AbstractSnapshot *  snapshot,
                                                                               const GeoPhysics::Formation *        currentFormation,
                                                                                     DerivedFormationPropertyPtr &  depthHighRes ) const
{
   try
   {
      const GeoPhysics::Formation * formationAbove = 0;
      if( currentFormation->getTopSurface()->getSnapshot() == 0 ||
          currentFormation->getTopSurface()->getSnapshot()->getTime() > snapshot->getTime() )
      {
         formationAbove = dynamic_cast<const GeoPhysics::Formation*>( currentFormation->getTopSurface()->getTopFormation() );
      }

      const bool includeGhostNodes = true;
      const unsigned int firstI = depthHighRes->firstI( includeGhostNodes );
      const unsigned int lastI  = depthHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = depthHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ  = depthHighRes->lastJ( includeGhostNodes );
      const unsigned int topNodeIndex = depthHighRes->lastK();

      const double time = snapshot->getTime();

      if( formationAbove == 0 )
      {
         // No formation above is available. depthHighRes must be 0 or undefined for invalid nodes
         double propertyNodeValue = 0.0;
         for( unsigned int i = firstI; i <= lastI; ++i )
         {
            for( unsigned int j = firstJ; j <= lastJ; ++j )
            {
               propertyNodeValue = m_projectHandle->getNodeIsValid(i, j) ? m_projectHandle->getSeaBottomDepth(i, j, time) : DataAccess::Interface::DefaultUndefinedMapValue;
               depthHighRes->set( i, j, topNodeIndex, propertyNodeValue );
            }
         }
      }
      else
      {
         // Available formation above. depthHighRes retrieved from there
         // No check on node validity is required because it has been already done on the above formation
         const FormationPropertyPtr depthHighResAbove = propertyManager.getFormationProperty( depthHighResProperty,
                                                                                              snapshot,
                                                                                              formationAbove );
         assert( depthHighResAbove != 0 );

         for( unsigned int i = firstI; i <= lastI; ++i )
         {
            for( unsigned int j = firstJ; j <= lastJ; ++j )
            {
               depthHighRes->set( i, j, topNodeIndex, depthHighResAbove->get( i, j, 0 ) );
            }
         }
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::DepthHighResFormationCalculator::computeForMantle( const DataModel::AbstractSnapshot * snapshot,
                                                                                 DerivedFormationPropertyPtr & depthHighRes ) const
{
   try
   {
      const double time = snapshot->getTime();

      const bool isALC = (m_projectHandle->getBottomBoundaryConditions() == DataAccess::Interface::ADVANCED_LITHOSPHERE_CALCULATOR);
       
      double basementThickness = 0.0;
      double mantleThickness   = 0.0;;
      if( isALC )
      {
         basementThickness = m_projectHandle->getMantleFormation()->getInitialLithosphericMantleThickness()
                           + m_projectHandle->getCrustFormation()->getInitialCrustalThickness();
      }
      else
      {
         mantleThickness   = m_projectHandle->getMantleFormation()->getPresentDayThickness();
      }

      const GeoPhysics::GeoPhysicsCrustFormation * crust = dynamic_cast<const GeoPhysics::GeoPhysicsCrustFormation*>( m_projectHandle->getCrustFormation() );
      assert( crust != 0 );

      const GeoPhysics::GeoPhysicsMantleFormation * mantle = 0;
      if( isALC )
      {
         mantle = dynamic_cast<const GeoPhysics::GeoPhysicsMantleFormation*>( m_projectHandle->getMantleFormation() );
         assert( mantle != 0 );
      }

      const bool includeGhostNodes = true;
      const unsigned int firstI = depthHighRes->firstI( includeGhostNodes );
      const unsigned int lastI  = depthHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = depthHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ  = depthHighRes->lastJ( includeGhostNodes );
      const unsigned int firstK = depthHighRes->firstK();
      const unsigned int lastK  = depthHighRes->lastK();

      for( unsigned int i = firstI; i <= lastI; ++i )
      {
         for( unsigned int j = firstJ; j <= lastJ; ++j )
         {
            if( m_projectHandle->getNodeIsValid(i, j) )
            {
               const double currentCrustThickness = m_projectHandle->getCrustThickness( i, j, time );
               const double crustThinningRatio    = ( currentCrustThickness > 0.0 ) ? (crust->getCrustMaximumThickness(i,j)/currentCrustThickness) : -1.0;

               double mantleSegmentHeight = 0.0;
               if( isALC )
               {
                  if( currentCrustThickness <= 0.0 )
                  {
                     throw formattingexception::GeneralException() << "Effective Crustal thickness can't be 0";
                  }
                  mantleSegmentHeight = mantle->m_mantleElementHeight0 / crustThinningRatio;
               }
               else
               {
                  basementThickness = mantleThickness + m_projectHandle->getCrustThickness( i, j, 0.0 );
                  mantleSegmentHeight = m_projectHandle->getRunParameters()->getBrickHeightMantle() / crustThinningRatio;
               }

               const double currentMantleThickness  = basementThickness - currentCrustThickness;
               const double maximumDepth            = depthHighRes->get( i, j, lastK ) + currentMantleThickness;
               for( unsigned int k = lastK; k > firstK; --k )
               {
                  depthHighRes->set( i, j, k - 1, NumericFunctions::Minimum(maximumDepth, depthHighRes->get( i, j, k ) + mantleSegmentHeight) );
               }
            }
            else
            {
               for( unsigned int k = firstK; k <= lastK; ++k )
               {
                  depthHighRes->set(i, j, k, DataAccess::Interface::DefaultUndefinedMapValue);
               }
            }
         }
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}



void DerivedProperties::DepthHighResFormationCalculator::computeForCoupledRunWithNonGeometricLoop( const GeoPhysics::Formation *       formation,
                                                                                                   const DataModel::AbstractSnapshot * snapshot,
                                                                                                         DerivedFormationPropertyPtr & depthHighRes,
                                                                                                         FormationPropertyList &       derivedProperties ) const
{
   try
   {
      const double time = snapshot->getTime();

      const bool includeGhostNodes = true;
      const unsigned int firstI = depthHighRes->firstI( includeGhostNodes );
      const unsigned int lastI  = depthHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = depthHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ  = depthHighRes->lastJ( includeGhostNodes );
      const unsigned int firstK = depthHighRes->firstK();
      const unsigned int lastK  = depthHighRes->lastK();

      double realThickness          = 0.0;
      double depthHighResValue      = 0.0;
      double depthHighResAboveValue = 0.0;
      
      for( unsigned int i = firstI; i <= lastI; ++i )
      {
         for( unsigned int j = firstJ; j <= lastJ; ++j )
         {
            if( m_projectHandle->getNodeIsValid(i, j) )
            {
               depthHighResAboveValue = depthHighRes->get(i, j, lastK);
               
               // Loop index is shifted up by 1.
               for( unsigned int k = lastK; k > firstK; --k )
               {
                  // index k     is top node of segment
                  // index k - 1 is bottom node of segment
                  realThickness = formation->getRealThickness( i, j, k - 1, time );
                  if( realThickness == DataAccess::Interface::DefaultUndefinedMapValue ||
                      realThickness == DataAccess::Interface::DefaultUndefinedScalarValue )
                  {
                     depthHighResValue = depthHighResAboveValue;
                  }
                  else
                  {
                     depthHighResValue = depthHighResAboveValue + realThickness;
                  }
                  depthHighRes->set( i, j, k - 1, depthHighResValue );
                  depthHighResAboveValue = depthHighResValue;
               }
            }
            else
            {
               for( unsigned int k = firstK; k <= lastK; ++k )
               {
                  depthHighRes->set(i, j, k, DataAccess::Interface::DefaultUndefinedMapValue);
               }
            }
         }
      }

      derivedProperties.clear();
      derivedProperties.push_back( depthHighRes );
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}



void DerivedProperties::DepthHighResFormationCalculator::computeForSubsampledRun(       AbstractPropertyManager &     propertyManager,
                                                                                  const GeoPhysics::Formation *       formation,
                                                                                  const DataModel::AbstractSnapshot * snapshot,
                                                                                        DerivedFormationPropertyPtr & depthHighRes,
                                                                                        FormationPropertyList &       derivedProperties ) const
{
   try
   {
      const double time = snapshot->getTime();

      const bool includeGhostNodes = true;
      const unsigned int firstI = depthHighRes->firstI( includeGhostNodes );
      const unsigned int lastI  = depthHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = depthHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ  = depthHighRes->lastJ( includeGhostNodes );
      const unsigned int firstK = depthHighRes->firstK();
      const unsigned int lastK  = depthHighRes->lastK();
      
      double solidThickness         = 0.0;
      double segmentThickness       = 0.0;
      double densityDifference      = 0.0;
      double depthHighResValue      = 0.0;
      double depthHighResAboveValue = 0.0;

      const GeoPhysics::FluidType * fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType());
      const double constFluidDensity = (fluid == 0) ? 0.0 : fluid->getConstantDensity();
      
      // I can't know at this level whether the VES or max VES will be required
      // because it depends on the following (i,j,k)-related specific conditions
      // - lithology(i,j)->isIncompressible()
      // - formation->isMobileLayer()
      // - solidThickness(i,j,k,t) > GeoPhysics::ThicknessTolerance
      const DataModel::AbstractProperty * const maxVesHighResProperty = propertyManager.getProperty( "MaxVesHighRes" );
      assert( maxVesHighResProperty != 0 );
      FormationPropertyPtr maxVesHighRes;
      
      // VES, pressure and temperature might be required only for coupled runs and not in the crust
      const DataModel::AbstractProperty * vesHighResProperty = 0;
      FormationPropertyPtr vesHighRes;

      if( !formation->isCrust() )
      {
         // The mantle case is handled in DepthHighResFormationCalculator::computeForMantle
         maxVesHighRes = propertyManager.getFormationProperty( maxVesHighResProperty, snapshot, formation );
         assert( maxVesHighRes != 0 );

         if( m_isCoupledMode )
         {
            vesHighResProperty = propertyManager.getProperty( "VesHighRes" );
            assert( vesHighResProperty != 0 );
            vesHighRes = propertyManager.getFormationProperty( vesHighResProperty, snapshot, formation );
            assert( vesHighRes != 0 );
         }
      }

      for( unsigned int i = firstI; i <= lastI; ++i )
      {
         for( unsigned int j = firstJ; j <= lastJ; ++j )
         {
            if( m_projectHandle->getNodeIsValid(i, j) )
            {
               depthHighResAboveValue = depthHighRes->get(i, j, lastK);
               const GeoPhysics::CompoundLithology * lithology = formation->getCompoundLithology(i, j);
               assert( lithology != 0 );

               // the density difference cannot be less than 0
               if ( lithology->density() > constFluidDensity ) 
               {
                  densityDifference = lithology->density() - constFluidDensity;
               }
               else
               {
                  densityDifference = 0.0;
               }

               const double oneMinusSurfacePorosity = 1.0 - lithology->surfacePorosity();
               
               // Loop index is shifted up by 1.
               for( unsigned int k = lastK; k > firstK; --k )
               {
                  // index k     is top node of segment
                  // index k - 1 is bottom node of segment
                  solidThickness = formation->getSolidThickness( i, j, k - 1, time);

                  if( lithology->isIncompressible() )
                  {
                     // This case covers also the crust
                     depthHighResValue = depthHighResAboveValue;
                     if( solidThickness > 0.0 )    // Is that check necessary?
                     {
                        depthHighResValue += solidThickness / oneMinusSurfacePorosity;
                     }
                  }
                  else if( formation->isMobileLayer() )
                  {
                     depthHighResValue = depthHighResAboveValue + solidThickness;
                  }
                  else
                  {
                     depthHighResValue = depthHighResAboveValue;
                     if( solidThickness > GeoPhysics::ThicknessTolerance )    // Is that check necessary?
                     {
                        if( m_isCoupledMode )
                        {
                           segmentThickness = lithology->computeSegmentThickness( maxVesHighRes->get( i, j, k ),
                                                                                  maxVesHighRes->get( i, j, k - 1 ),
                                                                                  vesHighRes->get( i, j, k ),
                                                                                  vesHighRes->get( i, j, k - 1 ),
                                                                                  densityDifference,
                                                                                  solidThickness );
                        }
                        else
                        {
                           // Should the ves be used here too?
                           segmentThickness = lithology->computeSegmentThickness( maxVesHighRes->get( i, j, k ),
                                                                                  maxVesHighRes->get( i, j, k - 1 ),
                                                                                  densityDifference,
                                                                                  solidThickness );
                        }
                        depthHighResValue += segmentThickness;
                     }
                  }
                  
                  depthHighRes->set(i, j, k - 1, depthHighResValue);
                  depthHighResAboveValue = depthHighResValue;
               }
            }
            else
            {
               for( unsigned int k = firstK; k <= lastK; ++k )
               {
                  depthHighRes->set(i, j, k, DataAccess::Interface::DefaultUndefinedMapValue);
               }
            }
         }
      }

      derivedProperties.clear();
      derivedProperties.push_back(depthHighRes);

   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}
