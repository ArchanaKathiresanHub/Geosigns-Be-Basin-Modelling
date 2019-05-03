//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DATAACCESS__DOMAIN_PROPERTY_COLLECTION_H_
#define _DATAACCESS__DOMAIN_PROPERTY_COLLECTION_H_

// DataAccess
#include "Interface/Snapshot.h"
#include "Interface/Property.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"

#include "DerivedPropertyManager.h"

// DataMining
#include "DomainProperty.h"
#include "DataMiningObjectFactory.h"

// STL
#include <map>
#include <string>


namespace DataAccess
{
   namespace Mining
   {
      class ObjectFactory;

      /// A collection of domain-properties.
      ///
      /// Each one if allocated by the domain-property factory and saved in a
      /// mapping from the DAL::property to the DomainProperty.
      class DomainPropertyCollection
      {

         /// \brief A mapping from a DAL::property to a domain-property.
         typedef std::map < const Interface::Property*, DomainProperty* > PropertyToDomainProperty;

      public:

         DomainPropertyCollection ( Interface::ProjectHandle* projectHandle );

         virtual ~DomainPropertyCollection ();

         /// Return a pointer to the project-handle.
         Interface::ProjectHandle * getProjectHandle() const { return m_projectHandle; }

         /// Return a constant pointer to the current snapshot.
         const Interface::Snapshot * getSnapshot() const { return m_snapshot; }

         /// Deletes all properties that have been collected.
         virtual void clear ();

         /// Set the snapshot.
         ///
         /// Deletes all previously collected data.
         /// All properties allocated from this point forward will be from this snapshot time.
         /// If the snapshot to be set is the same as that aslready set then nothing will change.
         virtual void setSnapshot( const Interface::Snapshot* snapshot );

         /// Retrieve a domain-property from the collection.
         ///
         /// The first time a particular property is selected it will
         /// be allocated by the factory and saved in the collection.
         virtual DomainProperty * getDomainProperty ( const std::string &                        propertyName,
                                                      DerivedProperties::DerivedPropertyManager& propertyManager ) const;

         /// Retrieve a domain-property from the collection.
         ///
         /// The first time a particular property is selected it will
         /// be allocated by the factory and saved in the collection.
         virtual DomainProperty * getDomainProperty ( const Interface::Property *                property,
                                                      DerivedProperties::DerivedPropertyManager& propertyManager ) const;

         /// \brief Initialises all the domain-properties that are in the collection.
         void initialiseProperties ();

         /// Determine if a property is a part of the collection.
         virtual bool contains ( const Interface::Property* property ) const;


      private :

         Interface::ProjectHandle*      m_projectHandle;   ///< The project-handle.
         const Mining::ObjectFactory*   m_propertyFactory; ///< The factory that allocates all domain-properties.
         const Interface::Snapshot*     m_snapshot;        ///< The snapshot at which all properties are retrieved.

         /// The collection of domain-properties.
         ///
         /// A mapping from the DAL::property to the domain-property.
         // This is mutable so that properties can be added if they are not in the map
         // in the get-domain-property functions and these function remain const.
         mutable PropertyToDomainProperty m_domainProperty;
      };
   }
}

#endif // _DATAACCESS__DOMAIN_PROPERTY_COLLECTION_H_
