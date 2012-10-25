#include <values.h>

#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi

#include <vector>

using namespace std;

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

using namespace DataAccess;
using Interface::GridMap;
using Interface::Grid;

#include "Reservoir.h"
#include "Trap.h"
#include "PersistentTrap.h"
using namespace PersistentTraps;

Reservoir::Reservoir (Interface::ProjectHandle * projectHandle, database::Record * record)
   : m_persistentTrapId (0), m_averageDepth (0), Interface::Reservoir (projectHandle, record)
{
}

Reservoir::~Reservoir (void)
{
}

bool Reservoir::extractRelevantTraps (const Interface::Snapshot * snapshot)
{
   m_traps.clear ();

   Interface::TrapList * traps =
      m_projectHandle->getTraps (this, snapshot, 0);

   Interface::TrapList::const_iterator trapIter;
   for (trapIter = traps->begin (); trapIter != traps->end (); ++trapIter)
   {
      Trap * trap = (Trap *) * trapIter;
      unsigned int id = trap->getId ();
      m_traps[id] = trap;
   }

   delete traps;
   return true;
}

bool Reservoir::isActive (const Interface::Snapshot * snapshot) const
{
   if (getActivityMode () == "NeverActive") return false; // not active anyway

   const Interface::Formation * formation = getFormation ();
   const Interface::Surface * topSurface = formation->getTopSurface ();
   const Interface::Snapshot * depoSnapshot = topSurface->getSnapshot ();

   if (depoSnapshot->getTime () <= snapshot->getTime ()) return false; // not active anyway

   else if (getActivityMode () == "AlwaysActive") return true;

   else if (getActivityMode () == "ActiveFrom")
   {
      const Interface::Snapshot * activeFromSnapshot = m_projectHandle->findSnapshot (getActivityStart ());
      return (activeFromSnapshot->getTime () > snapshot->getTime ());
   }
}

const GridMap * Reservoir::getPropertyGridMap (const string & propertyName, const Interface::Snapshot * snapshot)
{
   Interface::PropertyValueList * propertyValues =
      m_projectHandle->getPropertyValues (Interface::RESERVOIR,
	    m_projectHandle->findProperty (propertyName),
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

bool Reservoir::determineTrapSealPermeabilities (const Interface::Snapshot * snapshot)
{
   const GridMap * gridMap = getPropertyGridMap ("ResRockSealPermeability", snapshot);

   if (!gridMap) return false;

   map < unsigned int, Trap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap *trap = (*trapIter).second;
      assert (trap);
      unsigned int i, j;
      trap->getGridPosition (i, j);

      double permeability = gridMap->getValue (i, j);
      trap->setSealPermeability (permeability);
   }

   return true;
}

bool Reservoir::determineTrapPorosities (const Interface::Snapshot * snapshot)
{
   const GridMap * gridMap = getPropertyGridMap ("ResRockPorosity", snapshot);

   if (!gridMap) return false;

   map < unsigned int, Trap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap *trap = (*trapIter).second;
      assert (trap);
      unsigned int i, j;
      trap->getGridPosition (i, j);

      double porosity = gridMap->getValue (i, j);
      trap->setPorosity (porosity);
   }

   return true;
}

bool Reservoir::determineTrapExtents (const Interface::Snapshot * snapshot)
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
	    Trap * trap = getTrap ((int) value);
	    if (trap)
	    {
	       trap->addExtent (i, j);
	    }
	 }
      }
   }
   return true;
}

bool Reservoir::determineAverageDepth (const Interface::Snapshot * snapshot)
{
   const GridMap * gridMap = getPropertyGridMap ("ResRockTop", snapshot);
   if (!gridMap) return false;

   double average = gridMap->getAverageValue ();
   setAverageDepth (average);

   return true;
}

Trap * Reservoir::getTrap (int id)
{
   map < unsigned int, Trap *, less < unsigned int > >::iterator trapIter;

   trapIter = m_traps.find (id);
   if (trapIter != m_traps.end ())
      return (*trapIter).second;
   else
      return 0;
}


void Reservoir::computePersistentTraps (const Interface::Snapshot * previousSnapshot)
{
   extendPersistentTraps (previousSnapshot);
   createPersistentTraps ();
}

/// Create persistent traps for traps that have not been assigned to persistent traps yet
void Reservoir::createPersistentTraps ()
{
   map < unsigned int, Trap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = (* trapIter).second;
      assert (trap);
      if (trap->getPersistentTrap () != 0) continue;

      PersistentTrap * persistentTrap = new PersistentTrap (m_projectHandle->getHighResolutionOutputGrid (), ++m_persistentTrapId);
      m_persistentTraps.push_back (persistentTrap);
      persistentTrap->addTrap (trap);
   }
}


/// Extend persistent traps with fitting traps
void Reservoir::extendPersistentTraps (const Interface::Snapshot * previousSnapshot)
{
   vector < PersistentTrap * >::iterator persistentTrapIter;

   for (persistentTrapIter = m_persistentTraps.begin (); persistentTrapIter != m_persistentTraps.end (); ++persistentTrapIter)
   {
      PersistentTrap *persistentTrap = *persistentTrapIter;

      // check if persistent trap is still alive
      if (persistentTrap->getSnapshotOfLastTrap () != previousSnapshot) continue;
      persistentTrap->findFittingTrap (m_traps);
   }
}

void Reservoir::savePersistentTraps (Table * trapperIoTbl)
{
   vector < PersistentTrap * >::iterator persistentTrapIter;

   for (persistentTrapIter = m_persistentTraps.begin (); persistentTrapIter != m_persistentTraps.end (); ++persistentTrapIter)
   {
      PersistentTrap *persistentTrap = *persistentTrapIter;

      persistentTrap->save (trapperIoTbl);
   }
}

void Reservoir::deletePersistentTraps (void)
{
   vector < PersistentTrap * >::iterator persistentTrapIter;

   for (persistentTrapIter = m_persistentTraps.begin (); persistentTrapIter != m_persistentTraps.end (); ++persistentTrapIter)
   {
      PersistentTrap *persistentTrap = *persistentTrapIter;
      delete persistentTrap;
   }
   m_persistentTraps.clear ();
}

#ifdef OOD
void Reservoir::print (ostream & ostr)
{
   ostr << "Reservoir: " << getName ()
      << " (" << getFormationName () << "), # traps = " << m_traps.size () << endl;

   map < unsigned int, Trap *, less < unsigned int > >::iterator trapIter;

   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = (* trapIter).second;
      assert (trap);
      ostr << * trap;
   }
}

ostream & operator<< (ostream & ostr, Reservoir & reservoir)
{
   reservoir.print (ostr);
   return ostr;
}

void Reservoir::deleteTraps (void)
{
   m_traps.clear ();
}
#endif
