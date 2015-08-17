#ifndef _PERSISTENTTRAPS_PROJECTHANDLE_H_
#define _PERSISTENTTRAPS_PROJECTHANDLE_H_

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi


#include <string>
#include <map>
#include <vector>

using namespace std;

namespace database
{
   class Database;
}

#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class Snapshot;
   }
}

using namespace DataAccess;


/// ProjectHandle Class
namespace PersistentTraps
{
   class ProjectHandle : public Interface::ProjectHandle
   {
      public:
	 /// Constructor
	 ProjectHandle (database::Database * database, const string & name, const string & accessMode, DataAccess::Interface::ObjectFactory* factory);

	 /// Destructor
	 ~ProjectHandle (void);

	 bool migrationCompleted (void);

	 bool extractRelevantTraps (const Interface::Snapshot * snapshot);
	 /// load the surfaces occupied for each of the traps
	 bool determineTrapExtents (const Interface::Snapshot * snapshot);

	 bool determineTrapSealPermeabilities (const Interface::Snapshot * snapshot);
	 bool determineTrapPorosities (const Interface::Snapshot * snapshot);

	 /// determine the average depth of each of the reservoirs
	 bool determineReservoirDepths (const Interface::Snapshot * snapshot);

	 /// Create the persistentTraps, this functions calls all the others 
	 bool createPersistentTraps (void);
	 /// Compute the PersistentTraps for this model
	 bool computePersistentTraps (const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot);

	 /// store the persistentTraps in their database table.
	 void savePersistentTraps (void);

	 void deletePersistentTraps (void);

	 void saveProject (const string & fileName);

      private:

   };
}

#endif // _PERSISTENTTRAPS_PROJECTHANDLE_H_

