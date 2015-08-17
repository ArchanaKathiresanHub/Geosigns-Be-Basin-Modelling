#include "DataMiningProjectHandle.h"

#include <iostream>
#include <iomanip>
#include <set>
using namespace std;

#include "PropertyAttribute.h"

#include "DomainPropertyCollection.h"
#include "DomainPropertyFactory.h"

#include "HeatFlowMagnitudeCalculator.h"
#include "HeatFlowCalculator.h"
#include "FluidVelocityCalculator.h"
#include "FracturePressureCalculator.h"
#include "BrineDensityCalculator.h"
#include "BrineViscosityCalculator.h"
#include "RadiogenicHeatProductionCalculator.h"
#include "ThermalConductivityCalculator.h"
#include "TemperatureGradientCalculator.h"
#include "BasinTemperatureGradientCalculator.h"
#include "BasementHeatFlowCalculator.h"

DataAccess::Mining::ProjectHandle::ProjectHandle( database::Database * database, const std::string & name, const std::string & accessMode, DataAccess::Interface::ObjectFactory* objectFactory ) :
   GeoPhysics::ProjectHandle ( database, name, accessMode, objectFactory ) {

   addNewProperties();

   m_domainPropertyCollection = ((Mining::DomainPropertyFactory*)(getFactory()))->produceDomainPropertyCollection ( this );
   m_cauldronDomain = ((Mining::DomainPropertyFactory*)(getFactory()))->produceCauldronDomain ( this );

   Interface::PropertyList* currentProperties = getProperties ( true );
   Interface::PropertyList::iterator propIter;

   for ( propIter = currentProperties->begin (); propIter != currentProperties->end (); ++propIter ) {
      m_alternativeNamesMap [ (*propIter)->getName ()] = (*propIter)->getName ();
   }

   delete currentProperties;
}

void DataAccess::Mining::ProjectHandle::addNewProperties () {

   const Interface::Property* property;
   Interface::Property* newProperty;

   newProperty = getFactory ()->produceProperty (this, 0, "HeatFlowMagnitude", "HeatFlowMagnitude", "mW/m^2", Interface::FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new HeatFlowCalculatorAllocator ( HeatFlowCalculator::MAGNITUDE ));
   // ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new HeatFlowMagnitudeCalculatorAllocator );

   newProperty = getFactory ()->produceProperty (this, 0, "FluidVelocityMagnitude", "FluidVelocityMagnitude", "mm/y", Interface::FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new FluidVelocityCalculatorAllocator ( FluidVelocityCalculator::MAGNITUDE ));

   property = dynamic_cast<const Interface::Property*>(findProperty ( "BrineViscosity" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new BrineViscosityCalculatorAllocator );

   property = dynamic_cast<const Interface::Property*>(findProperty ( "BrineDensity" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new BrineDensityCalculatorAllocator );

   newProperty = getFactory ()->produceProperty (this, 0, "RadiogenicHeatProduction", "RadiogenicHeatProduction", "uW/m^3", Interface::FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new RadiogenicHeatProductionCalculatorAllocator );

   property = dynamic_cast<const Interface::Property*>(findProperty ( "FracturePressure" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new FracturePressureCalculatorAllocator );

   newProperty = getFactory ()->produceProperty (this, 0, "ThermalConductivity", "ThermalConductivity", "w/(K.m)", Interface::FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new ThermalConductivityCalculatorAllocator ( true ));

   newProperty = getFactory ()->produceProperty (this, 0, "ThermalConductivityH", "ThermalConductivityH", "w/(K.m)", Interface::FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new ThermalConductivityCalculatorAllocator ( false ));


   newProperty = getFactory ()->produceProperty (this, 0, "TemperatureGradient", "TemperatureGradient", "C/km", Interface::FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new TemperatureGradientCalculatorAllocator );

   newProperty = getFactory ()->produceProperty (this, 0, "BasinTemperatureGradient", "BasinTemperatureGradient", "C/km", Interface::FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new BasinTemperatureGradientCalculatorAllocator );

   newProperty = getFactory ()->produceProperty (this, 0, "BasementHeatFlow", "BasementHeatFlow", "mW/m^2", Interface::FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY );
   m_properties.push_back ( newProperty );
   m_derivedProperties.push_back ( newProperty );
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( newProperty, new BasementHeatFlowCalculatorAllocator );


   property = dynamic_cast<const Interface::Property*>(findProperty ( "FluidVelocityX" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new FluidVelocityCalculatorAllocator ( FluidVelocityCalculator::X ));

   property = dynamic_cast<const Interface::Property*>(findProperty ( "FluidVelocityY" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new FluidVelocityCalculatorAllocator ( FluidVelocityCalculator::Y ));

   property = dynamic_cast<const Interface::Property*>(findProperty ( "FluidVelocityZ" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new FluidVelocityCalculatorAllocator ( FluidVelocityCalculator::Z ));

   property = dynamic_cast<const Interface::Property*>(findProperty ( "HeatFlowX" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new HeatFlowCalculatorAllocator ( HeatFlowCalculator::X ));

   property = dynamic_cast<const Interface::Property*>(findProperty ( "HeatFlowY" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new HeatFlowCalculatorAllocator ( HeatFlowCalculator::Y ));

   property = dynamic_cast<const Interface::Property*>(findProperty ( "HeatFlowZ" ));
   m_derivedProperties.push_back ( const_cast<Interface::Property*>(property));
   ((Mining::DomainPropertyFactory*)(getFactory()))->addAllocator ( property, new HeatFlowCalculatorAllocator ( HeatFlowCalculator::Z ));



}


void DataAccess::Mining::ProjectHandle::addAlternativeName ( const std::string& propertyName,
                                                             const std::string& alternativeName ) {
   m_alternativeNamesMap [ alternativeName ] = propertyName;
}

const std::string& DataAccess::Mining::ProjectHandle::getPropertyNameFromAlternative ( const std::string& alternativeName ) const {

   StringMap::const_iterator nameIter = m_alternativeNamesMap.find ( alternativeName );

   if ( nameIter != m_alternativeNamesMap.end ()) {
      return nameIter->second;
   } else {
      return m_noName;
   }

}

void DataAccess::Mining::ProjectHandle::listProperties ( std::ostream& out ) const {

   Interface::MutablePropertyList::const_iterator propIter;

   out << std::endl << std::setw ( 40 ) << "Property name" << setw ( 15 ) << "Unit" << std::endl;

   for ( propIter = m_properties.begin (); propIter != m_properties.end (); ++propIter ) {
      const Interface::Property* property = *propIter;

      out << std::setw ( 40 ) << property->getName () << setw ( 15 ) << property->getUnit () << std::endl;
   }

}

void DataAccess::Mining::ProjectHandle::listSavedProperties ( std::ostream& out ) const {

   std::set<const Interface::Property*> properties;
   std::set<const Interface::Property*>::const_iterator propIter;
   Interface::MutablePropertyValueList::const_iterator propValIter;
   Interface::MutablePropertyList::const_iterator derivedPropIter;

   for ( propValIter = m_propertyValues.begin (); propValIter != m_propertyValues.end (); ++propValIter ) {

      const Interface::PropertyValue* propVal = *propValIter;

      properties.insert ( propVal->getProperty ());
   }

   out << std::endl << " It is possible for properties to be both saved and derived." << std::endl;
   out << " This means that the mined property value will be a calculated value" << std::endl;
   out << " rather than a value interpolated from the saved values." << std::endl;


   out << std::endl << "   Saved properties" << std::endl;
   out << std::setw ( 30 ) << "Property name" << setw ( 15 ) << "Unit" << std::endl;
   out << std::setw ( 30 ) << "-------------" << setw ( 15 ) << "----" << std::endl;

   for ( propIter = properties.begin (); propIter != properties.end (); ++propIter ) {

      const Interface::Property* property = *propIter;

      out << std::setw ( 30 ) << property->getName () << setw ( 15 ) << property->getUnit () << std::endl;
   }

   out << std::endl << std::endl << "   Derived properties" << std::endl;
   out << std::setw ( 30 ) << "Property name" << setw ( 15 ) << "Unit" << std::endl;
   out << std::setw ( 30 ) << "-------------" << setw ( 15 ) << "----" << std::endl;

   for ( derivedPropIter = m_derivedProperties.begin (); derivedPropIter != m_derivedProperties.end (); ++derivedPropIter ) {

      const Interface::Property* property = *derivedPropIter;

      out << std::setw ( 30 ) << property->getName () << setw ( 15 ) << property->getUnit () << std::endl;
      // properties.insert ( property );
   }


}

