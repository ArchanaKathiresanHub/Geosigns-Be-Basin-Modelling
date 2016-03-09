#include "ThermalConductivityCalculator.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithology.h"
#include "CompoundProperty.h"


DataAccess::Mining::ThermalConductivityCalculator::ThermalConductivityCalculator ( const DomainPropertyCollection*            collection,
                                                                                   DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                   const Interface::Snapshot*                 snapshot,
                                                                                   const Interface::Property*                 property,
                                                                                   const bool                                 normalConductivity ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property ),
   m_normalConductivity ( normalConductivity )
{
   m_temperature = 0;
   m_pressure = 0;
   m_porosity = 0;
   m_initialised = false;
}

bool DataAccess::Mining::ThermalConductivityCalculator::initialise () {

   if ( not m_initialised ) {
      m_temperature = getPropertyCollection ()->getDomainProperty ( "Temperature", getPropertyManager ());
      m_pressure = getPropertyCollection ()->getDomainProperty ( "Pressure", getPropertyManager ());
      m_porosity = getPropertyCollection ()->getDomainProperty ( "Porosity", getPropertyManager ());

      if ( m_temperature != 0 and m_porosity != 0 and m_pressure != 0 ) {
         m_initialised = m_porosity->initialise();
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}
double DataAccess::Mining::ThermalConductivityCalculator::compute ( const ElementPosition& position ) const {

   if ( not position.isValidElement () or not m_initialised ) {
      return Interface::DefaultUndefinedMapValue;
   }

   const Interface::Formation* formation = position.getFormation ();
   const GeoPhysics::Formation* geoForm = dynamic_cast<const GeoPhysics::Formation*>( formation );
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>( geoForm->getFluidType ());

   if ( formation == 0 or fluid == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   double porosity;
   double temperature;
   double pressure;
   double conductivityN;
   double conductivityP;

   porosity = m_porosity->compute ( position );
   temperature = m_temperature->compute ( position );
   pressure = m_pressure->compute ( position );

   if ( temperature != Interface::DefaultUndefinedMapValue and porosity != Interface::DefaultUndefinedMapValue and pressure != Interface::DefaultUndefinedMapValue  ) {
      porosity = 0.01 * porosity;

      const GeoPhysics::CompoundLithology* lithology = geoForm->getCompoundLithology ( position.getI (), position.getJ ());

      lithology->calcBulkThermCondNP ( fluid, porosity, temperature, pressure, conductivityN, conductivityP );
   } else {
      conductivityN = Interface::DefaultUndefinedMapValue;
      conductivityP = Interface::DefaultUndefinedMapValue;
   }

   if ( m_normalConductivity ) {
      return conductivityN;
   } else {
      return conductivityP;
   }

}

DataAccess::Mining::ThermalConductivityCalculatorAllocator::ThermalConductivityCalculatorAllocator ( const bool normalConductivity ) :
   m_normalConductivity ( normalConductivity )
{
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::ThermalConductivityCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                           DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                           const Interface::Snapshot*                 snapshot,
                                                                                                           const Interface::Property*                 property ) const {
   return new ThermalConductivityCalculator ( collection, propertyManager, snapshot, property, m_normalConductivity );
}
