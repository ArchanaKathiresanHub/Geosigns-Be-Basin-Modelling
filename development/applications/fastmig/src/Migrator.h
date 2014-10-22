#ifndef _MIGRATION_MIGRATOR_H_
#define _MIGRATION_MIGRATOR_H_

#include "MassBalance.h"

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <fstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include <fstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <fstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include <string>
#include <vector>

using namespace std;

namespace database
{
   class Database;
   class Table;
   class Record;
}

#include "GeoPhysicsProjectHandle.h"
#include "MigrationPropertyManager.h"


using namespace DataAccess;

namespace migration
{
   class Reservoir;
   class Barrier;
   class TrapPropertiesRequest;
   class MigrationRequest;
#ifdef USEOTGC
   class OilToGasCracker;
#endif
   const string MigrationActivityName = "HighResMigration";


   /// Top class of the Migration class hierarchy.
   /// Inherits from the DataAccess::Migrator class to provide easy access to Formations, Reservoirs, etc.
   class Migrator : public GeoPhysics::ProjectHandle
   {
      public:
	 /// Constructor
	 Migrator (database::Database * database, const string & name, const string & accessMode);
	 static Migrator *CreateFrom (const string & inputFileName);
	 virtual ~Migrator (void);

	 bool saveTo (const string & outputFileName);

	 /// The top migration routine
	 bool compute (void);

	 /// Compute the offsets of each reservoir from the top and bottom of its formation
	 bool computeDepthOffsets (void);

	 /// Compute the net/gross fractions
	 bool computeNetToGross (void);

	 /// Check if there are any active reservoirs at given snapshot
	 bool activeReservoirs (const Interface::Snapshot * snapshot);

	 /// Charge the active reservoirs with charge already in the traps and
	 /// additionally expelled charge between the given snapshots.
	 bool chargeReservoirs (const Interface::Snapshot * start, const Interface::Snapshot * end);

	 /// Charge the specified reservoir with charge already in the traps and
	 /// additionally expelled charge between the given snapshots.
	 bool chargeReservoir (Reservoir * reservoir, Reservoir * reservoirAbove, Reservoir * reservoirBelow,
	       const Interface::Snapshot * start, const Interface::Snapshot * end);

	 /// Collect expelled charges into the given reservoir from the appropriate source rocks.
	 bool collectAndMigrateExpelledCharges (Reservoir * reservoir, Reservoir * reservoirAbove, Reservoir * reservoirBelow,
	       const Interface::Snapshot * start, const Interface::Snapshot * end, Barrier * barrier);

	 /// retrieve the reservoirs, for the specified formation if specified.
	 virtual DataAccess::Interface::ReservoirList * getReservoirs (const Interface::Formation * formation = 0) const;

	 /// retrieve the complete list of formations
	 virtual DataAccess::Interface::FormationList * getAllFormations (void) const;

	 void addTrapRecord (Reservoir * reservoir, TrapPropertiesRequest & tpRequest);

	 database::Record * copyMigrationRecord (database::Record * oldRecord, const string & newMigrationProcess);

	 void addMigrationRecord (const string & srcReservoirName, const string & srcFormationName, 
	       const string & dstReservoirName,
	       MigrationRequest & mr);

	 database::Record * findMigrationRecord (const string & srcReservoirName, const string & srcFormationName, 
	       const string & dstReservoirName,
	       MigrationRequest & mr);

	 database::Record * createMigrationRecord (const string & srcReservoirName, const string & srcFormationName, 
	       const string & dstReservoirName,
	       MigrationRequest & mr);

	 void sortMigrationRecords (void);
	 void uniqueMigrationRecords (void);
	 void sanitizeMigrationRecords (void);
	 void checkMigrationRecords (void);
	 void clearMigrationRecordLists (void);
	 void renumberMigrationRecordTrap (const Interface::Snapshot * snapshot, int oldTrapNumber, int newTrapNumber);

	 void deleteExpelledChargeMaps (const Interface::Snapshot * snapshot);

	 const Interface::GridMap * getPropertyGridMap (const string & propertyName,
                                                        const Interface::Snapshot * snapshot,
                                                        const Interface::Reservoir * reservoir,
                                                        const Interface::Formation * formation,
                                                        const Interface::Surface * surface) const;


         MigrationPropertyManager& getPropertyManager ();


      private:
	 mutable DataAccess::Interface::FormationList * m_formations;
	 mutable DataAccess::Interface::ReservoirList * m_reservoirs;
         ofstream m_massBalanceFile;
	 MassBalance<ofstream>* m_massBalance;

	 database::Table * m_trapIoTbl;
	 database::Table * m_migrationIoTbl;

      bool mergeOutputFiles ();

#ifdef USEOTGC
	 OilToGasCracker * m_otgc;
#endif

	 vector<database::Record *> * m_migrationRecordLists;

      MigrationPropertyManager m_propertyManager;

   };
}

inline migration::MigrationPropertyManager& migration::Migrator::getPropertyManager () {
   return m_propertyManager;
}


#endif // _MIGRATION_MIGRATOR_H
