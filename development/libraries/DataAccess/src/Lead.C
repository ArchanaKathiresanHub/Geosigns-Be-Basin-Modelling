#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include <string>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/Lead.h"
#include "Interface/LeadTrap.h"
#include "Interface/ObjectFactory.h"

using namespace DataAccess;
using namespace Interface;

Lead::Lead (ProjectHandle * projectHandle, unsigned int id) : DAObject (projectHandle, 0), m_id (id)
{
}

Lead::~Lead (void)
{
   // delete the LeadTraps
   MutableLeadTrapList::const_iterator leadTrapIter;

   for (leadTrapIter = m_leadTraps.begin (); leadTrapIter != m_leadTraps.end (); ++leadTrapIter)
   {
      LeadTrap * leadTrap = * leadTrapIter;
      delete leadTrap;
   }
   m_leadTraps.clear ();
}

unsigned int Lead::getId (void) const
{
   return m_id;
}

LeadTrap * Lead::findLeadTrap (Reservoir * reservoir, unsigned int trapId) const
{
   // find a LeadTrap
   MutableLeadTrapList::const_iterator leadTrapIter;

   for (leadTrapIter = m_leadTraps.begin (); leadTrapIter != m_leadTraps.end (); ++leadTrapIter)
   {
      LeadTrap * leadTrap = * leadTrapIter;

      if (leadTrap->matchesConditions ((Reservoir *) reservoir, trapId))
	 return leadTrap;
   }
   return 0;
}

LeadTrap * Lead::createLeadTrap (Trap * trap)
{
   LeadTrap * leadTrap = getFactory ()->produceLeadTrap ((ProjectHandle *) getProjectHandle (), trap);
   m_leadTraps.push_back (leadTrap);

   return leadTrap;
}

LeadTrapList * Lead::getLeadTraps (void) const
{
   LeadTrapList * leadTrapList = new LeadTrapList;

   MutableLeadTrapList::const_iterator leadTrapIter;

   for (leadTrapIter = m_leadTraps.begin (); leadTrapIter != m_leadTraps.end (); ++leadTrapIter)
   {
      LeadTrap * leadTrap = * leadTrapIter;
      leadTrapList->push_back (leadTrap);
   }
   return leadTrapList;
}

void Lead::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void Lead::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "Lead:";
   buf << " id = " << getId ();
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
