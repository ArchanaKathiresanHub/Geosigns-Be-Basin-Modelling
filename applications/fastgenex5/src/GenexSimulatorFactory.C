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

#include "GenexSimulatorFactory.h"
#include "SourceRock.h"
#include "GenexSimulator.h"

using namespace GenexSimulation;

DataAccess::Interface::ProjectHandle *
GenexSimulatorFactory::produceProjectHandle (database::Database * database, const string & name, const string & accessMode)
	                     
{
   return new GenexSimulator (database, name, accessMode);
}

DataAccess::Interface::SourceRock *
GenexSimulatorFactory::produceSourceRock (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new SourceRock(projectHandle, record);
}


