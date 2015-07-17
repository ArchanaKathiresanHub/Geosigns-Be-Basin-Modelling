#include "DomainPropertyCollection.h"
#include "DomainPropertyFactory.h"

#include <iostream>

namespace DataAccess { namespace Mining
{

   DomainPropertyCollection::DomainPropertyCollection ( Interface::ProjectHandle* handle ) :
      m_projectHandle ( handle ),
      m_snapshot ( 0 )
   {
      m_propertyFactory = dynamic_cast<Mining::DomainPropertyFactory*>( m_projectHandle->getFactory ());
   }

   DomainPropertyCollection::~DomainPropertyCollection()
   {
      m_snapshot = 0;
      clear ();
   }

   void DomainPropertyCollection::setSnapshot ( const Interface::Snapshot * snapshot )
   {

      if ( snapshot != m_snapshot )
      {
         clear();
         m_snapshot = snapshot;
      }

   }

   void DomainPropertyCollection::clear()
   {
      for ( PropertyToDomainProperty::iterator propIter = m_domainProperty.begin(); propIter != m_domainProperty.end(); ++propIter )
      {
         delete propIter->second;
      }

      m_domainProperty.clear();
      m_snapshot = 0;
   }

   DomainProperty * DomainPropertyCollection::getDomainProperty( const std::string &                        propertyName,
                                                                 DerivedProperties::DerivedPropertyManager& propertyManager ) const
   {
      const Interface::Property * property = m_projectHandle->findProperty( propertyName );
      assert ( property != 0 );
      return getDomainProperty ( property, propertyManager );
   }

   DomainProperty * DomainPropertyCollection::getDomainProperty( const Interface::Property *                property,
                                                                 DerivedProperties::DerivedPropertyManager& propertyManager ) const
   {
      assert ( property != 0 );

      PropertyToDomainProperty::iterator propIter = m_domainProperty.find( property );
      DomainProperty                   * result   = 0;

      if ( propIter != m_domainProperty.end() )
      {
         // If a property already exists then return the corresponding domain-property.
         result = propIter->second;
      }
      else
      {
         // If the property does not exist in the map then allocate a new one using the factory.
         // Then add it to the map and return the newly created domain-property.
         result = m_propertyFactory->allocate( this, propertyManager, m_snapshot, property );
         if ( !result )
         {
            std::cerr << "  Property " << property->getName () << " cannot be found in the domain-property factory." << std::endl;
            std::exit ( 1 );
         }

         m_domainProperty [ property ] = result;
      }
      return result;
   }

   void DomainPropertyCollection::initialiseProperties()
   {
      for ( PropertyToDomainProperty::iterator iter = m_domainProperty.begin(); iter != m_domainProperty.end(); ++iter )
      {
         iter->second->initialise();
      }
   }

   bool DomainPropertyCollection::contains( const Interface::Property * property ) const
   {
      return m_domainProperty.find( property ) != m_domainProperty.end();
   }

}} // namespace DataAccess::Mining
