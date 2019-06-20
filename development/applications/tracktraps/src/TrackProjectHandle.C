//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <cassert>
#include <iostream>

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include "TrackProjectHandle.h"
#include "Formation.h"
#include "Surface.h"
#include "TrackReservoir.h"
#include "TrackTrap.h"
#include "PersistentTrap.h"

#include "Snapshot.h"

using namespace database;
using namespace DataAccess;
using namespace PersistentTraps;
using namespace std;

static bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2);

TrackProjectHandle::TrackProjectHandle (database::ProjectFileHandlerPtr database,
                              const string & name,
                              const string & accessMode,
                              const Interface::ObjectFactory* factory)
   : Interface::ProjectHandle (database, name, accessMode, factory)
{
}

TrackProjectHandle::~TrackProjectHandle (void)
{
}

bool TrackProjectHandle::createPersistentTraps (void)
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
    delete snapshots;
   return false;
      }

      if (result) result = determineTrapExtents (snapshot);
      if (result) result = determineTrapPorosities (snapshot);
      if (result) result = determineReservoirDepths (snapshot);

      if (!result)
      {
   cerr << "Could not find one or more maps, output is incomplete" << endl;
    delete snapshots;
   return false;
      }

      // Compute the PersistentTraps
      computePersistentTraps (snapshot, previousSnapshot);
      previousSnapshot = snapshot;
   }

   savePersistentTraps ();
   deletePersistentTraps ();
   delete snapshots;
   return true;
}

bool TrackProjectHandle::extractRelevantTraps (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return true;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;

      if (!reservoir->extractRelevantTraps (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}


bool TrackProjectHandle::determineTrapExtents (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return true;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;
      if ( reservoir->getActivityMode() == "ActiveFrom" && snapshot->getTime()>reservoir->getActivityStart()) continue;
      if (!reservoir->determineTrapExtents (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool TrackProjectHandle::determineTrapSealPermeabilities (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return true;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;

      if (!reservoir->determineTrapSealPermeabilities (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool TrackProjectHandle::determineTrapPorosities (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return true;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;
      if ( reservoir->getActivityMode() == "ActiveFrom" && snapshot->getTime()>reservoir->getActivityStart()) continue;
      if (!reservoir->determineTrapPorosities (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool TrackProjectHandle::determineReservoirDepths (const Interface::Snapshot * snapshot)
{
   bool result = true;
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return true;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
     TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;
      if ( reservoir->getActivityMode() == "ActiveFrom" && snapshot->getTime()>reservoir->getActivityStart()) continue;
      if (!reservoir->determineAverageDepth (snapshot)) result = false;
   }
   delete reservoirs;
   return result;
}

bool TrackProjectHandle::computePersistentTraps (const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot)
{
   bool result = true;

   Interface::ReservoirList * reservoirs = getReservoirs (0);

   Interface::ReservoirList::const_iterator reservoirIter;

   const Interface::Formation* srFormation = getBottomSourceRockFormation ();
   if (srFormation == nullptr)
      return true;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
     TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      if (!reservoir->isActive (snapshot) or !isDeposited (srFormation, snapshot)) continue;

      reservoir->computePersistentTraps (previousSnapshot);
   }
   delete reservoirs;
   return result;
}

void TrackProjectHandle::savePersistentTraps (void)
{
   Table *trapperIoTbl = getTable ("TrapperIoTbl");

   if (!trapperIoTbl) return;

   trapperIoTbl->clear ();

   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
     TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      cerr << "Saving persistent traps of reservoir " << reservoir->getName () << endl;

      reservoir->savePersistentTraps (trapperIoTbl);
   }
   delete reservoirs;
}

void TrackProjectHandle::deletePersistentTraps (void)
{
   Interface::ReservoirList * reservoirs = getReservoirs (0);
   Interface::ReservoirList::const_iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
     TrackReservoir * reservoir = (TrackReservoir *) * reservoirIter;

      reservoir->deletePersistentTraps ();
   }
   delete reservoirs;
}

void TrackProjectHandle::saveProject (const string & fileName)
{

   if ( getProjectFileHandler () != nullptr ) {
      getProjectFileHandler ()->saveToFile (fileName);
   }

}


/// Used in the sorting of the Reservoirs, lowest Reservoir first
bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2)
{
#if DEBUG
   cerr << "Depth (" << reservoir1->getName () << ") = " << ((PersistentTraps::Reservoir *) reservoir1)->getAverageDepth ();
   cerr << "\tDepth (" << reservoir2->getName () << ") = " << ((PersistentTraps::Reservoir *) reservoir2)->getAverageDepth ();
   cerr << endl;
#endif

   return ((PersistentTraps::TrackReservoir *) reservoir1)->getAverageDepth () > ((PersistentTraps::TrackReservoir *) reservoir2)->getAverageDepth ();
}

Interface::Formation * TrackProjectHandle::getBottomSourceRockFormation ()
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

bool TrackProjectHandle::isDeposited (const Interface::Formation * formation, const Interface::Snapshot * snapshot)
{
   const Interface::Surface * bottomSurface = formation->getBottomSurface ();
   const Interface::Snapshot * bottomSnapshot = bottomSurface->getSnapshot ();

   if (bottomSnapshot->getTime () > snapshot->getTime ())
      return true;
   else
      return false;
}
