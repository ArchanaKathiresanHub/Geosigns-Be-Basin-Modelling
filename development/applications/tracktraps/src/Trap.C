//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include <limits>
#include <cfloat>
#include <iostream>
using namespace std;

#include "Trap.h"

#include "PersistentTrap.h"
#include "Reservoir.h"
#include "Interface/Snapshot.h"

#include "database.h"
#include "cauldronschemafuncs.h"

#include "EosPack.h"

#include "generics.h"

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;
using Utilities::Physics::StockTankPressureMPa;
using Utilities::Physics::StockTankTemperatureC;
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::MegaPaToPa;
using Utilities::Maths::CelciusToKelvin;
#include "ConstantsNumerical.h"
using Utilities::Numerical::IbsNoDataValue;

using namespace PersistentTraps;

Trap::Trap (Interface::ProjectHandle * projectHandle, database::Record * record)
   : Interface::Trap (projectHandle, record)
{
   m_persistentTrap = 0;
   m_porosity = -1;
   m_sealPermeability = -1;
}

Trap::~Trap (void)
{
}

void Trap::addExtent (unsigned int i, unsigned int j)
{
   // cerr << "adding (" << i << ", " << j << ") to " << getReservoir ()->getName () << "::" << getId () << endl;
   m_extent[0].push_back (i);
   m_extent[1].push_back (j);
}

PersistentTrap * Trap::findMatchingPersistentTrap (vector < PersistentTrap * >&persistentTraps,
      const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot)
{
   PersistentTrap * closestPersistentTrap = 0;
   double closestDistance = std::numeric_limits<double>::max();

   assert (getSnapshot () == snapshot);

   vector<PersistentTrap *>::iterator persistentTrapIter;
   for (persistentTrapIter = persistentTraps.begin (); persistentTrapIter != persistentTraps.end (); ++persistentTrapIter)
   {
      PersistentTrap *persistentTrap = *persistentTrapIter;
      assert (persistentTrap);

      // the last trap of a persistent trap must  be a trap with the previous snapshot
      if (persistentTrap->getSnapshotOfLastTrap () != previousSnapshot) continue;

      bool isPenetrated = persistentTrap->penetrates (this);
      bool penetrates = persistentTrap->isPenetratedBy (this);

      if (isPenetrated && penetrates)
      {
	 cerr << "trap " << getId () << " penetrates AND is penetrated by " << persistentTrap->getId () << endl;
	 return persistentTrap;
      }
   }
   return closestPersistentTrap;
}

PersistentTrap * Trap::findClosestPersistentTrap (vector < PersistentTrap * >&persistentTraps,
      const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot)
{
   PersistentTrap * closestPersistentTrap = 0;
   double closestDistance = std::numeric_limits<double>::max();

   assert (getSnapshot () == snapshot);

   vector<PersistentTrap *>::iterator persistentTrapIter;
   for (persistentTrapIter = persistentTraps.begin (); persistentTrapIter != persistentTraps.end (); ++persistentTrapIter)
   {
      PersistentTrap *persistentTrap = *persistentTrapIter;
      assert (persistentTrap);

      // the last trap of a persistent trap must  be a trap with the previous snapshot
      if (persistentTrap->getSnapshotOfLastTrap () != previousSnapshot) continue;

      bool isPenetrated = persistentTrap->penetrates (this);
      bool penetrates = persistentTrap->isPenetratedBy (this);

      if (isPenetrated && penetrates)
      {
	 cerr << "trap " << getId () << " illegally penetrates AND is penetrated by " << persistentTrap->getId () << endl;
	 return persistentTrap;
      }
      
      unsigned int i, j;
      getGridPosition (i, j);
      double distance = persistentTrap->getDistance (i, j);
      assert (distance >= 0);

      if (distance < closestDistance)
      {
	 closestDistance = distance;
	 closestPersistentTrap = persistentTrap;
      }
   }
   return closestPersistentTrap;
}

bool Trap::contains (unsigned int i, unsigned int j) const
{
   // cerr << "checking (" << i << ", " << j << ") with " << getReservoir ()->getName () << "::" << getId () << endl;
   vector < unsigned int >::const_iterator extentIter[2];

   for (extentIter[0] = m_extent[0].begin (), extentIter[1] = m_extent[1].begin ();
	 extentIter[0] != m_extent[0].end () && extentIter[1] != m_extent[1].end ();
	 ++extentIter[0], ++extentIter[1])
   {
      if (i == * extentIter[0] && j == * extentIter[1])
      {
	 // cerr << "---> found" << endl;
	 return true;
      }
   }
   // cerr << "---> not found" << endl;
   return false;
}

int Trap::computeOverlap (Trap * trap)
{
   vector < unsigned int >::const_iterator extentIter[2];

   int count = 0;
   for (extentIter[0] = m_extent[0].begin (), extentIter[1] = m_extent[1].begin ();
	 extentIter[0] != m_extent[0].end () && extentIter[1] != m_extent[1].end ();
	 ++extentIter[0], ++extentIter[1])
   {
      if (trap->contains (* extentIter[0], * extentIter[1]))
      {
	 ++count;
      }
   }
   return count;
}

void Trap::save (database::Table * table)
{
   int comp, phase;

   double masses[ComponentManager::NumberOfOutputSpecies];

   double massesRC[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies];
   double densitiesRC[ComponentManager::NumberOfPhases];
   double viscositiesRC[ComponentManager::NumberOfPhases];

   // save properties for both reservoir phases
   database::Record * record = table->createRecord ();

   saveStructuralProperties (record);

   for (comp = 0; comp < ComponentManager::NumberOfOutputSpecies; ++comp)
   {
      masses[comp] = getMass ((Interface::ComponentId) (comp));
   }

   // perform PVT under reservoir conditions
   performPVT (masses, getTemperature (), getPressure (),
	 massesRC, densitiesRC, viscositiesRC);

   for (phase = 0; phase < ComponentManager::NumberOfPhases; ++phase)
   {
      viscositiesRC[phase] *= 0.001; // Converting back to SI (from cP to Pa*s)
   }

   saveReservoirChargeProperties (record, massesRC, densitiesRC, viscositiesRC);

   for (phase = 0; phase < ComponentManager::NumberOfPhases; ++phase)
   {
      saveStockTankChargeProperties (record, phase, massesRC[phase], densitiesRC[phase], viscositiesRC[phase]);
   }
}

void Trap::saveStructuralProperties (database::Record * record)
{
   assert (m_persistentTrap);
   database::setPersistentTrapID (record, m_persistentTrap->getId ());
   database::setReservoirName (record, getReservoir ()->getName ());
   database::setAge (record, getSnapshot ()->getTime ());
   database::setTrapID (record, getId ());

   double x, y;
   getPosition (x, y);
   database::setXCoord (record, x);
   database::setYCoord (record, y);
   database::setTrapCapacity (record, database::getTrapCapacity (getRecord ()));
   database::setDepth (record, getDepth ());
   database::setSpillDepth (record, getSpillDepth ());
   getSpillPointPosition (x, y);
   database::setSpillPointXCoord (record, x);
   database::setSpillPointYCoord (record, y);
   database::setPressure (record, getPressure ());
   database::setTemperature (record, getTemperature ());

   database::setPermeability (record, database::getPermeability (getRecord ()));
   database::setSealPermeability (record, database::getSealPermeability (getRecord ()));
   database::setPorosity (record, getPorosity ());
   database::setNetToGross (record, database::getNetToGross (getRecord ()));
}

void Trap::saveReservoirChargeProperties (database::Record * record,
      double masses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies],
      double density[ComponentManager::NumberOfPhases],
      double viscosity[ComponentManager::NumberOfPhases])
{
   double cep;
   cep = database::getCEPGas (getRecord ());
   if (cep < 0)
   {
      cep = IbsNoDataValue;
   }
   database::setCEPVapour (record, cep);

   cep = database::getCEPOil (getRecord ());
   if (cep < 0)
   {
      cep = IbsNoDataValue;
   }
   database::setCEPLiquid (record, cep);

   double ct;
   ct = database::getCriticalTemperatureGas (getRecord ());
   if (ct < 0)
   {
      ct = IbsNoDataValue;
   }
   database::setCriticalTemperatureVapour (record, ct);

   ct = database::getCriticalTemperatureOil (getRecord ());
   if (ct < 0)
   {
      ct = IbsNoDataValue;
   }
   database::setCriticalTemperatureLiquid (record, ct);

   double it;
   it = database::getInterfacialTensionGas (getRecord ());
   if (it < 0)
   {
      it = IbsNoDataValue;
   }
   database::setInterfacialTensionVapour (record, it);

   it = database::getInterfacialTensionOil (getRecord ());
   if (it < 0)
   {
      it = IbsNoDataValue;
   }
   database::setInterfacialTensionLiquid (record, it);

   database::setWCSurface (record, getWCSurface ());
   database::setGOC (record, getGOC ());
   database::setOWC (record, getOWC ());

   database::setFracturePressure (record, database::getFracturePressure (getRecord ()));
   
   double massTotal[ComponentManager::NumberOfPhases];

   int phaseRC;
   for (phaseRC = 0; phaseRC < ComponentManager::NumberOfPhases; ++phaseRC)
   {
      massTotal[phaseRC] = Accumulate (masses[phaseRC], ComponentManager::NumberOfOutputSpecies);

      record->setValue (MassPrefix + ComponentManager::getInstance ().GetPhaseName (phaseRC), massTotal[phaseRC] > 1 ? massTotal[phaseRC] : 0);

      record->setValue (VolumePrefix + ComponentManager::getInstance ().GetPhaseName (phaseRC), massTotal[phaseRC] > 1 ? massTotal[phaseRC] / density[phaseRC] : 0);
      record->setValue (DensityPrefix + ComponentManager::getInstance ().GetPhaseName (phaseRC), massTotal[phaseRC] > 1 ? density[phaseRC] : IbsNoDataValue);
      record->setValue (ViscosityPrefix + ComponentManager::getInstance ().GetPhaseName (phaseRC), massTotal[phaseRC] > 1 ? viscosity[phaseRC] : IbsNoDataValue);
   }

   double buoyancy = 0;
   if (massTotal[ComponentManager::Vapour] > 1)
   {
      buoyancy += (1000 - density[ComponentManager::Vapour]) * AccelerationDueToGravity * (getGOC () - getDepth ());
   }
   if (massTotal[ComponentManager::Liquid] > 1)
   {
      buoyancy += (1000 - density[ComponentManager::Liquid]) * AccelerationDueToGravity * (getOWC () - getGOC ());
   }
   database::setBuoyancy (record, buoyancy * PaToMegaPa);
}

void Trap::saveStockTankChargeProperties (database::Record * record, int phaseRC, double masses[ComponentManager::NumberOfOutputSpecies], double density,  double viscosity)
{
   double viscositiesST[ComponentManager::NumberOfPhases];
   double densitiesST[ComponentManager::NumberOfPhases];
   double phaseMassesST[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies];

   performPVT (masses, StockTankTemperatureC, StockTankPressureMPa,
	 phaseMassesST, densitiesST, viscositiesST);

   int phaseST;

   double phaseMassSTTotal[ComponentManager::NumberOfPhases];
   for (phaseST = 0; phaseST < ComponentManager::NumberOfPhases; ++phaseST)
   {
      phaseMassSTTotal[phaseST] = Accumulate (phaseMassesST[phaseST], ComponentManager::NumberOfOutputSpecies);
      viscositiesST[phaseST] *= 0.001; // Converting back to SI (from cP to Pa*s)
   }

   if (phaseRC == ComponentManager::Vapour)
   {
      double cgr = IbsNoDataValue;
      if (phaseMassSTTotal[ComponentManager::Vapour] > 1)
      {
	 cgr = 0;
	 if (phaseMassSTTotal[ComponentManager::Liquid] > 1)
	 {
	    double volumeCondensate = phaseMassSTTotal[ComponentManager::Liquid] / densitiesST[ComponentManager::Liquid];
	    double volumeGas = phaseMassSTTotal[ComponentManager::Vapour] / densitiesST[ComponentManager::Vapour];
	    cgr = volumeCondensate / volumeGas;
	 }
      }

      database::setCGR (record, cgr);
   }

   if (phaseRC == ComponentManager::Liquid)
   {
      double gor = IbsNoDataValue;
      double oilAPI = IbsNoDataValue;

      if (phaseMassSTTotal[ComponentManager::Liquid] > 1)
      {
	 gor = 0;
	 if (phaseMassSTTotal[ComponentManager::Vapour] > 1)
	 {
	    double volumeOil = phaseMassSTTotal[ComponentManager::Liquid] / densitiesST[ComponentManager::Liquid];
	    double volumeGas = phaseMassSTTotal[ComponentManager::Vapour] / densitiesST[ComponentManager::Vapour];
	    gor = volumeGas / volumeOil;
	 }

	 oilAPI = 141.5/(0.001*densitiesST[ComponentManager::Liquid]) -131.5;
      }

      database::setGOR (record, gor);
      database::setOilAPI (record, oilAPI);
   }

   for (phaseST = 0; phaseST < ComponentManager::NumberOfPhases; ++phaseST)
   {
      double phaseMassSTTotal = Accumulate (phaseMassesST[phaseST], ComponentManager::NumberOfOutputSpecies);
      record->setValue (StockTankPhaseNames[phaseRC][phaseST] + MassPrefix, phaseMassSTTotal > 1 ? phaseMassSTTotal : 0);

      record->setValue (StockTankPhaseNames[phaseRC][phaseST] + VolumePrefix, phaseMassSTTotal > 1 ? phaseMassSTTotal / densitiesST[phaseST] : 0);
      record->setValue (StockTankPhaseNames[phaseRC][phaseST] + DensityPrefix, phaseMassSTTotal > 1 ? densitiesST[phaseST] : IbsNoDataValue);
      record->setValue (StockTankPhaseNames[phaseRC][phaseST] + ViscosityPrefix, phaseMassSTTotal > 1 ? viscositiesST[phaseST] : IbsNoDataValue);

      int comp;
      for (comp = 0; comp < ComponentManager::NumberOfOutputSpecies; ++comp)
      {
	 record->setValue (StockTankPhaseNames[phaseRC][phaseST] + MassPrefix + TableSpeciesNames[comp], phaseMassSTTotal > 1 ? phaseMassesST[phaseST][comp] : 0);
      }
   }

#ifdef TOBEDONE
   double num, denom;
   double gasMass;

   int phase;

   num = 0; 
   for (comp = ComponentManager::C5; comp <=  ComponentManager::C2; ++comp)
   {
      num += masses[comp] / MolarWeights[comp];
   }

   denom = 0; 
   gasMass = 0;
   for (comp =  ComponentManager::C5; comp <= ComponentManager::C1; ++comp)
   {
      denom += masses[comp] / MolarWeights[comp];
      gasMass += masses[comp];
   }

   if (denom != 0)
   {
      record->setValue ("GasWetness", num / denom);
   }
   else
   {
      record->setValue ("GasWetness", double (-1));
   }

   for (phase = 0; phase < ComponentManager::NumberOfPhases; ++phase)
   {
      double num = 0; 
      for (comp = ComponentManager::C5; comp <= ComponentManager::C1; ++comp)
      {
	 num += phaseMassesST[phase][comp];
      }

      double denom = 0; 
      for (comp = ComponentManager::C15PlusAro; comp <= ComponentManager::C6Minus14Sat; ++comp)
      {
	 denom += phaseMassesST[phase][comp];
      }

      if (performedPVT && denom > 0)
      {
	 record->setValue (ComponentManager::getInstance ().GetPhaseName (phase) + "GORM", num/denom);
      }
      else
      {
	 record->setValue (ComponentManager::getInstance ().GetPhaseName (phase) + "GORM", (double) -1);
      }
   }
#endif
}

bool Trap::performPVT (double masses[ComponentManager::NumberOfOutputSpecies], double temperature, double pressure,
      double phaseMasses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], double phaseDensities[ComponentManager::NumberOfPhases], double phaseViscosities[ComponentManager::NumberOfPhases])
{
   bool performedPVT = false;
   double massTotal = 0;

   int phase, comp;

   for (comp = 0; comp < ComponentManager::NumberOfOutputSpecies; ++comp)
   {
      massTotal += masses[comp];

      for (phase = 0; phase < ComponentManager::NumberOfPhases; ++phase)
      {
	 phaseMasses[phase][comp] = 0;
      }
   }

   for (phase = 0; phase < ComponentManager::NumberOfPhases; ++phase)
   {
      phaseDensities[phase] = 0;
      phaseViscosities[phase] = 0;
   }

   if (massTotal > 100)
   {
      performedPVT = pvtFlash::EosPack::getInstance().computeWithLumping (temperature + CelciusToKelvin, pressure * MegaPaToPa, masses, phaseMasses, phaseDensities, phaseViscosities);

   }

   return performedPVT;
}


void Trap::printOn (ostream & ostr) const
{
   Interface::Trap::printOn (ostr);
   if (m_persistentTrap)
   {
      ostr << " (" << * m_persistentTrap << ")" << endl;
   }
}
