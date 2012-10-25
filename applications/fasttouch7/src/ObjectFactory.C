#include "FastTouch.h"
#include "ObjectFactory.h"

#include "database.h"

using namespace fasttouch;

DataAccess::Interface::ProjectHandle *
ObjectFactory::produceProjectHandle (database::Database * database,
	 const string & name, const string & accessMode)
{
   return new FastTouch (database, name, accessMode);
}

