#include "DepthHighResFormationCalculator.h"

#include <assert.h>
#include "CompoundLithology.h"
#include "FormattingException.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsCrustFormation.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsMantleFormation.h"
#include "IndirectFormationProperty.h"
#include "Interface/MantleFormation.h"
#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "NumericFunctions.h"

DerivedProperties::DepthHighResFormationCalculator::DepthHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle ) :
   m_projectHandle( projectHandle ),
   m_isCoupledMode( false ),
   m_isSubsampled( ! ((*(m_projectHandle->getLowResolutionOutputGrid())) == (*(m_projectHandle->getHighResolutionOutputGrid()))) ),
   m_isNonGeometricLoopActive( m_projectHandle->getRunParameters()->getNonGeometricLoop() )
{
   try
   {
      addPropertyName( "DepthHighRes" );

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
      if( !m_isSubsampled )
      {
         computeIndirectly( propertyManager,
                            snapshot,
                            formation,
                            derivedProperties );
      }
      else
      {
         const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

         const DataModel::AbstractProperty * depthHighResProperty = propertyManager.getProperty( getPropertyNames()[ 0 ] );

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

         for( unsigned int i = firstI; i <= lastI; ++i )
         {
            for( unsigned int j = firstJ; j <= lastJ; ++j )
            {
               depthHighRes->set( i, j, topNodeIndex, depthHighResAbove->getA( i, j, 0 ) );
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

                  const GeoPhysics::GeoPhysicsMantleFormation * mantle = dynamic_cast<const GeoPhysics::GeoPhysicsMantleFormation*>( m_projectHandle->getMantleFormation() );
                  mantleSegmentHeight = mantle->m_mantleElementHeight0 / crustThinningRatio;
               }
               else
               {
                  basementThickness = mantleThickness + m_projectHandle->getCrustThickness( i, j, 0.0 );
                  mantleSegmentHeight = m_projectHandle->getRunParameters()->getBrickHeightMantle() / crustThinningRatio;
               }

               const double currentMantleThickness  = basementThickness - currentCrustThickness;
               const double maximumDepth            = depthHighRes->getA( i, j, lastK ) + currentMantleThickness;
               for( unsigned int k = lastK; k > firstK; --k )
               {
                  depthHighRes->set( i, j, k - 1, NumericFunctions::Minimum(maximumDepth, depthHighRes->getA( i, j, k ) + mantleSegmentHeight) );
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
               const GeoPhysics::CompoundLithology * const lithology = formation->getCompoundLithology(i, j);
               depthHighResAboveValue = depthHighRes->getA(i, j, lastK);
               
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
                  depthHighRes->set( i, j, k, depthHighResValue );
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
      const double fluidDensity = (fluid == 0) ? 0.0 : fluid->getConstantDensity();
      
      // I can't know at this level whether the VES or max VES will be required
      // because it depends on the following (i,j,k)-related specific conditions
      // - lithology->isIncompressible()
      // - formation->isMobileLayer()
      // - solidThickness > GeoPhysics::ThicknessTolerance
      const DataModel::AbstractProperty * const maxVesHighResProperty = propertyManager.getProperty( "MaxVesHighRes" );
      FormationPropertyPtr maxVesHighRes;

      if( !formation->isCrust() )
      {
         maxVesHighRes = propertyManager.getFormationProperty( maxVesHighResProperty, snapshot, formation );
      }

      // VES might be required only for coupled runs
      const DataModel::AbstractProperty * vesHighResProperty = 0;
      FormationPropertyPtr vesHighRes;
      if( m_isCoupledMode )
      {
         vesHighResProperty = propertyManager.getProperty( "VesHighRes" );
         vesHighRes         = propertyManager.getFormationProperty( vesHighResProperty, snapshot, formation );
      }

      for( unsigned int i = firstI; i <= lastI; ++i )
      {
         for( unsigned int j = firstJ; j <= lastJ; ++j )
         {
            if( m_projectHandle->getNodeIsValid(i, j) )
            {
               depthHighResAboveValue = depthHighRes->getA(i, j, lastK);
               const GeoPhysics::CompoundLithology * lithology = formation->getCompoundLithology(i, j);
               densityDifference = lithology->density() - fluidDensity;    // Is that correct?? Fluid density should depend on P-T...
               const double oneMinusSurfacePorosity = 1.0 - lithology->surfacePorosity();
               
               // Loop index is shifted up by 1.
               for( unsigned int k = lastK; k > firstK; --k )
               {
                  // index k     is top node of segment
                  // index k - 1 is bottom node of segment
                  solidThickness = formation->getSolidThickness( i, j, k - 1, time);

                  if( lithology->isIncompressible() )
                  {
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
                           assert( maxVesHighRes != 0 );
                           assert( vesHighRes != 0 );
                           segmentThickness = lithology->computeSegmentThickness( maxVesHighRes->getA( i, j, k ),
                                                                                  maxVesHighRes->getA( i, j, k - 1 ),
                                                                                  vesHighRes->getA( i, j, k ),
                                                                                  vesHighRes->getA( i, j, k - 1 ),
                                                                                  densityDifference,
                                                                                  solidThickness );
                        }
                        else
                        {
                           assert( maxVesHighRes != 0 );
                           // Should the ves be used here too?
                           segmentThickness = lithology->computeSegmentThickness( maxVesHighRes->getA( i, j, k ),
                                                                                  maxVesHighRes->getA( i, j, k - 1 ),
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
