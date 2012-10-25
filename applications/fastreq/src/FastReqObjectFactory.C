#ifdef sgi
  #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
   #define USESTANDARD
#endif // sgi


#include "database.h"

#include "FastReqObjectFactory.h"
#include "FastReqProjectHandle.h"

using namespace FastReq;

DataAccess::Interface::ProjectHandle *FastReqObjectFactory::produceProjectHandle (database::Database * database, const string & name, const string & accessMode)
{
   return new FastReqProjectHandle (database, name, accessMode);
}
