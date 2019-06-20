//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// Table Io library
#include "database.h"
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
#include "Grid.h"
#include "ProjectHandle.h"
#include "Reservoir.h"
#include "Snapshot.h"
#include "Trap.h"
using namespace DataAccess;
using namespace Interface;

static const char ** ComponentNames = CBMGenerics::ComponentManager::getInstance().getSpeciesNameInputList();


const string PhaseNames[] =
{ 
   "Gas", "Oil"
};

Trap::Trap (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_reservoir = 0;
   m_snapshot = 0;
}

Trap::~Trap (void)
{
}

bool Trap::matchesConditions (const Reservoir * reservoir, const Snapshot * snapshot, unsigned int id)
{
   if (snapshot != 0 && getSnapshot () != snapshot) return false;
   if (reservoir != 0 && getReservoir () != reservoir) return false;
   if (id != 0 && getId () != id) return false;
   return true;
}

unsigned int Trap::getId (void) const
{
   return database::getTrapID (m_record);
}

void Trap::setReservoir (const Reservoir * reservoir)
{
   m_reservoir = reservoir;
}

const Reservoir * Trap::getReservoir (void) const
{
   return m_reservoir;
}

void Trap::setSnapshot (const Snapshot * snapshot)
{
   m_snapshot = snapshot;
}

const Snapshot * Trap::getSnapshot (void) const
{
   return m_snapshot;
}

/// Return the real world position of this Trap
void Trap::getPosition (double & x, double & y) const
{
   x = database::getXCoord (m_record);
   y = database::getYCoord (m_record);
}

/// Return the grid position of this Trap
void Trap::getGridPosition (unsigned int & i, unsigned int & j) const
{
   double x;
   double y;
   getPosition (x, y);

   const Grid * grid = (const Grid *) m_projectHandle->getHighResolutionOutputGrid ();

   grid->getGridPoint (x, y, i, j);
}

/// return the mass of the given component in this Trap
double Trap::getMass (ComponentId componentId) const
{
   return getMass (ComponentNames[componentId]);
}

/// return the mass of the given component in this Trap
double Trap::getMass (const string & componentName) const
{
   string componentMassName = "Mass" + componentName;
   return m_record->getValue<double>(componentMassName);
}

/// return the volume of the given phase in this Trap
double Trap::getVolume (PhaseId phaseId) const
{
   return getVolume (PhaseNames[phaseId]);
}

double Trap::getVolume (const string & phaseName) const
{
   string phaseVolumeName = "Volume" + phaseName;
   return m_record->getValue<double>(phaseVolumeName);
}

/// return the Trap's depth
double Trap::getDepth (void) const
{
   return database::getDepth (m_record);
}

/// return the Trap's depth
double Trap::getSpillDepth (void) const
{
   return database::getSpillDepth (m_record);
}

/// Return the real world position of this Trap's spill point
void Trap::getSpillPointPosition (double & x, double & y) const
{
   x = database::getSpillPointXCoord (m_record);
   y = database::getSpillPointYCoord (m_record);
}

/// Return the grid position of this Trap's spill point
void Trap::getSpillPointGridPosition (unsigned int & i, unsigned int & j) const
{
   double x;
   double y;
   getSpillPointPosition (x, y);

   const Grid * grid = (const Grid *) m_projectHandle->getHighResolutionOutputGrid ();

   grid->getGridPoint (x, y, i, j);
}

/// return the Trap's temperature
double Trap::getTemperature (void) const
{
   return database::getTemperature (m_record);
}
/// return the Trap's pressure
double Trap::getPressure (void) const
{
   return database::getPressure (m_record);
}

/// return the Trap's gas-oil contact depth
double Trap::getGOC (void) const
{
   return database::getGOC (m_record);
}

/// return the Trap's oil-water contact depth
double Trap::getOWC (void) const
{
   return database::getOWC (m_record);
}

/// return the Trap's surface at the oil-water contact depth
double Trap::getWCSurface (void) const
{
   return database::getWCSurface (m_record);
}

void Trap::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void Trap::asString (string & str) const
{
   ostringstream buf;

   unsigned int i;
   unsigned int j;
   getGridPosition (i, j);
   buf << "Trap:";
   buf << " id = " << getId ();
   buf << ", reservoir name = " << getReservoir ()->getName ();
   buf << ", snapshot = " << getSnapshot ()->getTime ();
   buf << ", position = (" << i << ", " << j << ")";
   buf << ",\ndepth = " << getDepth ();
   buf << ", temperature = " << getTemperature ();
   buf << ", pressure = " << getPressure ();
   buf << ", goc = " << getGOC ();
   buf << ", owc = " << getOWC ();
   buf << ", owc surface = " << getWCSurface ();
   buf << endl;

   str = buf.str ();
}
