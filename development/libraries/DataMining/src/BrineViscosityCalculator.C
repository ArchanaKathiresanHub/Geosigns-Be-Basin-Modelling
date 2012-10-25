#include "BrineViscosityCalculator.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithology.h"

DataAccess::Mining::BrineViscosityCalculator::BrineViscosityCalculator ( const DomainPropertyCollection*  collection,
                                                                         const Interface::Snapshot* snapshot,
                                                                         const Interface::Property* property ) :
   DomainFormationProperty ( collection, snapshot, property )
{
   m_temperature = 0;
   m_initialised = false;
}

bool DataAccess::Mining::BrineViscosityCalculator::initialise () {

   if ( not m_initialised ) {
      m_temperature = getPropertyCollection ()->getDomainProperty ( "Temperature" );

      if ( m_temperature != 0 ) {
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
   double porePressure;
   double viscosity;

   const GeoPhysics::Formation* formation = dynamic_cast<const GeoPhysics::Formation*>(position.getFormation ());
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

   temperature = m_temperature->compute ( position );

   if ( temperature != Interface::DefaultUndefinedMapValue and fluid != 0 ) {
      viscosity = fluid->viscosity ( temperature );
   } else {
      viscosity = Interface::DefaultUndefinedMapValue;
   }

   return viscosity;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::BrineViscosityCalculatorAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                      const Interface::Snapshot* snapshot,
                                                                                                      const Interface::Property* property ) const {
   return new BrineViscosityCalculator ( collection, snapshot, property );
}
