#include <cassert>
#include <iostream>

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include "ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Reservoir.h"
#include "Trap.h"
#include "PersistentTrap.h"

#include "Interface/Snapshot.h"

using namespace database;
using namespace DataAccess;
using namespace PersistentTraps;
using namespace std;

static bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2);

ProjectHandle::ProjectHandle (database::Database * database,
      const string & name, const string & accessMode, DataAccess::Interface::ObjectFactory* factory)
   : Interface::ProjectHandle (database, name, accessMode, factory)
{
}

ProjectHandle::~ProjectHandle (void)
{
}

bool ProjectHandle::createPersistentTraps (void)
{
   Interface::SnapshotList * snapshots = getSnapshots (MAJOR);

   Interface::SnapshotList::const_iterator snapshotIter;

   const Interface::Snapshot * previousSnapshot = 0;
   
   for (snapshotIter = snapshots->begin (); snapshotIter != snapshots->end (); ++snapshotIter)
   {
      const Interface::Snapshot * snapshot = * snapshotIter;
      cerr << "Snapshot: " << snapshot->getTime () << endl;
      bool result = extractRelevantTraps (snapshot);
      if (!result)
      {
	 cerr << "Error in extracting the traps" << endl;
	 return false;
      }

      if (result) result = determineTrapExtents (snapshot);
      if (result) result = determineTrapPorosities (snapshot);
      if (result) result = determineReservoirDepths (snapshot);

      if (!result)
      {
	 cerr << "Could not find one or more maps, output is incomplete" << endl;
	 return false;
      }

      // Compute the PersistentTraps
      computePersistentTraps (snapshot, previousSnapshot);
      previousSnapshot = snapshot;
   }

   savePersistentTraps ();
   deletePersistentTraps ();

   return true;
}

bool ProjectHandle::extractRelevantTraps (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return false;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;

      if (!reservoir->extractRelevantTraps (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}


bool ProjectHandle::determineTrapExtents (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return false;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;
      if ( reservoir->getActivityMode() == "ActiveFrom" && snapshot->getTime()>reservoir->getActivityStart()) continue;
      if (!reservoir->determineTrapExtents (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool ProjectHandle::determineTrapSealPermeabilities (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return false;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;

      if (!reservoir->determineTrapSealPermeabilities (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool ProjectHandle::determineTrapPorosities (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return false;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;
      if ( reservoir->getActivityMode() == "ActiveFrom" && snapshot->getTime()>reservoir->getActivityStart()) continue;
      if (!reservoir->determineTrapPorosities (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool ProjectHandle::determineReservoirDepths (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return false;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;
      if ( reservoir->getActivityMode() == "ActiveFrom" && snapshot->getTime()>reservoir->getActivityStart()) continue;
      if (!reservoir->determineAverageDepth (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool ProjectHandle::computePersistentTraps (const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot)
{
   bool result = true;

   Interface::ReservoirList * reservoirs = getReservoirs (0);

   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return false;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;

      reservoir->computePersistentTraps (previousSnapshot);
   }
   delete reservoirs;
   return result;
}

void ProjectHandle::savePersistentTraps (void)
{
   Table *trapperIoTbl = getTable ("TrapperIoTbl");

   if (!trapperIoTbl) return;

   trapperIoTbl->clear ();

   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      cerr << "Saving persistent traps of reservoir " << reservoir->getName () << endl;

      reservoir->savePersistentTraps (trapperIoTbl);
   }
   delete reservoirs;
}

void ProjectHandle::deletePersistentTraps (void)
{
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;

      reservoir->deletePersistentTraps ();
   }
   delete reservoirs;
}

void ProjectHandle::saveProject (const string & fileName)
{
   Database * db = getDataBase ();
   if (db) db->saveToFile (fileName);
}


/// Used in the sorting of the Reservoirs, lowest Reservoir first
bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2)
{
#if DEBUG
   cerr << "Depth (" << reservoir1->getName () << ") = " << ((PersistentTraps::Reservoir *) reservoir1)->getAverageDepth ();
   cerr << "\tDepth (" << reservoir2->getName () << ") = " << ((PersistentTraps::Reservoir *) reservoir2)->getAverageDepth ();
   cerr << endl;
#endif
   
   return ((PersistentTraps::Reservoir *) reservoir1)->getAverageDepth () > ((PersistentTraps::Reservoir *) reservoir2)->getAverageDepth ();
}

Interface::Formation * ProjectHandle::getBottomSourceRockFormation ()
{
   Interface::FormationList * formations = getFormations ();
   Interface::FormationList::iterator formationIter;

   Interface::Formation * bottomSourceRockFormation = 0;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Interface::Formation * formation = const_cast<Interface::Formation *> (*formationIter);

      if (formation->isSourceRock ()) bottomSourceRockFormation = formation;
   }

   delete formations;

   return bottomSourceRockFormation;
}

bool ProjectHandle::isDeposited (const Interface::Formation * formation, const Interface::Snapshot * snapshot)
{
   const Interface::Surface * bottomSurface = formation->getBottomSurface ();
   const Interface::Snapshot * bottomSnapshot = bottomSurface->getSnapshot ();

   if (bottomSnapshot->getTime () > snapshot->getTime ())
      return true;
   else
      return false;
}

