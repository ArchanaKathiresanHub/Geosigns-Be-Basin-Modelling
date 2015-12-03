//#include <values.h>
#include <limits.h>
#include <float.h>

#include <vector>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi

using namespace std;

#include "database.h"
using namespace database;

#include "Interface/Grid.h"
using DataAccess::Interface::Grid;

#include "PersistentTrap.h"

#include "Trap.h"

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
	 cerr << "trap " << trap->getId () << " penetrates AND is penetrated by persistent trap " << getId () << endl;
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
void PersistentTrap::findFittingTrap (map < unsigned int, Trap *, less < unsigned int > > traps)
{
   Trap * mostOverlappingTrap = 0;
   int largestOverlap = 0;

   map < unsigned int, Trap *, less < unsigned int > >::iterator trapIter;
   for (trapIter = traps.begin (); trapIter != traps.end (); ++trapIter)
   {
      Trap * trap = (* trapIter).second;

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

void PersistentTrap::addTrap (Trap * trap)
{
   m_traps.push_back (trap);
   trap->setPersistentTrap (this);
}

int PersistentTrap::computeOverlap (Trap * trap)
{
   return getLastTrap ()->computeOverlap (trap);
}


Trap * PersistentTrap::getLastTrap (void)
{
   return m_traps.back ();
}

const Interface::Snapshot * PersistentTrap::getSnapshotOfLastTrap (void)
{
   return getLastTrap ()->getSnapshot ();
}

bool PersistentTrap::penetrates (Trap * trap)
{
   unsigned int iPT, jPT;

   getGridPosition (iPT, jPT);
   return trap->contains (iPT, jPT);
}

bool PersistentTrap::isPenetratedBy (Trap * trap)
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
   vector<Trap *>::iterator trapIter;
   for (trapIter = m_traps.begin (); trapIter != m_traps.end (); ++trapIter)
   {
      Trap * trap = * trapIter;
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

