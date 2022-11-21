//#include <values.h>
#include <limits.h>
#include <float.h>
#include <vector>
#include <iostream>

using namespace std;

#include "database.h"
using namespace database;

#include "Grid.h"
using DataAccess::Interface::Grid;

#include "PersistentTrap.h"

#include "TrackTrap.h"

using namespace PersistentTraps;


PersistentTrap::PersistentTrap (const Grid * grid, int persistentTrapId) : m_grid (grid), m_id (persistentTrapId)
{
}

PersistentTrap::~PersistentTrap (void)
{
}

#ifdef PENETRATIONANDDISTANCEBASED
void PersistentTrap::findFittingTrap (map < unsigned int, Trap *, less < unsigned int > > traps)
{
   Trap * closestTrap = 0;
   double closestDistance = MAXDOUBLE;

   map < unsigned int, Trap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = traps.begin (); trapIter != traps.end (); ++trapIter)
   {
      Trap * trap = (* trapIter).second;

      if (trap->getPersistentTrap () != 0) continue;

      bool penetratesTrap = penetrates (trap);
      bool isPenetratedByTrap = isPenetratedBy (trap);

      if (isPenetratedByTrap && penetratesTrap)
      {
          LogHandler(LogHandler::INFO_SEVERITY) << "trap " << trap->getId () << " penetrates AND is penetrated by persistent trap " << getId () << endl;
	 closestTrap = trap;
	 break;
      }
      
      if (!isPenetratedByTrap && !penetratesTrap) continue;

      unsigned int i, j;
      trap->getGridPosition (i, j);

      double distance = getDistance (i, j);
      assert (distance >= 0);

      if (distance < closestDistance)
      {
	 closestDistance = distance;
	 closestTrap = trap;
      }
   }

   if (closestTrap) addTrap (closestTrap);
}
#else // overlap based
void PersistentTrap::findFittingTrap (map < unsigned int, TrackTrap *, less < unsigned int > > traps)
{
   TrackTrap * mostOverlappingTrap = 0;
   int largestOverlap = 0;

   map < unsigned int, TrackTrap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = traps.begin (); trapIter != traps.end (); ++trapIter)
   {
      TrackTrap * trap = (* trapIter).second;

      if (trap->getPersistentTrap () != 0) continue;

      int overlap = computeOverlap (trap);
      if (overlap > largestOverlap)
      {
	 largestOverlap = overlap;
	 mostOverlappingTrap = trap;
      }
   }

   if (mostOverlappingTrap) addTrap (mostOverlappingTrap);
}
#endif

void PersistentTrap::addTrap (TrackTrap * trap)
{
   m_traps.push_back (trap);
   trap->setPersistentTrap (this);
}

int PersistentTrap::computeOverlap (TrackTrap * trap)
{
   return getLastTrap ()->computeOverlap (trap);
}


TrackTrap * PersistentTrap::getLastTrap (void)
{
   return m_traps.back ();
}

const Interface::Snapshot * PersistentTrap::getSnapshotOfLastTrap (void)
{
   return getLastTrap ()->getSnapshot ();
}

bool PersistentTrap::penetrates (TrackTrap * trap)
{
   unsigned int iPT, jPT;

   getGridPosition (iPT, jPT);
   return trap->contains (iPT, jPT);
}

bool PersistentTrap::isPenetratedBy (TrackTrap * trap)
{
   unsigned int i, j;

   trap->getGridPosition (i, j);
   return getLastTrap ()->contains (i, j);
}

double PersistentTrap::getDistance (unsigned int i, unsigned int j)
{
   unsigned int iPT, jPT;

   getGridPosition ( iPT, jPT);
   return m_grid->getDistance (iPT, jPT, i, j);
}

void PersistentTrap::getGridPosition (unsigned int & i, unsigned int & j)
{
   return getLastTrap ()->getGridPosition (i, j);
}

void PersistentTrap::save (Table * table)
{
   for ( TrackTrap * trap : m_traps )
   {
      trap->save (table);
   }
}

void PersistentTrap::print (ostream & ostr)
{
   ostr << "PersistentTrap " << getId ();
}

ostream & operator<< (ostream & ostr, PersistentTrap & persistentTrap)
{
   persistentTrap.print (ostr);
   return ostr;
}

