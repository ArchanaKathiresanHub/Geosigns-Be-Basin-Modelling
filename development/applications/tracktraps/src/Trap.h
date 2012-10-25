#ifndef _PERSISTENTTRAPS_TRAP_H_
#define _PERSISTENTTRAPS_TRAP_H_

#include <vector>

using namespace std;

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

#include "Interface/Trap.h"

using namespace DataAccess;

namespace PersistentTraps
{
   class Reservoir;
   class PersistentTrap;

   /// Class containing all the relevant Trap data and methods
   class Trap : public Interface::Trap
   {
      public:
	 /// Create a new Trap in an Reservoir
	 Trap (Interface::ProjectHandle * projectHandle, database::Record * record);

	 ~Trap (void);

	 /// Add Point to the Trap's extent
	 void addExtent (unsigned int i, unsigned int j);

	 /// find the PersistentTrap that is closest to the Trap's Hotspot and is penetrating the Trap
	 PersistentTrap * findMatchingPersistentTrap (vector < PersistentTrap * >&persistentTraps,
	       const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot);

	 /// find the PersistentTrap that is closest to the Trap's Hotspot and is penetrating the Trap
	 PersistentTrap * findClosestPersistentTrap (vector < PersistentTrap * >&persistentTraps,
	       const Interface::Snapshot * snapshot, const Interface::Snapshot * previousSnapshot);

	 /// Check if point is part of the Trap's Extent
	 bool contains (unsigned int i, unsigned int j) const;

	 int computeOverlap (Trap * trap);

	 /// Set the PersistentTrap of the Trap
	 inline void setPersistentTrap (PersistentTrap * persistentTrap);

	 /// Return the PersistentTrap of the Trap
	 inline PersistentTrap *getPersistentTrap (void);

	 inline void setPorosity (double porosity);
	 inline double getPorosity (void);

	 inline void setSealPermeability (double sealPermeability);
	 inline double getSealPermeability (void);

	 void save (database::Table * table);

	 /// print some debugging info.
	 void printOn (ostream & ostr) const;

      private:
	 void saveStructuralProperties (database::Record * record);
	 void saveReservoirChargeProperties (database::Record * record, double masses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], double density[ComponentManager::NumberOfPhases], double viscosity[ComponentManager::NumberOfPhases]);
	 void saveStockTankChargeProperties (database::Record * record, int phaseRC, double masses[ComponentManager::NumberOfOutputSpecies], double density, double viscosity);

	 bool performPVT (double masses[ComponentManager::NumberOfOutputSpecies], double temperature, double pressure,
	       double phaseMasses[ComponentManager::NumberOfPhases][ComponentManager::NumberOfOutputSpecies], double phaseDensities[ComponentManager::NumberOfPhases], double phaseViscosities[ComponentManager::NumberOfPhases]);

      private:

	 /// Surface of the Trap.
	 vector<unsigned int> m_extent[2];

	 double m_porosity;
	 double m_sealPermeability;

	 PersistentTrap * m_persistentTrap;
   };
}

void PersistentTraps::Trap::setPersistentTrap (PersistentTrap * persistentTrap)
{
   m_persistentTrap = persistentTrap;
}

PersistentTraps::PersistentTrap * PersistentTraps::Trap::getPersistentTrap (void)
{
   return m_persistentTrap;
}

void PersistentTraps::Trap::setPorosity (double porosity)
{
   m_porosity = porosity;
}

double PersistentTraps::Trap::getPorosity (void)
{
   return m_porosity;
}

void PersistentTraps::Trap::setSealPermeability (double sealPereability)
{
   m_sealPermeability = sealPereability;
}

double PersistentTraps::Trap::getSealPermeability (void)
{
   return m_sealPermeability;
}

#endif // _PERSISTENTTRAPS_TRAP_H_

