#include "BrineViscosityCalculator.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithology.h"

DataAccess::Mining::BrineViscosityCalculator::BrineViscosityCalculator ( const DomainPropertyCollection*            collection,
                                                                         DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                         const Interface::Snapshot*                 snapshot,
                                                                         const Interface::Property*                 property ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property )
{
   m_temperature = 0;
   m_pressure = 0;
   m_initialised = false;
}

bool DataAccess::Mining::BrineViscosityCalculator::initialise () {

   if ( not m_initialised ) {
      m_temperature = getPropertyCollection ()->getDomainProperty ( "Temperature", getPropertyManager ());
      m_pressure = getPropertyCollection ()->getDomainProperty ( "Pressure", getPropertyManager ());
      
      if ( m_temperature != 0 && m_pressure != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::BrineViscosityCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised ) {
      return Interface::DefaultUndefinedMapValue;
   }

   double temperature;
   double pressure;
   double viscosity;

   const GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(position.getFormation ());
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

   temperature = m_temperature->compute ( position );
   pressure = m_pressure->compute ( position );
   
   if ( temperature != Interface::DefaultUndefinedMapValue and fluid != 0 ) {
      viscosity = fluid->viscosity ( temperature, pressure );
   } else {
      viscosity = Interface::DefaultUndefinedMapValue;
   }

   return viscosity;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::BrineViscosityCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                      DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                      const Interface::Snapshot*                 snapshot,
                                                                                                      const Interface::Property*                 property ) const {
   return new BrineViscosityCalculator ( collection, propertyManager, snapshot, property );
}
