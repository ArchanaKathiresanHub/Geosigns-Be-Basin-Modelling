#ifndef _DATAACCESS__DOMAIN_PROPERTY_COLLECTION_H_
#define _DATAACCESS__DOMAIN_PROPERTY_COLLECTION_H_

#include <map>
#include <string>

#include "Interface/Snapshot.h"
#include "Interface/Property.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"


#include "DomainProperty.h"
#include "DomainPropertyFactory.h"
#include "DomainPropertyCollection.h"

namespace DataAccess {

   namespace Mining {

      class DomainPropertyFactory;

   }

}

namespace DataAccess {

   namespace Mining {

      /// A collection of domain-properties.
      ///
      /// Each one if allocated by the domain-property factory and saved in a 
      /// mapping from the DAL::property to the DomainProperty.
      class DomainPropertyCollection {

         /// \typedef PropertyToDomainProperty
         /// A mapping from a DAL::property to a domain-property.
         typedef std::map < const Interface::Property*, DomainProperty* > PropertyToDomainProperty;

      public :

         DomainPropertyCollection ( Interface::ProjectHandle* projectHandle );

         virtual ~DomainPropertyCollection ();

//          /// Set the factory for the domain-properties.
//          ///
//          /// The client application remains responsible for finalisation 
//          /// and destruction of the property-factory.
//          static void setFactory ( DomainPropertyFactory* factory );

//          /// Set the project-handle.
//          ///
//          /// The client application remains responsible for finalisation 
//          /// and destruction of the project-handle.
//          static void setProjectHandle ( Interface::ProjectHandle* handle );


         /// Return a pointer to the project-handle.
         Interface::ProjectHandle* getProjectHandle () const;

         /// Return a constant pointer to the current snapshot.
         const Interface::Snapshot* getSnapshot () const;


         /// Deletes all properties that have been collected.
         virtual void clear ();

         /// Set the snapshot.
         ///
         /// Deletes all previously collected data.
         /// All properties allocated from this point forward will be from this snapshot time.
         /// If the snapshot to be set is the same as that aslready set then nothing will change.
         virtual void setSnapshot ( const Interface::Snapshot* snapshot );

         /// Retrieve a domain-property from the collection.
         ///
         /// The first time a particular property is selected it will
         /// be allocated by the factory and saved in the collection.
         virtual DomainProperty* getDomainProperty ( const std::string& propertyName ) const;

         /// Retrieve a domain-property from the collection.
         ///
         /// The first time a particular property is selected it will
         /// be allocated by the factory and saved in the collection.
         virtual DomainProperty* getDomainProperty ( const Interface::Property* property ) const;

         /// \brief Initialises all the domain-properties that are in the collection.
         void initialiseProperties ();

         /// Determine if a property is a part of the collection.
         virtual bool contains ( const Interface::Property* property ) const;


      private :

         /// The project-handle from which all properties are retrieved.
         Interface::ProjectHandle* m_projectHandle;

         /// The factory that allocates all domain-properties.
         Mining::DomainPropertyFactory* m_propertyFactory;

         /// The snapshot at which all properties are retrieved.
         const Interface::Snapshot* m_snapshot;

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
