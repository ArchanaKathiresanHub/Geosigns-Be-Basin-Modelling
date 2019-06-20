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

// TableIO library
#include "database.h"
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
#include "Grid.h"
#include "ProjectHandle.h"
#include "Reservoir.h"
#include "Snapshot.h"
#include "Trapper.h"
using namespace DataAccess;
using namespace Interface;

static const char** ComponentNames = CBMGenerics::ComponentManager::getInstance().getSpeciesNameInputList();

const string PhaseNames[] =
{ 
   "Vapour", "Liquid"
};

const string StockTankPhaseNames[] =
{ 
   "FreeGas", "Condensate", "SolutionGas", "StockTankOil"
};

Trapper::Trapper (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_reservoir = 0;
   m_snapshot = 0;
   m_downstreamTrapper = 0;
}

Trapper::~Trapper (void)
{
}

bool Trapper::matchesConditions (const Reservoir * reservoir, const Snapshot * snapshot, unsigned int id, unsigned int persistentId)
{
   if (snapshot != 0 && getSnapshot () != snapshot) return false;
   if (reservoir != 0 && getReservoir () != reservoir) return false;
   if (id != 0 && getId () != id) return false;
   if (persistentId != 0 && getPersistentId () != persistentId) return false;
   return true;
}

unsigned int Trapper::getPersistentId (void) const
{
   return database::getPersistentTrapID (m_record);
}

unsigned int Trapper::getId (void) const
{
   return database::getTrapID (m_record);
}

void Trapper::setReservoir (const Reservoir * reservoir)
{
   m_reservoir = reservoir;
}

const Reservoir * Trapper::getReservoir (void) const
{
   return m_reservoir;
}

void Trapper::setSnapshot (const Snapshot * snapshot)
{
   m_snapshot = snapshot;
}

const Snapshot * Trapper::getSnapshot (void) const
{
   return m_snapshot;
}

void Trapper::addUpstreamTrapper (Trapper * trapper)
{
   m_upstreamTrappers.push_back (trapper);
}

void Trapper::setDownstreamTrapper (Trapper * trapper)
{
   m_downstreamTrapper = trapper;
}

/// Return the trappers that spill to this trap
const TrapperList * Trapper::getUpstreamTrappers (void) const
{
   TrapperList * trapperList = new TrapperList;

   MutableTrapperList::const_iterator trapperIter;

   for (trapperIter = m_upstreamTrappers.begin (); trapperIter != m_upstreamTrappers.end (); ++trapperIter)
   {
      trapperList->push_back (* trapperIter);
   }
   return trapperList;
}

/// Return the trap this trap spills to
const Trapper * Trapper::getDownstreamTrapper (void) const
{
   return m_downstreamTrapper;
}

/// Return the real world position of this Trapper
void Trapper::getPosition (double & x, double & y) const
{
   x = database::getXCoord (m_record);
   y = database::getYCoord (m_record);
}

/// Return the real world spill point position of this Trapper
void Trapper::getSpillPointPosition (double & x, double & y) const
{
   x = database::getSpillPointXCoord (m_record);
   y = database::getSpillPointYCoord (m_record);
}

/// Return the grid position of this Trapper
void Trapper::getGridPosition (unsigned int & i, unsigned int & j) const
{
   double x;
   double y;
   getPosition (x, y);

   const Grid * grid = (const Grid *) m_projectHandle->getHighResolutionOutputGrid ();

   grid->getGridPoint (x, y, i, j);
}


double Trapper::getMass (PhaseId rcPhaseId, PhaseId stPhaseId, ComponentId componentId) const
{
   string stockTankPhaseName = StockTankPhaseNames[(int) rcPhaseId * 2 + (int) stPhaseId];
   return getMass (stockTankPhaseName, ComponentNames[componentId]);
}

double Trapper::getMass (PhaseId rcPhaseId, ComponentId componentId) const
{
   return getMass (rcPhaseId, PhaseId::GAS, componentId) +
          getMass (rcPhaseId, PhaseId::OIL, componentId);
}

/// return the mass of the given component in this Trapper
double Trapper::getMass (ComponentId componentId) const
{
   return getMass (PhaseId::GAS, PhaseId::GAS, componentId) +
          getMass (PhaseId::GAS, PhaseId::OIL, componentId) +
          getMass (PhaseId::OIL, PhaseId::GAS, componentId) +
          getMass (PhaseId::OIL, PhaseId::OIL, componentId);
}

/// return the mass of the given component in this Trapper
double Trapper::getMass (const string & stockTankPhaseName, const string & componentName) const
{
   string componentMassName = stockTankPhaseName + "Mass" + componentName;
   return m_record->getValue<double> (componentMassName);
}

double Trapper::getMass (PhaseId rcPhaseId) const
{
   string phaseMassName = "Mass" + PhaseNames[rcPhaseId];
   return m_record->getValue<double> (phaseMassName);
}


double Trapper::getMass (PhaseId rcPhaseId, PhaseId stPhaseId) const
{
   string phaseMassName = StockTankPhaseNames[(int) rcPhaseId * 2 + (int) stPhaseId] + "Mass";
   return m_record->getValue<double> (phaseMassName);
}

double Trapper::getVolume (const string & phaseName) const
{
   string phaseVolumeName = "Volume" + phaseName;
   return m_record->getValue<double> (phaseVolumeName);
}

/// return the volume of the given phase in this Trapper
double Trapper::getVolume (PhaseId phaseId) const
{
   return getVolume (PhaseNames[phaseId]);
}

double Trapper::getVolume (PhaseId rcPhaseId, PhaseId stPhaseId) const
{
   string phaseVolumeName = StockTankPhaseNames[(int) rcPhaseId * 2 + (int) stPhaseId] + "Volume";
   return m_record->getValue<double> (phaseVolumeName);
}

double Trapper::getViscosity (const string & phaseName) const
{
   string phaseViscosityName = "Viscosity" + phaseName;
   return m_record->getValue<double> (phaseViscosityName);
}

/// return the volume of the given phase in this Trapper
double Trapper::getViscosity (PhaseId phaseId) const
{
   return getViscosity (PhaseNames[phaseId]);
}

double Trapper::getViscosity (PhaseId rcPhaseId, PhaseId stPhaseId) const
{
   string phaseViscosityName = StockTankPhaseNames[(int) rcPhaseId * 2 + (int) stPhaseId] + "Viscosity";
   return m_record->getValue<double> (phaseViscosityName);
}

double Trapper::getDensity (const string & phaseName) const
{
   string phaseDensityName = "Density" + phaseName;
   return m_record->getValue<double> (phaseDensityName);
}

/// return the volume of the given phase in this Trapper
double Trapper::getDensity (PhaseId phaseId) const
{
   return getDensity (PhaseNames[phaseId]);
}

double Trapper::getDensity (PhaseId rcPhaseId, PhaseId stPhaseId) const
{
   string phaseDensityName = StockTankPhaseNames[(int) rcPhaseId * 2 + (int) stPhaseId] + "Density";
   return m_record->getValue<double> (phaseDensityName);
}

double Trapper::getInterfacialTension (const string & phaseName) const
{
   string phaseInterfacialTensionName = "InterfacialTension" + phaseName;
   return m_record->getValue<double> (phaseInterfacialTensionName);
}

/// return the volume of the given phase in this Trapper
double Trapper::getInterfacialTension (PhaseId phaseId) const
{
   return getInterfacialTension (PhaseNames[phaseId]);
}

double Trapper::getCriticalTemperature (const string & phaseName) const
{
   string phaseCriticalTemperatureName = "CriticalTemperature" + phaseName;
   return m_record->getValue<double> (phaseCriticalTemperatureName);
}

/// return the volume of the given phase in this Trapper
double Trapper::getCriticalTemperature (PhaseId phaseId) const
{
   return getCriticalTemperature (PhaseNames[phaseId]);
}

double Trapper::getCEP (const string & phaseName) const
{
   string phaseCEPName = "CEP" + phaseName;
   return m_record->getValue<double> (phaseCEPName);
}

/// return the volume of the given phase in this Trapper
double Trapper::getCEP (PhaseId phaseId) const
{
   return getCEP (PhaseNames[phaseId]);
}

/// return the Trapper's OilAPI
double Trapper::getOilAPI (void) const
{
   return database::getOilAPI (m_record);
}

/// return the Trapper's CGR
double Trapper::getCGR (void) const
{
   return database::getCGR (m_record);
}

/// return the Trapper's GOR
double Trapper::getGOR (void) const
{
   return database::getGOR (m_record);
}

/// return the Trapper's FracturePressure
double Trapper::getFracturePressure (void) const
{
   return database::getFracturePressure (m_record);
}

/// return the Trapper's Buoyancy
double Trapper::getBuoyancy (void) const
{
   return database::getBuoyancy (m_record);
}

/// return the Trapper's Capacity
double Trapper::getCapacity (void) const
{
   return database::getTrapCapacity (m_record);
}

/// return the Trapper's depth
double Trapper::getDepth (void) const
{
   return database::getDepth (m_record);
}

/// return the Trapper's depth
double Trapper::getSpillDepth (void) const
{
   return database::getSpillDepth (m_record);
}

/// return the Trapper's temperature
double Trapper::getTemperature (void) const
{
   return database::getTemperature (m_record);
}
/// return the Trapper's pressure
double Trapper::getPressure (void) const
{
   return database::getPressure (m_record);
}

/// return the Trapper's gas-oil contact depth
double Trapper::getGOC (void) const
{
   return database::getGOC (m_record);
}

/// return the Trapper's oil-water contact depth
double Trapper::getOWC (void) const
{
   return database::getOWC (m_record);
}

/// return the Trapper's surface at the oil-water contact depth
double Trapper::getWCSurface (void) const
{
   return database::getWCSurface (m_record);
}

/// return the Trapper's Permeability
double Trapper::getPermeability (void) const
{
   return database::getPermeability (m_record);
}

/// return the Trapper's SealPermeability
double Trapper::getSealPermeability (void) const
{
   return database::getSealPermeability (m_record);
}

/// return the Trapper's porosity
double Trapper::getPorosity (void) const
{
   return database::getPorosity (m_record);
}

/// return the Trapper's net to gross
double Trapper::getNetToGross (void) const
{
   return database::getNetToGross (m_record);
}

void Trapper::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void Trapper::asString (string & str) const
{
   ostringstream buf;

   unsigned int i;
   unsigned int j;
   getGridPosition (i, j);
   buf << "Trapper:";
   buf << " id = " << getId ();
   buf << " persistent id = " << getPersistentId ();
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
