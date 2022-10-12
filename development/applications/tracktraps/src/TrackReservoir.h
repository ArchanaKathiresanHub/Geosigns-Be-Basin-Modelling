#ifndef _PERSISTENTTRAPS_RESERVOIR_H_
#define _PERSISTENTTRAPS_RESERVOIR_H_

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

namespace database
{
   class Record;
   class Table;
}

#include "Reservoir.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
   namespace Interface
   {
      class GridMap;
      class Snapshot;
   }
}

using namespace DataAccess;


/// Reservoir Class
namespace PersistentTraps
{
   class TrackTrap;
   class PersistentTrap;

   class TrackReservoir : public Interface::Reservoir
   {
      public:
   /// Constructor
   TrackReservoir (Interface::ProjectHandle& projectHandle, database::Record * record);

   /// Destructor
   ~TrackReservoir (void);

   bool extractRelevantTraps (const Interface::Snapshot * snapshot);

	 /// Check whether the Reservoir is active at the specified snapshot
	 bool isActive (const Interface::Snapshot * snapshot) const;

	 TrackTrap * getTrap (int id);

	 const Interface::GridMap * getPropertyGridMap (const std::string & propertyName, const Interface::Snapshot * snapshot);

	 bool determineTrapSealPermeabilities (const Interface::Snapshot * snapshot);
	 bool determineTrapPorosities (const Interface::Snapshot * snapshot);

	 /// Calculate the coverages of all the Reservoir's Traps
	 bool determineTrapExtents (const Interface::Snapshot * snapshot);

	 bool determineAverageDepth (const Interface::Snapshot * snapshot);

	 /// Extend the given PersistentTraps with Traps penetrated by them and
	 /// create new PersistentTraps for Traps not penetrated by them.
	 void computePersistentTraps (const Interface::Snapshot * previousSnapshot);
	 void createPersistentTraps ();

	 void extendPersistentTraps (const Interface::Snapshot * previousSnapshot);
	 void savePersistentTraps (database::Table * trapperIoTbl);

	 void deletePersistentTraps (void);

	 inline bool hasName (const std::string & name);

	 inline void setAverageDepth (double depth);
	 inline double getAverageDepth (void);

private:
	std::vector < PersistentTrap * > m_persistentTraps;

   /// The Traps we work on
   std::map < unsigned int, TrackTrap *, std::less < unsigned int > > m_traps;

	 /// the persistent trap id's for this reservoir.
	 int m_persistentTrapId;


	 /// Average Depth of the Reservoir, used for sorting purposes.
	 double m_averageDepth;
	 };
}

bool PersistentTraps::TrackReservoir::hasName (const std::string & name)
{
	 return getName () == name;
}

void PersistentTraps::TrackReservoir::setAverageDepth (double depth)
{
	 m_averageDepth = depth;
}

double PersistentTraps::TrackReservoir::getAverageDepth (void)
{
	 return m_averageDepth;
}

#endif // _PERSISTENTTRAPS_RESERVOIR_H_

