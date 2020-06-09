//#include <values.h>
#include <limits.h>
#include <float.h>

#include <assert.h>

#include <iostream>

#include <vector>

using namespace std;

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "ProjectHandle.h"
#include "PropertyValue.h"
#include "Formation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Grid.h"

using namespace DataAccess;
using Interface::GridMap;
using Interface::Grid;

#include "TrackReservoir.h"
#include "TrackTrap.h"
#include "PersistentTrap.h"
using namespace PersistentTraps;

TrackReservoir::TrackReservoir (Interface::ProjectHandle& projectHandle, database::Record * record)
   : m_persistentTrapId (0), m_averageDepth (0), Interface::Reservoir (projectHandle, record)
{
}

TrackReservoir::~TrackReservoir (void)
{
}

bool TrackReservoir::extractRelevantTraps (const Interface::Snapshot * snapshot)
{
   m_traps.clear ();

   Interface::TrapList * traps =
      getProjectHandle().getTraps (this, snapshot, 0);

   Interface::TrapList::const_iterator trapIter;
   for (trapIter = traps->begin (); trapIter != traps->end (); ++trapIter)
   {
      TrackTrap * trap = (TrackTrap *) * trapIter;
      unsigned int id = trap->getId ();
      m_traps[id] = trap;
   }

   delete traps;
   return true;
}

bool TrackReservoir::isActive (const Interface::Snapshot * snapshot) const
{
   if (getActivityMode () == "NeverActive") return false; // not active anyway

   const Interface::Formation * formation = getFormation ();
   const Interface::Surface * topSurface = formation->getTopSurface ();
   const Interface::Snapshot * depoSnapshot = topSurface->getSnapshot ();

   if (depoSnapshot->getTime () <= snapshot->getTime ()) return false; // not active anyway

   else if (getActivityMode () == "AlwaysActive") return true;

   else if (getActivityMode () == "ActiveFrom")
   {
      const Interface::Snapshot * activeFromSnapshot = getProjectHandle().findSnapshot (getActivityStart ());
      return (activeFromSnapshot->getTime () > snapshot->getTime ());
   }

   return false; // fixing warning for no return
}

const GridMap * TrackReservoir::getPropertyGridMap (const string & propertyName, const Interface::Snapshot * snapshot)
{
   Interface::PropertyValueList * propertyValues =
      getProjectHandle().getPropertyValues (Interface::RESERVOIR,
      getProjectHandle().findProperty (propertyName),
      snapshot, this, 0, 0,
      Interface::SURFACE);

   if (propertyValues->size () != 1)
   {
      cerr << "Error: Wrong number ("
   << propertyValues->size ()
   << " != 1) of "
   << propertyName
   << " propertyValues returned!"
   << endl;
      return 0;
   }

   const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

   delete propertyValues;
   return gridMap;
}

bool TrackReservoir::determineTrapSealPermeabilities (const Interface::Snapshot * snapshot)
{
   const GridMap * gridMap = getPropertyGridMap ("ResRockSealPermeability", snapshot);

   if (!gridMap) return false;

   map < unsigned int, TrackTrap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      TrackTrap *trap = (*trapIter).second;
      assert (trap);
      unsigned int i, j;
      trap->getGridPosition (i, j);

      double permeability = gridMap->getValue (i, j);
      trap->setSealPermeability (permeability);
   }

   return true;
}

bool TrackReservoir::determineTrapPorosities (const Interface::Snapshot * snapshot)
{
   const GridMap * gridMap = getPropertyGridMap ("ResRockPorosity", snapshot);

   if (!gridMap) return false;

   map < unsigned int, TrackTrap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      TrackTrap *trap = (*trapIter).second;
      assert (trap);
      unsigned int i, j;
      trap->getGridPosition (i, j);

      double porosity = gridMap->getValue (i, j);
      trap->setPorosity (porosity);
   }

   return true;
}

bool TrackReservoir::determineTrapExtents (const Interface::Snapshot * snapshot)
{
   const GridMap * gridMap = getPropertyGridMap ("ResRockTrapId", snapshot);
   if (!gridMap) return false;

   const Grid * grid = gridMap->getGrid ();

   unsigned int numI = grid->numI ();
   unsigned int numJ = grid->numJ ();

   for (unsigned int i = 0; i < numI; i++)
   {
      for (unsigned int j = 0; j < numJ; j++)
      {
   double value = gridMap->getValue (i, j);
   if (value != gridMap->getUndefinedValue () && value > 0)
   {
      TrackTrap * trap = getTrap ((int) value);
      if (trap)
      {
         trap->addExtent (i, j);
      }
   }
      }
   }
   return true;
}

bool TrackReservoir::determineAverageDepth (const Interface::Snapshot * snapshot)
{
   const GridMap * gridMap = getPropertyGridMap ("ResRockTop", snapshot);
   if (!gridMap) return false;

   double average = gridMap->getAverageValue ();
   setAverageDepth (average);

   return true;
}

TrackTrap * TrackReservoir::getTrap (int id)
{
   map < unsigned int, TrackTrap *, less < unsigned int > >::iterator trapIter;

   trapIter = m_traps.find (id);
   if (trapIter != m_traps.end ())
      return (*trapIter).second;
   else
      return 0;
}


void TrackReservoir::computePersistentTraps (const Interface::Snapshot * previousSnapshot)
{
   extendPersistentTraps (previousSnapshot);
   createPersistentTraps ();
}

/// Create persistent traps for traps that have not been assigned to persistent traps yet
void TrackReservoir::createPersistentTraps ()
{
   map < unsigned int, TrackTrap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      TrackTrap * trap = (* trapIter).second;
      assert (trap);
      if (trap->getPersistentTrap () != 0) continue;

      PersistentTrap * persistentTrap = new PersistentTrap (getProjectHandle().getHighResolutionOutputGrid (), ++m_persistentTrapId);
      m_persistentTraps.push_back (persistentTrap);
      persistentTrap->addTrap (trap);
   }
}


/// Extend persistent traps with fitting traps
void TrackReservoir::extendPersistentTraps (const Interface::Snapshot * previousSnapshot)
{
   for ( PersistentTrap *persistentTrap : m_persistentTraps )
   {
      // check if persistent trap is still alive
      if (persistentTrap->getSnapshotOfLastTrap () != previousSnapshot) continue;
      persistentTrap->findFittingTrap (m_traps);
   }
}

void TrackReservoir::savePersistentTraps (Table * trapperIoTbl)
{
   for ( PersistentTrap *persistentTrap : m_persistentTraps )
   {
      persistentTrap->save (trapperIoTbl);
   }
}

void TrackReservoir::deletePersistentTraps (void)
{
   for ( PersistentTrap *persistentTrap : m_persistentTraps )
   {
      delete persistentTrap;
   }
   m_persistentTraps.clear ();
}

#ifdef OOD
void TrackReservoir::print (ostream & ostr)
{
   ostr << "Reservoir: " << getName ()
      << " (" << getFormationName () << "), # traps = " << m_traps.size () << endl;

   map < unsigned int, TrackTrap *, less < unsigned int > >::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      TrackTrap * trap = (* trapIter).second;
      assert (trap);
      ostr << * trap;
   }
}

ostream & operator<< (ostream & ostr, TrackReservoir & reservoir)
{
   reservoir.print (ostr);
   return ostr;
}

void TrackReservoir::deleteTraps (void)
{
   m_traps.clear ();
}
#endif
