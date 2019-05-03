#ifndef _DATAMINING__PROJECT_HANDLE_H_
#define _DATAMINING__PROJECT_HANDLE_H_

#include <ostream>
#include <string>
#include <map>

#include "Interface/Interface.h"

#include "GeoPhysicsProjectHandle.h"
#include "ProjectFileHandler.h"

#include "DomainPropertyCollection.h"
#include "CauldronDomain.h"

namespace DataAccess {

   namespace Mining {

      class ProjectHandle : public GeoPhysics::ProjectHandle {

         typedef std::map<std::string, std::string> StringMap;

      public :

         ProjectHandle (database::ProjectFileHandlerPtr pfh, const std::string & name, const std::string & accessMode, const DataAccess::Interface::ObjectFactory* objectFactory);

         /// \brief Get the domain-property collection.
         ///
         /// Should this be a singleton? Since only one will exist.
         DomainPropertyCollection * getDomainPropertyCollection() { return m_domainPropertyCollection; }

         CauldronDomain & getCauldronDomain() { return *m_cauldronDomain; }

         /// \brief Add an alternative name for the property name.
         void addAlternativeName ( const std::string& propertyName,
                                   const std::string& alternativeName );

         /// \brief Get property from a possible alternative.
         ///
         /// The alternative name can be identical to the property-name.
         /// If the alternative name has not been set then an empty string will be returned.
         const std::string& getPropertyNameFromAlternative ( const std::string& alternativeName ) const;

         /// \brief List all the know properties to the stream.
         void listProperties ( std::ostream& out ) const;

         /// \brief List all the saved properties to the stream.
         void listSavedProperties ( std::ostream& out ) const;

      protected :

         DomainPropertyCollection* m_domainPropertyCollection;
         CauldronDomain*           m_cauldronDomain;
         StringMap                 m_alternativeNamesMap;
         std::string               m_noName;

      private :

         /// \brief Add non-standard properties.
         void addNewProperties ();

         /// \brief A list of dereived properties.
         ///
         /// These may be properties like porosity, permeability as well as
         /// brine-viscosity, ...
         /// These properties can also be found on the properties list.
         Interface::MutablePropertyList m_derivedProperties;

      };

   }

}

#endif // _DATAMINING__PROJECT_HANDLE_H_
