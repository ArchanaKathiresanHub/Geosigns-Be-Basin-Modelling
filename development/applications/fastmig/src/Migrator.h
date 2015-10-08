//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_MIGRATOR_H_
#define _MIGRATION_MIGRATOR_H_

#include "GeoPhysicsProjectHandle.h"
#include "MigrationPropertyManager.h"
#include "MassBalance.h"
#include "ObjectFactory.h"

#include <string>
#include <vector>
#include <memory>

namespace database
{
   class Database;
   class Table;
   class Record;
}

using namespace DataAccess;

namespace migration
{
   class Formation;
   class Reservoir;
   class Barrier;
   class TrapPropertiesRequest;
   class MigrationRequest;
#ifdef USEOTGC
   class OilToGasCracker;
#endif
   const std::string MigrationActivityName = "HighResMigration";

   class Migrator
   {
   public:
      /// Constructor
      Migrator(const string & name);// , DataAccess::Interface::ObjectFactory* objectFactory);
      virtual ~Migrator (void);

      bool saveTo (const std::string & outputFileName);

      /// The top migration routine
      bool compute (void);

      void openMassBalanceFile (void);
      void closeMassBalanceFile (void);

      /// The GeoPhysics methods used to initialise GeoPhysics::ProjectHandle and set up the grid.
      bool setUpBasinGeometry (void);

      bool computeFormationPropertyMaps (const Interface::Snapshot * snapshot, bool isPressureRun);
      bool removeComputedFormationPropertyMaps (void);
      bool getSeaBottomDepths (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot);

      bool createFormationNodes (void);

      /// Compute the offsets of each reservoir from the top and bottom of its formation
      bool computeDepthOffsets (void);

      /// Compute the net/gross fractions
      bool computeNetToGross (void);

      bool isPressureRun (void) const;

      bool performSnapshotMigration (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool pressureRun);

      /// retrieve the complete list of formations
      virtual DataAccess::Interface::FormationList * getAllFormations (void) const;

      /// get a formation from the complete list of formations by index
      Formation * getFormation (int index);

      /// get the index of a formation in the complete list of formations
      int getIndex (Formation * formation);

      /// get a reservoir from the complete list of reservoirs by index
      Reservoir * getReservoir (int index);

      /// get the index of a reservoir in the complete list of reservoirs
      int getIndex (Reservoir * reservoir);

      Formation * getBottomSourceRockFormation ();
      Formation * getTopActiveFormation (const Interface::Snapshot * end);
      Formation * getBottomActiveReservoirFormation (const Interface::Snapshot * end);
      Formation * getTopActiveReservoirFormation (const Interface::Snapshot * end);

      GeoPhysics::ProjectHandle * getProjectHandle (void);

      void clearFormationNodeProperties ();
      bool computeFormationNodeProperties (const Interface::Snapshot * end);
      bool computeTargetFormationNodes (Formation * targetFormation, Formation * sourceFormation);

      /// Check if there are any active reservoirs at given snapshot
      bool activeReservoirs (const Interface::Snapshot * snapshot);

      bool detectReservoirs (const Interface::Snapshot * end);

      /// Charge the active reservoirs with charge already in the traps and
      /// additionally expelled charge between the given snapshots.
      bool chargeReservoirs (const Interface::Snapshot * start, const Interface::Snapshot * end);

      /// Charge the specified reservoir with charge already in the traps and
      /// additionally expelled charge between the given snapshots.
      bool chargeReservoir (Reservoir * reservoir, Reservoir * reservoirAbove, Reservoir * reservoirBelow,
                            const Interface::Snapshot * start, const Interface::Snapshot * end);

      /// Collect expelled charges into the given reservoir from the appropriate source rocks.
      bool collectAndMigrateExpelledCharges (Reservoir * reservoir, Reservoir * reservoirAbove, Reservoir * reservoirBelow,
                                             const Interface::Snapshot * start, const Interface::Snapshot * end, Barrier * barrier);

      /// retrieve the reservoirs, for the specified formation if specified.
      virtual DataAccess::Interface::ReservoirList * getReservoirs (const Interface::Formation * formation = 0) const;

      void addTrapRecord (Reservoir * reservoir, TrapPropertiesRequest & tpRequest);

      database::Record * copyMigrationRecord (database::Record * oldRecord, const std::string & newMigrationProcess);

      void addMigrationRecord (const std::string & srcReservoirName, const std::string & srcFormationName,
                               const std::string & dstReservoirName, MigrationRequest & mr);

      database::Record * findMigrationRecord (const std::string & srcReservoirName, const std::string & srcFormationName,
                                              const std::string & dstReservoirName,
                                              MigrationRequest & mr);

      database::Record * createMigrationRecord (const std::string & srcReservoirName, const std::string & srcFormationName,
                                                const std::string & dstReservoirName,
                                                MigrationRequest & mr);

      bool retrieveFormationPropertyMaps (const Interface::Snapshot * end);
      bool restoreFormationPropertyMaps (const Interface::Snapshot * end);

      bool retrieveFormationCapillaryPressureMaps (const Interface::Snapshot * end);
      bool restoreFormationCapillaryPressureMaps (const Interface::Snapshot * end);

      bool saveSMFlowPaths (const Interface::Snapshot * start, const Interface::Snapshot * end);

      /// compute secondary migration flow paths
      bool computeSMFlowPaths (const Interface::Snapshot * start, const Interface::Snapshot * end);

      /// compute secondary migration flow paths between source reservoir and target reservoir
      bool computeSMFlowPaths (Formation * targetFormation, Formation * sourceFormation, const Interface::Snapshot * start, const Interface::Snapshot * end);

      bool loadExpulsionMaps (const Interface::Snapshot * start, const Interface::Snapshot * end);
      bool unloadExpulsionMaps (const Interface::Snapshot * end);

      void sortMigrationRecords (void);
      void uniqueMigrationRecords (void);
      void sanitizeMigrationRecords (void);
      void checkMigrationRecords (void);
      void clearMigrationRecordLists (void);

      void renumberMigrationRecordTrap (const Interface::Snapshot * snapshot, int oldTrapNumber, int newTrapNumber);

      void deleteExpelledChargeMaps (const Interface::Snapshot * snapshot);

      const Interface::GridMap * getPropertyGridMap (const std::string & propertyName,
                                                     const Interface::Snapshot * snapshot,
                                                     const Interface::Reservoir * reservoir,
                                                     const Interface::Formation * formation,
                                                     const Interface::Surface * surface);

      MigrationPropertyManager& getPropertyManager ();

      inline bool performVerticalMigration (void);
      inline bool performHDynamicAndCapillary (void) const;
      inline bool isBlockingOn (void);
      inline double getBlockingPermeability (void);
      inline double getBlockingPorosity (void);

      const Interface::GridMap * getPropertyGridMap (const string & propertyName, const Interface::Snapshot * snapshot,
                                                     const Interface::Reservoir * reservoir,
                                                     const Interface::Formation * formation,
                                                     const Interface::Surface * surface) const;

   private:
      GeoPhysics::ProjectHandle* openProject (const std::string & fileName);

      mutable DataAccess::Interface::FormationList * m_formations;
      mutable DataAccess::Interface::ReservoirList * m_reservoirs;
      ofstream m_massBalanceFile;

      std::auto_ptr<GeoPhysics::ProjectHandle> m_projectHandle;

      MassBalance<ofstream>* m_massBalance;

      database::Table * m_trapIoTbl;
      database::Table * m_migrationIoTbl;

      bool mergeOutputFiles ();

#ifdef USEOTGC
      OilToGasCracker * m_otgc;
#endif

      bool m_verticalMigration;
      bool m_hdynamicAndCapillary;
      bool m_reservoirDetection;
      bool m_isBlockingOn;
      double m_blockingPermeability;
      double m_blockingPorosity;

      double m_minOilColumnHeight;
      double m_minGasColumnHeight;

      vector<database::Record *> * m_migrationRecordLists;
      std::auto_ptr<MigrationPropertyManager> m_propertyManager;

   };
}

inline migration::MigrationPropertyManager& migration::Migrator::getPropertyManager ()
{
   if (!m_propertyManager.get ()) assert (0);
   return *(m_propertyManager.get ());
}

bool migration::Migrator::performVerticalMigration (void)
{
   return m_verticalMigration;
}

bool migration::Migrator::performHDynamicAndCapillary (void) const
{
   return m_hdynamicAndCapillary;
}

bool migration::Migrator::isBlockingOn (void)
{
   return m_isBlockingOn;
}

double migration::Migrator::getBlockingPermeability (void)
{
   return (m_isBlockingOn ? m_blockingPermeability : 0);
}

double migration::Migrator::getBlockingPorosity (void)
{
   return (m_isBlockingOn ? m_blockingPorosity : 0);
}


#endif // _MIGRATION_MIGRATOR_H
