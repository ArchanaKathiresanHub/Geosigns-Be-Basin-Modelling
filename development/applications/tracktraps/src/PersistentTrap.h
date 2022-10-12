#ifndef _PERSISTENTTRAPS_PERSISTENTTRAP_H_
#define _PERSISTENTTRAPS_PERSISTENTTRAP_H_

#include <iostream>

#include <vector>
#include <map>

namespace DataAccess
{
   namespace Interface
   {
      class Grid;
      class Snapshot;
   }
}

using namespace DataAccess;

namespace database
{
   class Table;
}

/// A PersistentTrap contains a collection of the Traps that are penetrated by it.
/// A PersistentTrap contains atmost one Trap per Reservoir.
namespace PersistentTraps
{
   class TrackReservoir;
   class TrackTrap;

   class PersistentTrap
   {
      public:
	 /// Constructor
	 PersistentTrap (const DataAccess::Interface::Grid * grid, int PersistentTrapId);

	 ~PersistentTrap (void);

   void findFittingTrap (std::map < unsigned int, TrackTrap *, std::less < unsigned int > > traps);

	 /// Add Trap to this PersistentTrap
   void addTrap (TrackTrap * trap);

   int computeOverlap (TrackTrap * trap);

	 /// Get the trap that was last added
   TrackTrap * getLastTrap (void);

	 /// Get the snapshot of the trap that was last added
	 const Interface::Snapshot * getSnapshotOfLastTrap (void);

	 /// Check if PersistentTrap  penetrates the Trap
   bool penetrates (TrackTrap * trap);

	 /// Check if PersistentTrap is penetrated by the Trap
   bool isPenetratedBy (TrackTrap * trap);

	 double getDistance (unsigned int i, unsigned int j);

	 /// Save this PersistentTrap to a PersistentTrapIoTbl table
	 void save (database::Table * table);

	 void getGridPosition (unsigned int & i, unsigned int & j);
	 inline int getId (void);

	 void print (std::ostream & ostr);

      private:
	 const DataAccess::Interface::Grid * m_grid;

	 /// the id of a PersistentTrap
	 int m_id;

	 /// The Traps of a PersistentTrap
   std::vector < TrackTrap * > m_traps;
   };
}

std::ostream & operator<< (std::ostream &, PersistentTraps::PersistentTrap &);

int PersistentTraps::PersistentTrap::getId (void)
{
   return m_id;
}

#endif // _PERSISTENTTRAPS_PERSISTENTTRAP_H_


