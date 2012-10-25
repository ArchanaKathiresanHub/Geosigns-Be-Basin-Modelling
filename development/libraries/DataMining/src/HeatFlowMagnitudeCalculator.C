#include "HeatFlowMagnitudeCalculator.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithology.h"

DataAccess::Mining::HeatFlowMagnitudeCalculator::HeatFlowMagnitudeCalculator ( const DomainPropertyCollection*  collection,
                                                                               const Interface::Snapshot* snapshot,
                                                                               const Interface::Property* property ) :
   DomainFormationProperty ( collection, snapshot, property )
{
   m_heatFlowX = 0;
   m_heatFlowY = 0;
   m_heatFlowZ = 0;
   m_initialised = false;
}

bool DataAccess::Mining::HeatFlowMagnitudeCalculator::initialise () {

   if ( not m_initialised ) {
      m_heatFlowX = getPropertyCollection ()->getDomainProperty ( "HeatFlowX" );
      m_heatFlowY = getPropertyCollection ()->getDomainProperty ( "HeatFlowY" );
      m_heatFlowZ = getPropertyCollection ()->getDomainProperty ( "HeatFlowZ" );

      if ( m_heatFlowX != 0 and m_heatFlowY != 0 and m_heatFlowZ != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::HeatFlowMagnitudeCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised ) {
      return Interface::DefaultUndefinedMapValue;
   }

   double heatFlowX;
   double heatFlowY;
   double heatFlowZ;
   double heatFlowMagnitude;

   heatFlowX = m_heatFlowX->compute ( position );
   heatFlowY = m_heatFlowY->compute ( position );
   heatFlowZ = m_heatFlowZ->compute ( position );

   if ( heatFlowX != Interface::DefaultUndefinedMapValue and heatFlowY != Interface::DefaultUndefinedMapValue and heatFlowZ != Interface::DefaultUndefinedMapValue ) {
      heatFlowMagnitude = std::sqrt ( heatFlowX * heatFlowX + heatFlowY * heatFlowY + heatFlowZ * heatFlowZ );
   } else {
      heatFlowMagnitude = Interface::DefaultUndefinedMapValue;
   }

   return heatFlowMagnitude;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::HeatFlowMagnitudeCalculatorAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                         const Interface::Snapshot* snapshot,
                                                                                                         const Interface::Property* property ) const {
   return new HeatFlowMagnitudeCalculator ( collection, snapshot, property );
}
