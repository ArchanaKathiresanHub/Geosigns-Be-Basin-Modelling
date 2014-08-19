#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>

#include "petsc.h"

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "Migrator.h"
#include "Reservoir.h"
#include "Formation.h"
#include "Surface.h"

#ifdef USEOTGC
#include "OilToGasCracker.h"
#endif
#include "RequestDefs.h"
#include "Barrier.h"

#include "rankings.h"

using namespace migration;
 
#include "Interface/ProjectHandle.h"
#include "Interface/Reservoir.h"
#include "Interface/PropertyValue.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "MassBalance.h"
#include "utils.h"

#include "h5_parallel_file_types.h"
#include "h5merge.h"

using namespace DataAccess;

static bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2);

extern string NumProcessorsArg;

Migrator::Migrator (database::Database * database, const string & name, const string & accessMode)
      : Interface::ProjectHandle (database, name, accessMode),
        m_massBalance(0)
{
   m_reservoirs = 0;
   m_formations = 0;
   InitializeRequestTypes ();
   if (GetRank () == 0)
   {
      m_migrationRecordLists = new vector<database::Record *> [NUMBEROFPROCESSES];
   }

   m_migrationIoTbl = 0;
   m_trapIoTbl = 0;

#ifdef USEOTGC   
   bool includeSulphur = containsSulphur ();
   m_otgc = new OilToGasCracker(includeSulphur);
#endif
}

Migrator::~Migrator (void)
{
   if (m_reservoirs) delete m_reservoirs;
   if (m_formations) delete m_formations;
#ifdef USEOTGC
   if (m_otgc) delete m_otgc;
#endif

   if (GetRank () == 0)
   {
      delete [] m_migrationRecordLists;
   }
}

Migrator * Migrator::CreateFrom (const string & inputFileName)
{
   Migrator * migrator = (Migrator *) Interface::OpenCauldronProject (inputFileName, "rw");
   if (migrator) migrator->loadFaults ();
   return migrator;
}

bool Migrator::saveTo (const string & outputFileName)
{
   return saveToFile (outputFileName);
}

bool Migrator::compute (void)
{
   string activityName = MigrationActivityName;
   activityName += NumProcessorsArg;
   H5_Parallel_PropertyList::setOneFilePerProcessOption ();

   // let's first clean out the mess from a previous run
   if (GetRank () == 0)
   {
      deletePropertyValues (DataAccess::Interface::RESERVOIR);

      if (!m_trapIoTbl) m_trapIoTbl = getTable ("TrapIoTbl");
      m_trapIoTbl->clear ();

      if (!m_migrationIoTbl) m_migrationIoTbl = getTable ("MigrationIoTbl");
      m_migrationIoTbl->clear ();
   }
   
   bool started = startActivity (activityName, getHighResolutionOutputGrid ());
   if (!started) return false;

   if (GetRank () == 0)
   {
      string fileName = utils::getProjectBaseName(m_projectName);
      fileName += "_MassBalance";

      m_massBalanceFile.open (fileName.c_str (), ios::out);
      m_massBalance = new MassBalance<ofstream>(m_massBalanceFile);
   }

   ComputeRanks (getActivityOutputGrid ());

   // compute the positions of the reservoirs within the formations
   computeDepthOffsets ();
   computeNetToGross ();

   Interface::SnapshotList * snapshots = getSnapshots ();

   Interface::SnapshotList::reverse_iterator snapshotIter;

   const Interface::Snapshot * start;
   const Interface::Snapshot * end = 0;
   // skip the first as it does not contain meaningful data.
   for (snapshotIter = snapshots->rbegin (), ++snapshotIter; snapshotIter != snapshots->rend (); ++snapshotIter)
   {
      start = end;
      end = * snapshotIter;

      if (!start) continue;

#if 0
      cerr << GetRankString () << ": " << "Snapshot:: " << end->getTime () << endl;
#endif

      if (activeReservoirs (end))
      {
         if (!chargeReservoirs (start, end))
         {
            return false;
         }
      }

      continueActivity ();

      deletePropertyValueGridMaps (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE | Interface::RESERVOIR,
            0, start, 0, 0, 0, Interface::MAP | Interface::VOLUME); 
   }

   deletePropertyValueGridMaps (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE | Interface::RESERVOIR,
         0, end, 0, 0, 0, Interface::MAP | Interface::VOLUME); 

   delete snapshots;

   if (GetRank () == 0)
   {
      m_massBalanceFile.close ();
      delete m_massBalance;
      m_massBalance = 0;
   }

#if 0
   cerr << GetRankString () << ": " << "Finishing activity" << endl;
#endif
   finishActivity ();
   bool status = true;
   if( !mergeOutputFiles ()) {
      PetscPrintf ( PETSC_COMM_WORLD, "MeSsAgE ERROR Unable to merge output files\n");
      status = false;
   }
#if 0
   cerr << GetRankString () << ": " << "Finished activity" << endl;
#endif

   return status;
}
bool Migrator::mergeOutputFiles ( ) {

   if( ! H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ){
      return true;
   }
   PetscBool noFileCopy = PETSC_FALSE;

   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );

   string filePathName = getProjectPath () + "/" + getOutputDir () + "/" + MigrationActivityName + "_Results.HDF";

   bool status = mergeFiles ( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), !noFileCopy );
   if( status ) {
     status = H5_Parallel_PropertyList::copyMergedFile( filePathName ); 
   }
   if( ! status ) {
      PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not merge the file %s.\n", filePathName.c_str() );               
   } else {
      ReportProgress ("Merged Output Maps");
   }
   return status;
}

/// compute the positions of the reservoirs within the formations
bool Migrator::computeDepthOffsets (void)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;
      
      assert (reservoir);

      reservoir->computeDepthOffsets (findSnapshot (0.));
   }
   return true;
}

/// compute the nett/gross fractions
bool Migrator::computeNetToGross (void)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) * reservoirIter;
      
      assert (reservoir);

      reservoir->computeNetToGross ();
   }
   return true;
}

bool Migrator::activeReservoirs (const Interface::Snapshot * snapshot)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      const Reservoir * reservoir = (const Reservoir *) * reservoirIter;
      
      assert (reservoir);

      if (reservoir->isActive (snapshot))
      {
         return true;
      }
   }
   return false;
}

bool Migrator::chargeReservoirs (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;
   Interface::ReservoirList::iterator reservoirAboveIter;


   Reservoir * reservoir = 0;
   Reservoir * reservoirBelow = 0;
   Reservoir * reservoirAbove = 0;
   for (reservoir = 0, reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      reservoir = (Reservoir *) * reservoirIter;

      reservoirAboveIter = reservoirIter;
      if (++reservoirAboveIter != reservoirs->end ())
         reservoirAbove = (Reservoir *) * reservoirAboveIter;
      else
         reservoirAbove = 0;

      assert (reservoir);
#if 0
      cerr << GetRankString () << ": " << "Evaluating Reservoir: " << reservoir->getName () << endl;
#endif
      if (reservoir->isActive (start))
      {
#if 0
         cerr << GetRankString () << ": " << "Charging Reservoir: " << reservoir->getName () << endl;
#endif
         if (!chargeReservoir (reservoir, reservoirAbove, reservoirBelow, start, end))
            return false;
         reservoirBelow = reservoir;
      }

#if 0
      reservoir->reportChargeDistributionCount ();
#endif
      ReportProgress ("Reservoir: ", reservoir->getName (), "", end->getTime ());
   }

   return true;
}

bool Migrator::chargeReservoir (Reservoir * reservoir, Reservoir * reservoirAbove, Reservoir * reservoirBelow,
      const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   if (GetRank () == 0)
   {
      // the existing ones belong to the previous snapshots, hence need to be cleared
      clearMigrationRecordLists ();
   }
   reservoir->setStart (start);
   reservoir->setEnd (end);

   reservoir->retainPreviousTraps ();
   reservoir->retainPreviousProperties ();

   reservoir->clearProperties ();
   if (!reservoir->computeProperties ())
      return false;

   reservoir->refineGeometry ();

   reservoir->saveComputedInputProperties ();

   reservoir->computePathways ();
   reservoir->computeTargetColumns ();

   reservoir->computeTraps ();

   Barrier * barrier = 0;
   if (reservoir->isBlockingEnabled ())
   {
      barrier = new Barrier (reservoir);
   }

   double totalRetainedBeforeCracking = reservoir->getTotalChargesToBeMigrated ();

#ifdef USEOTGC
   reservoir->crackChargesToBeMigrated (* m_otgc);


   // trap capacities will have changed
   reservoir->recomputeTrapDepthToVolumeFunctions ();
#endif

   double totalRetainedAfterCracking = reservoir->getTotalChargesToBeMigrated ();

   reservoir->migrateChargesToBeMigrated (0, reservoir);
   reservoir->clearPreviousProperties ();

#if 0
   if (GetRank () == 0) cerr << "totalRetainedAfterCracking = " << totalRetainedAfterCracking << endl;
   if (GetRank () == 0) cerr << "totalRetainedBeforeCracking = " << totalRetainedBeforeCracking << endl;
#endif

   collectAndMigrateExpelledCharges (reservoir, reservoirAbove, reservoirBelow, start, end, barrier);

   double totalExpelled = reservoir->getTotalToBeStoredCharges () - totalRetainedAfterCracking;

   double expelledBlocked = reservoir->getTotalBlocked ();

   if (reservoirBelow)
   {
      reservoir->collectLeakedCharges (reservoirBelow, barrier);
      reservoir->migrateChargesToBeMigrated (0, reservoirBelow);
   }

   if (barrier) delete barrier;

   double totalToBeStored = reservoir->getTotalToBeStoredCharges ();

   double totalLeakedIn = totalToBeStored - totalRetainedAfterCracking - totalExpelled;
   double leakedBlocked = reservoir->getTotalBlocked () - expelledBlocked;


   if (GetRank () == 0) m_massBalance->addToBalance ("Already in reservoir", totalRetainedBeforeCracking);

#ifdef USEOTGC
   double totalCrackingLoss = reservoir->getTotalCrackingLoss ();
   if (GetRank () == 0) m_massBalance->subtractFromBalance ("Charge converted by oil to gas cracking", totalCrackingLoss);

   double totalCrackingGain = reservoir->getTotalCrackingGain ();
   if (GetRank () == 0) m_massBalance->addToBalance ("Charge produced by oil to gas cracking", totalCrackingGain);
#endif

   totalExpelled += expelledBlocked;
   if (GetRank () == 0) m_massBalance->addToBalance ("Expelled into reservoir", totalExpelled);

   totalLeakedIn += leakedBlocked;
   if (GetRank () == 0) m_massBalance->addToBalance ("Leaked into reservoir", totalLeakedIn);

   if (GetRank () == 0) m_massBalance->subtractFromBalance ("Blocked from entering reservoir", leakedBlocked + expelledBlocked);

   if (Abs (totalLeakedIn) < 1e-3) totalLeakedIn = 0;

   if (!reservoir->fillAndSpill ())
      return false;

   double totalStoredInTraps = reservoir->getTotalStoredCharges ();
   if (GetRank () == 0) m_massBalance->subtractFromBalance ("Stored in reservoir", totalStoredInTraps);

   double totalBiodegraded = reservoir->getTotalBiodegradedCharges ();

   if (GetRank () == 0) m_massBalance->subtractFromBalance ("Biodegraded", totalBiodegraded);

   double totalDiffusionLeaked = reservoir->getTotalDiffusionLeakedCharges ();
   if (GetRank () == 0) m_massBalance->subtractFromBalance ("Diffused", totalDiffusionLeaked);

   double totalLeakedOutward = reservoir->getTotalToBeStoredCharges (true /* onBoundaryOnly */);
   if (GetRank () == 0) m_massBalance->subtractFromBalance ("Leaked outward from reservoir", totalLeakedOutward);
   double totalLeakedUpward = reservoir->getTotalToBeStoredCharges ();
   totalLeakedUpward -= totalLeakedOutward;
   if (GetRank () == 0) m_massBalance->subtractFromBalance ("Leaked upward from reservoir", totalLeakedUpward);


   reservoir->broadcastTrapDiffusionStartTimes ();
   reservoir->broadcastTrapPenetrationDistances ();

   reservoir->broadcastTrapFillDepthProperties ();
   reservoir->broadcastTrapChargeProperties ();

   double totalBroadcastFromTraps = reservoir->getTotalChargesToBeMigrated ();

   if (GetRank () == 0)
   {
      if (totalStoredInTraps > 0 && Abs (totalStoredInTraps - totalBroadcastFromTraps) / totalStoredInTraps > 0.01)
      {
         cerr << "Error: Difference between charge as stored in traps and charge as stored in columns is too large" << endl;
         cerr << "    Total stored in traps = " << totalStoredInTraps << endl;
         cerr << "    Total stored in columns = " << totalBroadcastFromTraps << endl;
         cerr << "    Error =  " << Abs (totalStoredInTraps - totalBroadcastFromTraps) / totalStoredInTraps << endl;
#ifdef CANNOTPRINTINCONSISTENTTRAPVOLUMES
         cerr << "    Inconsistent Traps:" << endl;
         reservoir->printInconsistentTrapVolumes ();
#endif
      }
   }

   reservoir->removePreviousTraps ();

   // print mass balance
   if (GetRank () == 0)
   {
      string str = string("Reservoir: ") + reservoir->getName();
      
      m_massBalance->printMassBalance (start, end, str); 
   }

   if (GetRank () == 0) m_massBalance->clear();

   reservoir->saveTrapProperties ();
   reservoir->computeFluxes ();
   reservoir->saveComputedOutputProperties ();

   return true;
}

// collect expelled charges into reservoirs from the formations that are
// above reservoirBelow and not above or just above reservoir

bool Migrator::collectAndMigrateExpelledCharges (Reservoir * reservoir, Reservoir * reservoirAbove, Reservoir * reservoirBelow,
      const Interface::Snapshot * start, const Interface::Snapshot * end, Barrier * barrier)
{
#if 0
   cerr << GetRankString () << ": " << "collectAndMigrateExpelledCharges (" << reservoir->getName () << ") starting" << endl;
#endif

   Interface::FormationList * formations = getAllFormations ();

   bool withinRange = false; // whether we reached the formation just above the reservoir's formation

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      const Formation * formation = (Formation *) * formationIter;

      const Formation * formationBelow = formation->getBottomFormation ();

      const Formation * formationBelowBelow = 0;
      if (formationBelow) formationBelowBelow = formationBelow->getBottomFormation ();

#if 0
      cerr << "\tFormation: " << formation->getName () << (formation->isSourceRock () ? "(SR)" : "(--)");
#endif

      if (withinRange || formationBelow == reservoir->getFormation () || formationBelowBelow == reservoir->getFormation ())
      {
         withinRange = true;
#if 0
         cerr << " is within range" << endl;
#endif
      }
      else
      {
#if 0
         cerr << " is out of range" << endl;
#endif
         continue;
      }

      unsigned int directionsToCollect = EXPELLEDNONE;

      if (formationBelow == reservoir->getFormation () || formationBelowBelow == reservoir->getFormation ())
      {
         // source rock atmost 2 formations above reservoir
#if 0
         cerr << "\t\tand just above reservoir" << endl;
#endif
         // check if reservoirAbove is in the way of reservoir with respect to downward migration
         if (!reservoirAbove ||
               !reservoirAbove->isActive (end) ||
               ((reservoirAbove->getFormation () != formationBelow &&
                 reservoirAbove->getFormation () != formationBelowBelow) &&
                reservoirAbove->getFormation () != formation))
         {
            directionsToCollect |= EXPELLEDDOWNWARD;
         }
      }
      else if (formation == reservoir->getFormation ())
      {
         // reservoir in source rock
#if 0
         cerr << "\t\tand contains reservoir" << endl;
#endif
         // check if reservoirBelow is also in the source rock
         if (!reservoirBelow || !reservoirBelow->isActive (end) || reservoirBelow->getFormation () != formation)
         {
            directionsToCollect |= EXPELLEDDOWNWARD;
         }
         // check if reservoirAbove is also in the source rock
         if (!reservoirAbove || !reservoirAbove->isActive (end) || reservoirAbove->getFormation () != formation)
         {
            directionsToCollect |= EXPELLEDUPWARD;
         }
      }
      else if (reservoirBelow &&
            (formationBelow == reservoirBelow->getFormation () ||
             formationBelowBelow == reservoirBelow->getFormation ()))
      {
         // source rock just above reservoir below
#if 0
         cerr << "\t\tand just above reservoir below" << endl;
#endif
         directionsToCollect |= EXPELLEDUPWARD;

         if (!reservoirBelow->isActive (end))
         {
            directionsToCollect |= EXPELLEDDOWNWARD;
         }
      }
      else
      {
#if 0
         cerr << "\t\tand comfortably below reservoir and above reservoir below" << endl;
#endif
         // comfortably below reservoir and above reservoirBelow
         directionsToCollect |= EXPELLEDDOWNWARD;
         directionsToCollect |= EXPELLEDUPWARD;
      }


      if (formation->isSourceRock ())
      {
         reservoir->collectExpelledCharges (formation, directionsToCollect, barrier);
         reservoir->migrateChargesToBeMigrated (formation, 0);
      }

      if (barrier && (directionsToCollect & EXPELLEDUPWARD)) barrier->updateBlocking (formation, end);

      if (reservoirBelow && reservoirBelow->isActive (end) &&
            formationBelow == reservoirBelow->getFormation ())
      {
#if 0
         cerr << "\t\tand last within range" << endl;
#endif
         break;
      }
   }

#if 0
   cerr << GetRankString () << ": " << "collectAndMigrateExpelledCharges (" << reservoir->getName () << ") finished" << endl;
#endif
   return false;
}

void Migrator::deleteExpelledChargeMaps (const Interface::Snapshot * snapshot)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      const Interface::Formation * formation = * formationIter;
      if (formation->isSourceRock ())
      {
         for (int componentId = FIRST_COMPONENT; componentId < NUM_COMPONENTS; ++componentId)
         {
            if (!ComponentsUsed[componentId]) continue;

            string propertyName = ComponentNames[componentId];
            propertyName += "ExpelledCumulative";

            const Interface::GridMap * gridMapStart = getPropertyGridMap (propertyName, snapshot, 0, formation, 0);

            if (gridMapStart) delete gridMapStart;
         }
      }
   }
}

Interface::FormationList * Migrator::getAllFormations (void) const
{
   if (!m_formations)
   {
      m_formations = Interface::ProjectHandle::getFormations ();
   }
   return m_formations;
}

Interface::ReservoirList * Migrator::getReservoirs (const Interface::Formation * formation) const
{
   if (formation)
   {
      Interface::ReservoirList * reservoirs = Interface::ProjectHandle::getReservoirs (formation);
      return reservoirs;
   }
   else
   {
      if (!m_reservoirs)
      {
         m_reservoirs = Interface::ProjectHandle::getReservoirs (0);

#if 0
         Interface::ReservoirList::iterator reservoirIter;
         cerr << GetRankString () << ": " << "Before sorting: " << endl;
         for (reservoirIter = m_reservoirs->begin (); reservoirIter != m_reservoirs->end (); ++reservoirIter)
         {
            const Reservoir * reservoir = (const Reservoir *) * reservoirIter;
            assert (reservoir);

            cerr << GetRankString () << ": " << "Reservoir: " << reservoir->getName () << endl;
         }

         cerr << GetRankString () << ": " << "Sorting ....." << endl;
#endif
         sort (m_reservoirs->begin (), m_reservoirs->end (), reservoirSorter);

#if 0
         cerr << GetRankString () << ": " << "After sorting: " << endl;
         for (reservoirIter = m_reservoirs->begin (); reservoirIter != m_reservoirs->end (); ++reservoirIter)
         {
            const Reservoir * reservoir = (const Reservoir *) * reservoirIter;
            assert (reservoir);

            cerr << GetRankString () << ": " << "Reservoir: " << reservoir->getName () << endl;
         }
#endif
      }

      return m_reservoirs;
   }
}

void Migrator::addTrapRecord (Reservoir * reservoir, TrapPropertiesRequest & tpRequest)
{
   if (!m_trapIoTbl) m_trapIoTbl = getTable ("TrapIoTbl");
   assert (m_trapIoTbl);
   Record * trapIoRecord = m_trapIoTbl->createRecord ();
   assert (trapIoRecord);

   database::setReservoirName (trapIoRecord, reservoir->getName ());
   database::setAge (trapIoRecord, reservoir->getEnd ()->getTime ());
   database::setTrapID (trapIoRecord, tpRequest.id);

   double posI = 0, posJ = 0;
   getActivityOutputGrid ()->getPosition ((unsigned int) tpRequest.i, (unsigned int) tpRequest.j, posI, posJ);

   database::setXCoord (trapIoRecord, posI);
   database::setYCoord (trapIoRecord, posJ);

   database::setTrapCapacity (trapIoRecord, tpRequest.capacity);
   database::setDepth (trapIoRecord, tpRequest.depth);
   database::setSpillDepth (trapIoRecord, tpRequest.spillDepth);

   double spillPointPosI = 0, spillPointPosJ = 0;
   getActivityOutputGrid ()->getPosition ((unsigned int) tpRequest.spillPointI, (unsigned int) tpRequest.spillPointJ, spillPointPosI, spillPointPosJ);

   database::setSpillPointXCoord (trapIoRecord, spillPointPosI);
   database::setSpillPointYCoord (trapIoRecord, spillPointPosJ);

   database::setWCSurface (trapIoRecord, tpRequest.wcSurface);
   database::setTemperature (trapIoRecord, tpRequest.temperature);
   database::setPressure (trapIoRecord, tpRequest.pressure);
   database::setPermeability (trapIoRecord, tpRequest.permeability);
   database::setSealPermeability (trapIoRecord, tpRequest.sealPermeability);
   database::setFracturePressure (trapIoRecord, tpRequest.fracturePressure);
   database::setNetToGross (trapIoRecord, tpRequest.netToGross);
   database::setCEPGas (trapIoRecord, tpRequest.cep[GAS]);
   database::setCEPOil (trapIoRecord, tpRequest.cep[OIL]);
   database::setCriticalTemperatureGas (trapIoRecord, tpRequest.criticalTemperature[GAS]);
   database::setCriticalTemperatureOil (trapIoRecord, tpRequest.criticalTemperature[OIL]);
   database::setInterfacialTensionGas (trapIoRecord, tpRequest.interfacialTension[GAS]);
   database::setInterfacialTensionOil (trapIoRecord, tpRequest.interfacialTension[OIL]);
   database::setFractSealStrength (trapIoRecord, tpRequest.fractureSealStrength);
   database::setGOC (trapIoRecord, tpRequest.goc);
   database::setOWC (trapIoRecord, tpRequest.owc);
   database::setVolumeGas (trapIoRecord, tpRequest.volume[GAS]);
   database::setVolumeOil (trapIoRecord, tpRequest.volume[OIL]);

   for (unsigned int i = 0; i < NumComponents; ++i)
   {
      string fieldName = "Mass";
      fieldName += TableComponentNames[i];
      trapIoRecord->setValue (fieldName, tpRequest.composition.getWeight ((ComponentId) i));
   }
}

database::Record * Migrator::copyMigrationRecord (database::Record * oldRecord, const string & newMigrationProcess)
{
   static int index = 0;
   database::Record * newRecord = m_migrationIoTbl->createRecord ();
   assert (newRecord);

   cerr << ++index << ": Creating MigrationIoRecord " << m_migrationIoTbl->size () << ": " << newMigrationProcess << endl;

   setMigrationProcess (newRecord, newMigrationProcess);

   setSourceAge (newRecord, getSourceAge (oldRecord));
   setSourceRockName (newRecord, getSourceRockName (oldRecord));
   setSourceReservoirName (newRecord, getSourceReservoirName (oldRecord));
   setSourceTrapID (newRecord, getSourceTrapID (oldRecord));
   setSourcePointX (newRecord, getSourcePointX (oldRecord));
   setSourcePointY (newRecord, getSourcePointY (oldRecord));

   setDestinationAge (newRecord, getDestinationAge (oldRecord));
   setDestinationReservoirName (newRecord, getDestinationReservoirName (oldRecord));
   setDestinationTrapID (newRecord, getDestinationTrapID (oldRecord));
   setDestinationPointX (newRecord, getDestinationPointX (oldRecord));
   setDestinationPointY (newRecord, getDestinationPointY (oldRecord));

   for (unsigned int component = 0; component < NumComponents; ++component)
   {
      string fieldName = "Mass";
      fieldName += TableComponentNames[component];

      double mass = oldRecord->getValue (fieldName, (double *) 0);
      newRecord->setValue (fieldName, mass);
   }

   return newRecord;
}

void Migrator::addMigrationRecord (const string & srcReservoirName, const string & srcFormationName, 
      const string & dstReservoirName,
      MigrationRequest & mr)
{
   assert (GetRank () == 0);

   if (!m_migrationIoTbl) m_migrationIoTbl = getTable ("MigrationIoTbl");
   assert (m_migrationIoTbl);
   
   database::Record * record;

   bool newlyCreated = false;

   if ((record = findMigrationRecord (srcReservoirName, srcFormationName, dstReservoirName, mr)) == 0)
   {
      record = createMigrationRecord (srcReservoirName, srcFormationName, dstReservoirName, mr);
      newlyCreated = true;
   }
   assert (record);

   for (unsigned int component = 0; component < NumComponents; ++component)
   {
      string fieldName = "Mass";
      fieldName += TableComponentNames[component];

      double mass = (newlyCreated ? 0 : record->getValue (fieldName, (double *) 0));
      mass += mr.composition.getWeight ((ComponentId) component);

      record->setValue (fieldName, mass);
   }
}

database::Record * Migrator::createMigrationRecord (const string & srcReservoirName, const string & srcFormationName, 
      const string & dstReservoirName,
      MigrationRequest & mr)
{
   assert (GetRank () == 0);

   database::Record * record = m_migrationIoTbl->createRecord ();
   assert (record);

   setMigrationProcess (record, MigrationProcessNames[mr.process]);

   setSourceAge (record, mr.source.age);
   setSourceRockName (record, srcFormationName);
   setSourceReservoirName (record, srcReservoirName);
   setSourceTrapID (record, mr.source.trapId);
   setSourcePointX (record, mr.source.x);
   setSourcePointY (record, mr.source.y);

   setDestinationAge (record, mr.destination.age);
   setDestinationReservoirName (record, dstReservoirName);
   setDestinationTrapID (record, mr.destination.trapId);
   setDestinationPointX (record, mr.destination.x);
   setDestinationPointY (record, mr.destination.y);

   m_migrationRecordLists[mr.process].push_back (record);
   return record;
}

database::Record * Migrator::findMigrationRecord (const string & srcReservoirName, const string & srcFormationName, 
      const string & dstReservoirName,
      MigrationRequest & mr)
{
   assert (GetRank () == 0);

   vector<database::Record *> & recordList = m_migrationRecordLists[mr.process];
   vector<database::Record *>::iterator iter;
   for (iter = recordList.begin (); iter != recordList.end (); ++iter)
   {
      database::Record * record = * iter;

      if (getSourceRockName (record) != srcFormationName) continue;

      if (getSourceReservoirName (record) != srcReservoirName) continue;
      if (getDestinationReservoirName (record) != dstReservoirName) continue;

      if (getSourceTrapID (record) != mr.source.trapId) continue;
      if (getDestinationTrapID (record) != mr.destination.trapId) continue;

      if (getSourcePointX (record) != mr.source.x) continue;
      if (getDestinationPointX (record) != mr.destination.x) continue;

      if (getSourcePointY (record) != mr.source.y) continue;
      if (getDestinationPointY (record) != mr.destination.y) continue;

      return record;
   }
   return 0;
}

bool MigrationIoTblSorter (database::Record * recordL,  database::Record * recordR)
{
   static int calls = 0;

   ++calls;

   if (recordL == 0 && recordR == 0)
   {
      cerr << "recordL = 0 && recordR = 0 in iteration " << calls << endl;
      return false;
   }

   if (recordR == 0)
   {
      cerr << "recordR = 0 in iteration " << calls << endl;
      return true;
   }
   if (recordL == 0)
   {
      cerr << "recordL = 0 in iteration " << calls << endl;
      return false;
   }

   if (getSourceAge (recordL) != getSourceAge (recordR))
      return (getSourceAge (recordL) > getSourceAge (recordR));

   if (getDestinationAge (recordL) != getDestinationAge (recordR))
      return (getDestinationAge (recordL) < getDestinationAge (recordR));

   if (getMigrationProcess (recordL) != getMigrationProcess (recordR))
      return (getMigrationProcess (recordL) < getMigrationProcess (recordR));

   if (getSourceRockName (recordL) != getSourceRockName (recordR))
      return (getSourceRockName (recordL) < getSourceRockName (recordR));

   if (getSourceReservoirName (recordL) != getSourceReservoirName (recordR))
      return (getSourceReservoirName (recordL) < getSourceReservoirName (recordR));

   if (getDestinationReservoirName (recordL) != getDestinationReservoirName (recordR))
      return (getDestinationReservoirName (recordL) < getDestinationReservoirName (recordR));

   if (getSourceTrapID (recordL) != getSourceTrapID (recordR))
      return (getSourceTrapID (recordL) < getSourceTrapID (recordR));

   if (getDestinationTrapID (recordL) != getDestinationTrapID (recordR))
      return (getDestinationTrapID (recordL) < getDestinationTrapID (recordR));

   return true;
}
   
void Migrator::sortMigrationRecords (void)
{
   // cerr << "Start sorting MigrationIoTbl" << endl;
   m_migrationIoTbl->stable_sort (MigrationIoTblSorter);
   // cerr << "Finish sorting MigrationIoTbl" << endl;
}

bool MigrationIoTblIsEqual (database::Record * recordL,  database::Record * recordR)
{
   if (getSourceAge (recordL) != getSourceAge (recordR))
      return false;

   if (getDestinationAge (recordL) != getDestinationAge (recordR))
      return false;

   if (getMigrationProcess (recordL) != getMigrationProcess (recordR))
      return false;

   if (getSourceRockName (recordL) != getSourceRockName (recordR))
      return false;

   if (getSourceReservoirName (recordL) != getSourceReservoirName (recordR))
      return false;

   if (getDestinationReservoirName (recordL) != getDestinationReservoirName (recordR))
      return false;

   if (getSourceTrapID (recordL) != getSourceTrapID (recordR))
      return false;

   if (getDestinationTrapID (recordL) != getDestinationTrapID (recordR))
      return false;

   return true;
}

bool MigrationIoTblMerge (database::Record * recordL,  database::Record * recordR)
{
   for (unsigned int component = 0; component < NumComponents; ++component)
   {
      string fieldName = "Mass";
      fieldName += TableComponentNames[component];

      double massL = recordL->getValue (fieldName, (double *) 0);
      double massR = recordR->getValue (fieldName, (double *) 0);

      recordL->setValue (fieldName, massL + massR);
   }

   return true;
}

void Migrator::uniqueMigrationRecords (void)
{
   // cerr << "Start uniquefying MigrationIoTbl" << endl;
   m_migrationIoTbl->unique (MigrationIoTblIsEqual, MigrationIoTblMerge);
   // cerr << "Finish uniquefying MigrationIoTbl" << endl;
}

void Migrator::checkMigrationRecords (void)
{
   // cerr << "Start checking MigrationIoTbl" << endl;
   database::Table::iterator iter;
   string migrationProcess;

   // cerr << "MigrationIoTbl size = " << m_migrationIoTbl->size () << endl;
   int index = 0;
   for (iter = m_migrationIoTbl->begin (), index = 0; iter != m_migrationIoTbl->end (); ++iter)
   {
      if (* iter == 0)
      {
         cerr << "Error: MigrationIoTbl[" << index << "] = 0" << endl;
      }
      else if (getSourceAge (* iter) < 0 || getDestinationAge (* iter) < 0)
      {
         cerr << "Age < 0" << endl;
      }
      else if (getMigrationProcess (*iter) == "")
      {
         cerr << "MigrationProcess = \"\"" << endl;
      }
      else ++index;
   }
   // cerr << "MigrationIoTbl counted = " << index << endl;;
   // cerr << "Finish checking MigrationIoTbl" << endl;

}

// Re-label or remove records of which source and/or destination trap id was changed to -10
void Migrator::sanitizeMigrationRecords (void)
{
   database::Table::iterator iter;
   database::Table::iterator nextIter;

   // cerr << "Start sanitizing MigrationIoTbl" << endl;
   for (iter = m_migrationIoTbl->begin (); iter != m_migrationIoTbl->end (); iter = nextIter)
   {
      database::Record * record = * iter;
      const string & migrationProcess = getMigrationProcess (record);
      int sourceTrapId = getSourceTrapID (record);
      int destinationTrapId = getDestinationTrapID (record);

      if (migrationProcess == MigrationProcessNames[REMIGRATION])
      {
         if (destinationTrapId < 0)
         {
            setMigrationProcess (record, MigrationProcessNames[REMIGRATIONLEAKAGE]);
         }
      }
      else if (migrationProcess == MigrationProcessNames[REMIGRATIONLEAKAGE])
      {
      }
      else if (migrationProcess == MigrationProcessNames[LEAKAGEFROMTRAP])
      {
         if (sourceTrapId < 0)
         {
            nextIter = m_migrationIoTbl->deleteRecord (iter);
            continue;
         }
      }
      else if (migrationProcess == MigrationProcessNames[LEAKAGETOTRAP])
      {
         if (destinationTrapId < 0)
         {
            setMigrationProcess (record, MigrationProcessNames[THROUGHLEAKAGE]);
         }
      }
      else if (migrationProcess == MigrationProcessNames[EXPULSION])
      {
         if (destinationTrapId < 0)
         {
            setMigrationProcess (record, MigrationProcessNames[EXPULSIONLEAKAGE]);
         }
      }
      else if (migrationProcess == MigrationProcessNames[ABSORPTION])
      {
      }
      else if (migrationProcess == MigrationProcessNames[EXPULSIONLEAKAGE])
      {
      }
      else if (migrationProcess == MigrationProcessNames[SPILL])
      {
         if (sourceTrapId < 0)
         {
            // shouldn't happen!!!

            nextIter = m_migrationIoTbl->deleteRecord (iter);
            continue;
         }
         else if (sourceTrapId == destinationTrapId)
         {
            nextIter = m_migrationIoTbl->deleteRecord (iter);
            continue;
         }
         else if (destinationTrapId < 0)
         {
            setMigrationProcess (record, MigrationProcessNames[SPILLUPOROUT]);
         }
      }
      else if (migrationProcess == MigrationProcessNames[SPILLUPOROUT])
      {
         if (sourceTrapId < 0)
         {
            nextIter = m_migrationIoTbl->deleteRecord (iter);
            continue;
         }
      }
      else if (migrationProcess == MigrationProcessNames[THROUGHLEAKAGE])
      {
      }
      else if (migrationProcess == MigrationProcessNames[BIODEGRADATION] ||
	    migrationProcess == MigrationProcessNames[DIFFUSION] ||
	    migrationProcess == MigrationProcessNames[OILTOGASCRACKINGLOST] ||
	    migrationProcess == MigrationProcessNames[OILTOGASCRACKINGGAINED])
      {
         if (sourceTrapId < 0)
         {
            nextIter = m_migrationIoTbl->deleteRecord (iter);
            continue;
         }
      }
      nextIter = iter + 1;
   }
   // cerr << "Finish sanitizing MigrationIoTbl" << endl;
}

void Migrator::clearMigrationRecordLists (void)
{
   assert (GetRank () == 0);
   for (int i = 0; i < NUMBEROFPROCESSES; ++i)
   {
      m_migrationRecordLists[i].clear ();
   }
}

void Migrator::renumberMigrationRecordTrap (const Interface::Snapshot * snapshot, int oldTrapNumber, int newTrapNumber)
{
#if 0
   cerr << snapshot->getTime () << ": Renumbering MigrationIoRecordTrap " << oldTrapNumber << " to " << newTrapNumber << endl;
#endif
   
   assert (GetRank () == 0);
   for (int i = 0; i < NUMBEROFPROCESSES; ++i)
   {
      vector<database::Record *> & recordList = m_migrationRecordLists[i];
      vector<database::Record *>::iterator iter;
      for (iter = recordList.begin (); iter != recordList.end (); ++iter)
      {
         database::Record * record = * iter;
         if (getSourceTrapID (record) == oldTrapNumber && findSnapshot (getSourceAge (record)) == snapshot)
         {
            setSourceTrapID (record, newTrapNumber);
         }

         if (getDestinationTrapID (record) == oldTrapNumber && findSnapshot (getDestinationAge (record)) == snapshot)
         {
            setDestinationTrapID (record, newTrapNumber);
         }
      }
   }
}

const Interface::GridMap * Migrator::getPropertyGridMap (const string & propertyName,
      const Interface::Snapshot * snapshot,
      const Interface::Reservoir * reservoir, const Interface::Formation * formation, const Interface::Surface * surface) const
{
   int selectionFlags = 0;

   if (reservoir) selectionFlags |= Interface::RESERVOIR;
   if (formation && !surface) selectionFlags |= Interface::FORMATION;
   if (surface && !formation) selectionFlags |= Interface::SURFACE;
   if (formation && surface) selectionFlags |= Interface::FORMATIONSURFACE;

   Interface::PropertyValueList * propertyValues = getPropertyValues (selectionFlags,
         findProperty (propertyName),
         snapshot, reservoir, formation, surface,
         Interface::MAP);

   if (propertyValues->size () != 1)
   {
      return 0;
   }

   const Interface::GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

   delete propertyValues;
   return gridMap;
}

bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2)
{
#if 0
   cerr << GetRankString () << ": " << "Depth (" << reservoir1->getName () << ") = " << ((migration::Reservoir *) reservoir1)->getAverageDepth ();
   cerr << GetRankString () << ": " << "\tDepth (" << reservoir2->getName () << ") = " << ((migration::Reservoir *) reservoir2)->getAverageDepth ();
   cerr << GetRank () << ": " << endl;
#endif
   
   return ((migration::Reservoir *) reservoir1)->getAverageDepth () > ((migration::Reservoir *) reservoir2)->getAverageDepth ();
}


