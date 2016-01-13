#include "MaxVesHighResFormationCalculator.h"

#include "FormattingException.h"
#include "Interface/SimulationDetails.h"
#include "IndirectFormationProperty.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"

DerivedProperties::MaxVesHighResFormationCalculator::MaxVesHighResFormationCalculator( const GeoPhysics::ProjectHandle * projectHandle ) :
   m_projectHandle( projectHandle ),
   m_isCoupledMode( false ),
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

      m_isCoupledMode = simulationDetails->getSimulatorMode () == "Overpressure" ||
                        simulationDetails->getSimulatorMode () == "LooselyCoupledTemperature" ||
                        simulationDetails->getSimulatorMode () == "CoupledHighResDecompaction" ||
                        simulationDetails->getSimulatorMode () == "CoupledPressureAndTemperature" ||
                        simulationDetails->getSimulatorMode () == "CoupledDarcy";

      if( !m_isSubsampled || m_isCoupledMode )
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
      // Check snapshot. It's not possible to ask for this property at a snapshot age earlier than the formation deposition age
      const GeoPhysics::Formation * const currentFormation = dynamic_cast<const GeoPhysics::Formation * const>( formation );
      if( m_projectHandle->findPreviousSnapshot( snapshot->getTime() ) > currentFormation->getBottomSurface()->getSnapshot() )
      {
         throw formattingexception::GeneralException() << "Invalid snapshot provided";
      }

      if( !m_isSubsampled || m_isCoupledMode )
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

      const DataModel::AbstractProperty * const maxVesProperty = propertyManager.getProperty( "MaxVes" );
      FormationPropertyPtr maxVes = propertyManager.getFormationProperty( maxVesProperty, snapshot, formation );

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

      DerivedFormationPropertyPtr maxVesHighRes = 
         DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( maxVesHighResProperty,
                                                                                       snapshot,
                                                                                       formation,
                                                                                       propertyManager.getMapGrid(),
                                                                                       currentFormation->getMaximumNumberOfElements() + 1 ) );
      
      if( snapshot->getTime() == currentFormation->getBottomSurface()->getSnapshot()->getTime() )
      {
         computeAtDepositionAge( maxVesHighRes );
      }
      else
      {
         computeAtGenericAge( propertyManager,
                              snapshot,
                              formation,
                              maxVesHighRes );
      }

      derivedProperties.clear();
      derivedProperties.push_back(maxVesHighRes);

   }
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}


void DerivedProperties::MaxVesHighResFormationCalculator::computeAtDepositionAge( DerivedFormationPropertyPtr & maxVesHighRes ) const
{
   try
   {
      const bool includeGhostNodes = true;
      const unsigned int firstI = maxVesHighRes->firstI( includeGhostNodes );
      const unsigned int lastI  = maxVesHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = maxVesHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ  = maxVesHighRes->lastJ( includeGhostNodes );
      const unsigned int firstK = maxVesHighRes->firstK();
      const unsigned int lastK  = maxVesHighRes->lastK();

      for( unsigned int i = firstI; i <= lastI; ++i )
      {
         for( unsigned int j = firstJ; j <= lastJ; ++j )
         {
            if( m_projectHandle->getNodeIsValid(i, j) )
            {
               for( unsigned int k = firstK; k <= lastK; ++k )
               {
                  maxVesHighRes->set(i, j, k, 0.0);
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
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}





void DerivedProperties::MaxVesHighResFormationCalculator::computeAtGenericAge(       AbstractPropertyManager &      propertyManager,
                                                                               const DataModel::AbstractSnapshot *  snapshot,
                                                                               const DataModel::AbstractFormation * formation,
                                                                                     DerivedFormationPropertyPtr & maxVesHighRes ) const
{
   try
   {
      const DataModel::AbstractProperty * const vesHighResProperty = propertyManager.getProperty( "VesHighRes" );

      FormationPropertyPtr currentVesHighRes = propertyManager.getFormationProperty( vesHighResProperty, snapshot, formation );

      const DataAccess::Interface::Snapshot * const prevSnapshot = m_projectHandle->findPreviousSnapshot(snapshot->getTime());
      FormationPropertyPtr previousVesHighRes = propertyManager.getFormationProperty( vesHighResProperty, prevSnapshot, formation );

      const bool includeGhostNodes = true;
      const unsigned int firstI = maxVesHighRes->firstI( includeGhostNodes );
      const unsigned int lastI  = maxVesHighRes->lastI( includeGhostNodes );
      const unsigned int firstJ = maxVesHighRes->firstJ( includeGhostNodes );
      const unsigned int lastJ  = maxVesHighRes->lastJ( includeGhostNodes );
      const unsigned int firstK = maxVesHighRes->firstK();
      const unsigned int lastK  = maxVesHighRes->lastK();

      for( unsigned int i = firstI; i <= lastI; ++i )
      {
         for( unsigned int j = firstJ; j <= lastJ; ++j )
         {
            if( m_projectHandle->getNodeIsValid(i, j) )
            {
               for( unsigned int k = firstK; k <= lastK; ++k )
               {
                  maxVesHighRes->set(i, j, k, max( currentVesHighRes->getA(i,j,k), previousVesHighRes->getA(i,j,k) ) );
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
   catch( formattingexception::GeneralException & ex )
   {
      throw ex;
   }
}
