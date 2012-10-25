#ifndef _DATAMINING__PROJECT_HANDLE_H_
#define _DATAMINING__PROJECT_HANDLE_H_

#include <string>

#include "Interface/ProjectHandle.h"
#include "database.h"

#include "DomainPropertyCollection.h"

namespace DataAccess {

   namespace Mining {

      class ProjectHandle : public Interface::ProjectHandle {

      public :

         ProjectHandle (database::Database * database, const std::string & name, const std::string & accessMode);

         DomainPropertyCollection* getDomainPropertyCollection ();

      protected :

         DomainPropertyCollection* m_domainPropertyCollection;

      };

   }

}


#endif // _DATAMINING__PROJECT_HANDLE_H_
