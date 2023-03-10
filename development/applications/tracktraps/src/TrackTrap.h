//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef PERSISTENTTRAPS_TRAP_H
#define PERSISTENTTRAPS_TRAP_H

#include <vector>

#include "generics.h"

#include "ComponentManager.h"

using namespace CBMGenerics;

namespace database
{
   class Record;
   class Table;
}

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }

   namespace Interface
   {
      class Snapshot;
   }
}

#include "Trap.h"

using namespace DataAccess;

namespace PersistentTraps
{
   class Reservoir;
   class PersistentTrap;

   /// Class containing all the relevant Trap data and methods
   class TrackTrap : public Interface::Trap
   {
      public:
         /// Create a new Trap in an Reservoir
         TrackTrap (Interface::ProjectHandle& projectHandle, database::Record * record);

         ~TrackTrap (void);

         /// Add Point to the Trap's extent
         void addExtent (unsigned int i, unsigned int j);

         /// find the PersistentTrap that is closest to the Trap's Hotspot and is penetrating the Trap
         PersistentTrap * findMatchingPersistentTrap (std::vector < PersistentTrap * >&persistentTraps,
               const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot);

         /// find the PersistentTrap that is closest to the Trap's Hotspot and is penetrating the Trap
         PersistentTrap * findClosestPersistentTrap (std::vector < PersistentTrap * >&persistentTraps,
               const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot);

         /// Check if point is part of the Trap's Extent
         bool contains (unsigned int i, unsigned int j) const;

         int computeOverlap (TrackTrap * trap);

         /// Set the PersistentTrap of the Trap
         inline void setPersistentTrap (PersistentTrap * persistentTrap);

         /// Return the PersistentTrap of the Trap
         inline PersistentTrap *getPersistentTrap (void);

         inline void setPorosity (double porosity);
         inline double getPorosity (void);

         inline void setSealPermeability (double sealPermeability);
         inline double getSealPermeability (void);

         inline double getBrineSalinity();

         void save (database::Table * table);

         /// print some debugging info.
         void printOn (ostream & ostr) const;

      private:
         void saveStructuralProperties (database::Record * record);
         void saveReservoirChargeProperties (database::Record * record, double masses[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES], double density[ComponentManager::NUMBER_OF_PHASES], double viscosity[ComponentManager::NUMBER_OF_PHASES]);
         void saveStockTankChargeProperties (database::Record * record, int phaseRC, double masses[ComponentManager::NUMBER_OF_SPECIES], double density, double viscosity);

         bool performPVT (double masses[ComponentManager::NUMBER_OF_SPECIES], double temperature, double pressure,
         double phaseMasses[ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES], double phaseDensities[ComponentManager::NUMBER_OF_PHASES], double phaseViscosities[ComponentManager::NUMBER_OF_PHASES]);

      private:

         /// Surface of the Trap.
          std::vector<unsigned int> m_extent[2];

         double m_porosity;
         double m_sealPermeability;
         double m_brineSalinity;//This is used in buoyancy calculation for TrapperIoTbl

         PersistentTrap * m_persistentTrap;
   };
}

void PersistentTraps::TrackTrap::setPersistentTrap (PersistentTrap * persistentTrap)
{
   m_persistentTrap = persistentTrap;
}

PersistentTraps::PersistentTrap * PersistentTraps::TrackTrap::getPersistentTrap (void)
{
   return m_persistentTrap;
}

void PersistentTraps::TrackTrap::setPorosity (double porosity)
{
   m_porosity = porosity;
}

double PersistentTraps::TrackTrap::getPorosity (void)
{
   return m_porosity;
}

void PersistentTraps::TrackTrap::setSealPermeability (double sealPereability)
{
   m_sealPermeability = sealPereability;
}

double PersistentTraps::TrackTrap::getSealPermeability (void)
{
   return m_sealPermeability;
}

inline double PersistentTraps::TrackTrap::getBrineSalinity()
{
   return m_brineSalinity;
}

#endif // PERSISTENTTRAPS_TRAP_H

