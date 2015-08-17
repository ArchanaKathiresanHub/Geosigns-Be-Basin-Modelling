#include "BrineDensityCalculator.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithology.h"

DataAccess::Mining::BrineDensityCalculator::BrineDensityCalculator ( const DomainPropertyCollection*            collection,
                                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                     const Interface::Snapshot*                 snapshot,
                                                                     const Interface::Property*                 property ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property )
{
   m_temperature = 0;
   m_porePressure = 0;
   m_initialised = false;
}

bool DataAccess::Mining::BrineDensityCalculator::initialise () {

   if ( not m_initialised ) {
      m_temperature = getPropertyCollection ()->getDomainProperty ( "Temperature", getPropertyManager ());
      m_porePressure = getPropertyCollection ()->getDomainProperty ( "Pressure", getPropertyManager ());

      if ( m_temperature != 0 and m_porePressure != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::BrineDensityCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised ) {
      return Interface::DefaultUndefinedMapValue;
   }

   double temperature;
   double porePressure;
   double density;

   const GeoPhysics::Formation* formation = dynamic_cast<const GeoPhysics::Formation*>(position.getFormation ());
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

   temperature = m_temperature->compute ( position );
   porePressure = m_porePressure->compute ( position );

   if ( temperature != Interface::DefaultUndefinedMapValue and porePressure != Interface::DefaultUndefinedMapValue and fluid != 0 ) {
      density = fluid->density ( temperature, porePressure );
   } else {
      density = Interface::DefaultUndefinedMapValue;
   }

   return density;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::BrineDensityCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                    DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                    const Interface::Snapshot*                 snapshot,
                                                                                                    const Interface::Property*                 property ) const {
   return new BrineDensityCalculator ( collection, propertyManager, snapshot, property );
}
