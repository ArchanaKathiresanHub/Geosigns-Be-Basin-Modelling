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

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include <string>
using namespace std;

#include "Interface/Interface.h"
#include "Interface/LeadTrap.h"
#include "Interface/Trap.h"
#include "Interface/TrapPhase.h"
#include "Interface/Reservoir.h"
#include "Interface/ObjectFactory.h"

using namespace DataAccess;
using namespace Interface;

LeadTrap::LeadTrap (ProjectHandle * projectHandle, Trap * trap) :
   DAObject (projectHandle, 0), m_trap (trap)
{
   m_trapPhases[Gas] = 0;
   m_trapPhases[Oil] = 0;
}


LeadTrap::~LeadTrap (void)
{
   if (m_trapPhases[Gas]) delete m_trapPhases[Gas];
   if (m_trapPhases[Oil]) delete m_trapPhases[Oil];
}

void LeadTrap::createTrapPhase (database::Record * record)
{
   TrapPhase * trapPhase = getFactory ()->produceTrapPhase (m_projectHandle, record);
   if (database::getReservoirPhase (record) == "Gas")
   {
      m_trapPhases[Gas] = trapPhase;
   }
   else
   {
      m_trapPhases[Oil] = trapPhase;
   }
}

bool LeadTrap::matchesConditions (const Reservoir * reservoir, unsigned int id)
{
   if (reservoir != 0 && getReservoir () != (const Reservoir *) reservoir) return false;
   if (id != 0 && getId () != id) return false;
   return true;
}

unsigned int LeadTrap::getId (void) const
{
   return m_trap->getId ();
}

const Reservoir * LeadTrap::getReservoir (void) const
{
   return m_trap->getReservoir ();
}

const Snapshot * LeadTrap::getSnapshot (void) const
{
   return m_trap->getSnapshot ();
}

/// Return the real world position of this Trap
void LeadTrap::getPosition (double & x, double & y) const
{
   m_trap->getPosition (x, y);
}

/// Return the grid position of this Trap
void LeadTrap::getGridPosition (unsigned int & i, unsigned int & j) const
{
   m_trap->getGridPosition (i, j);
}

/// return the mass of the given component in this Trap
double LeadTrap::getMass (ComponentId componentId) const
{
   return m_trap->getMass (componentId);
}

/// return the Trap's depth
double LeadTrap::getDepth (void) const
{
   return m_trap->getDepth ();
}

/// return the Trap's temperature
double LeadTrap::getTemperature (void) const
{
   return m_trap->getTemperature ();
}

/// return the Trap's pressure
double LeadTrap::getPressure (void) const
{
   return m_trap->getPressure ();
}

/// return the Trap's gas-oil contact depth
double LeadTrap::getGOC (void) const
{
   return m_trap->getGOC ();
}

/// return the Trap's oil-water contact depth
double LeadTrap::getOWC (void) const
{
   return m_trap->getOWC ();
}

/// return the Trap's surface at the oil-water contact depth
double LeadTrap::getWCSurface (void) const
{
   return m_trap->getWCSurface ();
}

/// return the LeadTrap's porosity
double LeadTrap::getPorosity (void) const
{
   if (m_trapPhases[Gas])
   {
      return m_trapPhases[Gas]->getPorosity ();
   }
   else if (m_trapPhases[Oil])
   {
      return m_trapPhases[Oil]->getPorosity ();
   }
   else
   {
      return -1;
   }
}


/// return the LeadTrap's seal permeability
double LeadTrap::getSealPermeability (void) const
{
   if (m_trapPhases[Gas])
   {
      return m_trapPhases[Gas]->getSealPermeability ();
   }
   else if (m_trapPhases[Oil])
   {
      return m_trapPhases[Oil]->getSealPermeability ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getGasWetness (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getGasWetness ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getN2GasRatio (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getN2GasRatio ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getAPIGravity (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getAPIGravity ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getCGR (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getCGR ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getFormVolFactor (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getFormVolFactor ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getGasMass (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getGasMass ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getOilMass (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getOilMass ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getGasDensity (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getGasDensity ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getOilDensity (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getOilDensity ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getGasViscosity (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getGasViscosity ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getOilViscosity (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getOilViscosity ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getGasVolume (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getGasVolume ();
   }
   else
   {
      return -1;
   }
}

double LeadTrap::getOilVolume (PhaseId phaseId) const
{
   if (m_trapPhases[phaseId])
   {
      return m_trapPhases[phaseId]->getOilVolume ();
   }
   else
   {
      return -1;
   }
}

void LeadTrap::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void LeadTrap::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "LeadTrap:";
   buf << " id = " << m_trap->getId ();
   buf << ", reservoir = " << getReservoir ()->getName ();
   double x, y;
   getPosition (x, y);
   buf << ", position = (" << x << ", " << y << ")";

   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
