#include "ProjectHandle.h"

#include "DomainPropertyFactory.h"

DataAccess::Mining::ProjectHandle::ProjectHandle (database::Database * database, const std::string & name, const std::string & accessMode) :
   Interface::ProjectHandle ( database, name, accessMode ) {

   m_domainPropertyCollection = ((Mining::DomainPropertyFactory*)(getFactory()))->produceDomainPropertyCollection ( this );

}

DataAccess::Mining::DomainPropertyCollection* DataAccess::Mining::ProjectHandle::getDomainPropertyCollection () {
   return m_domainPropertyCollection;
}
