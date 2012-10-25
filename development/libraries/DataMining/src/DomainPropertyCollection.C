#include "DomainPropertyCollection.h"

#include <iostream>

#include "DomainPropertyFactory.h"

DataAccess::Mining::DomainPropertyCollection::DomainPropertyCollection ( Interface::ProjectHandle* handle ) : m_projectHandle ( handle ) {
   m_snapshot = 0;
   m_propertyFactory = (Mining::DomainPropertyFactory*)(m_projectHandle->getFactory ());
}

DataAccess::Mining::DomainPropertyCollection::~DomainPropertyCollection () {
   m_snapshot = 0;

   clear ();
}

DataAccess::Interface::ProjectHandle* DataAccess::Mining::DomainPropertyCollection::getProjectHandle () const {
   return m_projectHandle;
}

const DataAccess::Interface::Snapshot* DataAccess::Mining::DomainPropertyCollection::getSnapshot () const {
   return m_snapshot;
} 


void DataAccess::Mining::DomainPropertyCollection::setSnapshot ( const Interface::Snapshot* snapshot ) {

   if ( snapshot != m_snapshot ) {
      clear ();
      m_snapshot = snapshot;
   }

}

void DataAccess::Mining::DomainPropertyCollection::clear () {

   PropertyToDomainProperty::iterator propIter;

   for ( propIter = m_domainProperty.begin (); propIter != m_domainProperty.end (); ++propIter ) {
      delete propIter->second;
   }

   m_domainProperty.clear ();
   m_snapshot = 0;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainPropertyCollection::getDomainProperty ( const std::string& propertyName ) const {

   const Interface::Property* property = m_projectHandle->findProperty ( propertyName );

   assert ( property != 0 );

   return getDomainProperty ( property );
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::DomainPropertyCollection::getDomainProperty ( const Interface::Property* property ) const {

   assert ( property != 0 );

   PropertyToDomainProperty::iterator propIter = m_domainProperty.find ( property );
   DomainProperty* result;

   if ( propIter != m_domainProperty.end ()) {
      // If a property already exists then return the corresponding domain-property.
      result = propIter->second;
   } else {
      // If the property does not exist in the map then allocate a new one using the factory.
      // Then add it to the map and return the newly created domain-property.
      result = m_propertyFactory->allocate ( this, m_snapshot, property );

      if ( result == 0 ) {
         std::cerr << "  Property " << property->getName () << " cannot be found in the domain-property factory." << std::endl;
         std::exit ( 1 );
      }

      m_domainProperty [ property ] = result;
   }

   return result;
}

void DataAccess::Mining::DomainPropertyCollection::initialiseProperties () {

   PropertyToDomainProperty::iterator iter;

   for ( iter = m_domainProperty.begin (); iter != m_domainProperty.end (); ++iter ) {
      iter->second->initialise ();
   }

}


bool DataAccess::Mining::DomainPropertyCollection::contains ( const Interface::Property* property ) const {

   PropertyToDomainProperty::const_iterator propIter = m_domainProperty.find ( property );

   return propIter != m_domainProperty.end ();
}
