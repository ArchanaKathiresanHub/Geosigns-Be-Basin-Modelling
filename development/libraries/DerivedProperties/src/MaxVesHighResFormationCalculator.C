//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MaxVesHighResFormationCalculator.h"

#include <assert.h>
#include "FormattingException.h"
#include "Interface/SimulationDetails.h"
#include "IndirectFormationProperty.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "NumericFunctions.h"

DerivedProperties::MaxVesHighResFormationCalculator::MaxVesHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle ) :
   m_projectHandle( projectHandle ),
   m_isSubsampled( ! ((*(m_projectHandle->getLowResolutionOutputGrid())) == (*(m_projectHandle->getHighResolutionOutputGrid()))) )
{
   try
   {
      addPropertyName( "MaxVesHighRes" );

      const DataAccess::Interface::SimulationDetails * simulationDetails = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

      if( simulationDetails == 0 )
      {
         throw formattingexception::GeneralException() << "Last simulation is missing";
      }

      if( !m_isSubsampled )
      {
         addDependentPropertyName( "MaxVes" );
      }
      else
      {
         addDependentPropertyName( "VesHighRes" );
      }
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}

void DerivedProperties::MaxVesHighResFormationCalculator::calculate(       AbstractPropertyManager &      propertyManager,
                                                                     const DataModel::AbstractSnapshot *  snapshot,
                                                                     const DataModel::AbstractFormation * formation,
                                                                           FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const GeoPhysics::Formation * const currentFormation = dynamic_cast<const GeoPhysics::Formation * const>( formation );
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
         computeForSubsampledRun( propertyManager,
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


void DerivedProperties::MaxVesHighResFormationCalculator::computeIndirectly(       AbstractPropertyManager &      propertyManager,
                                                                             const DataModel::AbstractSnapshot *  snapshot,
                                                                             const DataModel::AbstractFormation * formation,
                                                                                   FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const DataModel::AbstractProperty * const maxVesHighResProperty = propertyManager.getProperty( getPropertyNames()[ 0 ] );
      assert( maxVesHighResProperty != 0 );

      const DataModel::AbstractProperty * const maxVesProperty = propertyManager.getProperty( "MaxVes" );
      assert( maxVesProperty != 0 );
      FormationPropertyPtr maxVes = propertyManager.getFormationProperty( maxVesProperty, snapshot, formation );
      assert( maxVes != 0 );

      IndirectFormationPropertyPtr maxVesHighRes = IndirectFormationPropertyPtr( new DerivedProperties::IndirectFormationProperty( maxVesHighResProperty, maxVes) );

      derivedProperties.clear();
      derivedProperties.push_back( maxVesHighRes );
   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::MaxVesHighResFormationCalculator::computeForSubsampledRun(       AbstractPropertyManager &      propertyManager,
                                                                                   const DataModel::AbstractSnapshot *  snapshot,
                                                                                   const DataModel::AbstractFormation * formation,
                                                                                         FormationPropertyList &        derivedProperties ) const
{
   try
   {
      const GeoPhysics::Formation * const currentFormation = dynamic_cast<const GeoPhysics::Formation * const>( formation );

      const DataModel::AbstractProperty * const maxVesHighResProperty = propertyManager.getProperty( getPropertyNames()[ 0 ] );
      assert( maxVesHighResProperty != 0 );

      DerivedFormationPropertyPtr maxVesHighRes = 
         DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( maxVesHighResProperty,
                                                                                       snapshot,
                                                                                       formation,
                                                                                       propertyManager.getMapGrid(),
                                                                                       currentFormation->getMaximumNumberOfElements() + 1 ) );

      const bool includeGhostNodes = true;
      const unsigned int firstI = maxVesHighRes->firstI( includeGhostNodes );
      const unsigned int lastI  = maxVesHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = maxVesHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ  = maxVesHighRes->lastJ( includeGhostNodes );
      const unsigned int firstK = maxVesHighRes->firstK();
      const unsigned int lastK  = maxVesHighRes->lastK();

      const DataAccess::Interface::Snapshot * const prevSnapshot = m_projectHandle->findPreviousSnapshot( snapshot->getTime() );
      // Check snapshot. It's not possible to ask for this property at a snapshot age earlier than the formation deposition age
      if( prevSnapshot != 0 and 
          prevSnapshot->getTime() > currentFormation->getBottomSurface()->getSnapshot()->getTime() )
      {
         throw formattingexception::GeneralException() << "Invalid snapshot provided";
      }
      FormationPropertyPtr previousMaxVesHighRes = propertyManager.getFormationProperty( maxVesHighResProperty, prevSnapshot, formation );

      const DataModel::AbstractProperty * const vesHighResProperty = propertyManager.getProperty( "VesHighRes" );
      assert( vesHighResProperty != 0 );
      FormationPropertyPtr currentVesHighRes = propertyManager.getFormationProperty( vesHighResProperty, snapshot, formation );
      assert( currentVesHighRes != 0 );

      // If the previous snapshot is the deposition snapshot of the current formation
      // the max VES is the VES itself
      // These 2 conditions should be the same (hopefully)
      if( previousMaxVesHighRes == 0 &&
          prevSnapshot->getTime() == currentFormation->getBottomSurface()->getSnapshot()->getTime() )
      {
         for( unsigned int i = firstI; i <= lastI; ++i )
         {
            for( unsigned int j = firstJ; j <= lastJ; ++j )
            {
               if( m_projectHandle->getNodeIsValid(i, j) )
               {
                  for( unsigned int k = firstK; k <= lastK; ++k )
                  {
                     maxVesHighRes->set(i, j, k, currentVesHighRes->getA(i,j,k) );
                  }
               }
               else
               {
                  for( unsigned int k = firstK; k <= lastK; ++k )
                  {
                     maxVesHighRes->set(i, j, k, DataAccess::Interface::DefaultUndefinedMapValue);
                  }
               }
            }
         }
      }
      else
      {
         for( unsigned int i = firstI; i <= lastI; ++i )
         {
            for( unsigned int j = firstJ; j <= lastJ; ++j )
            {
               if( m_projectHandle->getNodeIsValid(i, j) )
               {
                  for( unsigned int k = firstK; k <= lastK; ++k )
                  {
                     maxVesHighRes->set(i, j, k, NumericFunctions::Maximum( currentVesHighRes->getA(i,j,k), previousMaxVesHighRes->getA(i,j,k) ) );
                  }
               }
               else
               {
                  for( unsigned int k = firstK; k <= lastK; ++k )
                  {
                     maxVesHighRes->set(i, j, k, DataAccess::Interface::DefaultUndefinedMapValue);
                  }
               }
            }
         }
      }

      derivedProperties.clear();
      derivedProperties.push_back(maxVesHighRes);

   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}
