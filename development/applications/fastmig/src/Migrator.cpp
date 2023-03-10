//
// Copyright (C) 2010-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef WIN32
// Uncomment this steatment to print memory consumption
// #define DEBUGMEMORY
#endif

#ifdef DEBUGMEMORY
#include <sstream>
#endif

// boost
#include <boost/lexical_cast.hpp>

// std library
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include "petsc.h"

// TableIO library
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

#include "Migrator.h"
#include "MigrationReservoir.h"
#include "Formation.h"
#include "MigrationSurface.h"

#include "LogHandler.h"

#ifdef USEOTGC
#include "OilToGasCracker.h"
#endif

#include "RequestDefs.h"
#include "Barrier.h"

#include "rankings.h"

using namespace migration;

#include "GeoPhysicsProjectHandle.h"
#include "ProjectHandle.h"
#include "Reservoir.h"
#include "ReservoirOptions.h"
#include "PropertyValue.h"
#include "Grid.h"
#include "GridMap.h"
#include "Snapshot.h"
#include "SimulationDetails.h"
#include "RunParameters.h"
#include "MassBalance.h"
#include "utils.h"

#include "h5_parallel_file_types.h"
#include "FilePath.h"

#ifndef _MSC_VER
#include "h5merge.h"
#endif
//utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::DefaultNumericalTolerance;
#include "NumericFunctions.h"

using namespace DataAccess;

static bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2);

extern std::string NumProcessorsArg;

Migrator::Migrator (const std::string & name) :
  m_objectFactory(this)
{
   m_projectHandle.reset (dynamic_cast<GeoPhysics::ProjectHandle *> (Interface::OpenCauldronProject (name, &m_objectFactory, getOutputTableNames())));

   if (!m_projectHandle.get ())
   {
      std::cerr << "Basin_Error: Can't open input project file: " << name << std::endl;
      exit (-1);
   }

   m_massBalance = nullptr;
   m_propertyManager.reset (new MigrationPropertyManager (*m_projectHandle));

   m_reservoirs = nullptr;
   m_formations = nullptr;


   InitializeRequestTypes ();
   if (GetRank () == 0)
   {
      m_migrationRecordLists = new std::vector<database::Record *>[NUMBEROFPROCESSES];
   }

   m_migrationIoTbl = nullptr;
   m_trapIoTbl = nullptr;
   m_ReservoirIoTbl = nullptr;

#ifdef USEOTGC
   bool includeSulphur = m_projectHandle->containsSulphur ();
   m_otgc = new OilToGasCracker (includeSulphur);
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
      delete[] m_migrationRecordLists;
   }
}

std::vector<std::string> Migrator::getOutputTableNames ( ) const {

  std::vector<std::string> outputTableNames;

   const int MaximumNumberOfOutputTables = 256;

   PetscBool outputTablesDefined = PETSC_FALSE;
   char* outputTableNamesArray [ MaximumNumberOfOutputTables ];
   int numberOfOutputTables = MaximumNumberOfOutputTables;

   PetscOptionsGetStringArray (PETSC_IGNORE, PETSC_IGNORE, "-outtabs", outputTableNamesArray, &numberOfOutputTables, &outputTablesDefined );

   if ( outputTablesDefined ) {

      for ( int i = 0; i < numberOfOutputTables; ++i ) {
         outputTableNames.push_back ( outputTableNamesArray [ i ]);
      }

   }

   return outputTableNames;
}

bool Migrator::saveTo (const std::string & outputFileName)
{
   return m_projectHandle->saveToFile (outputFileName);
}

GeoPhysics::ProjectHandle& Migrator::getProjectHandle()
{
   return *m_projectHandle;
}

bool Migrator::compute (const bool overpressuredLeakage)
{
   std::string activityName = MigrationActivityName;
   activityName += NumProcessorsArg;
   H5_Parallel_PropertyList::setOtherFileProcessOptions(false);

   // let's first clean out the mess from a previous run
   if (GetRank () == 0)
   {
      m_projectHandle->deletePropertyValues (DataAccess::Interface::RESERVOIR);

      m_projectHandle->deletePropertyValues (DataAccess::Interface::FORMATION, m_projectHandle->findProperty("FlowDirectionIJK"), 0, 0, 0, 0, DataAccess::Interface::VOLUME);

      if (!m_trapIoTbl) m_trapIoTbl = m_projectHandle->getTable ("TrapIoTbl");
      m_trapIoTbl->clear ();

      if (!m_migrationIoTbl) m_migrationIoTbl = m_projectHandle->getTable ("MigrationIoTbl");
      m_migrationIoTbl->clear ();

      m_projectHandle->deleteTrappers ();
      database::Table * trapperIoTbl = m_projectHandle->getTable ("TrapperIoTbl");
      trapperIoTbl->clear ();
   }

   bool started = m_projectHandle->startActivity (activityName, m_projectHandle->getHighResolutionOutputGrid ());
   if (!started) return false;

   ios::fmtflags f (std::cout.flags ());
   std::cout << std::setfill (' ');
   std::cout.flags (f);

   if (!started) return false;

   openMassBalanceFile ();

   ComputeRanks (m_projectHandle->getActivityOutputGrid ());

   m_advancedMigration = m_projectHandle->getRunParameters ()->getAdvancedMigration ();
   m_hdynamicAndCapillary = m_projectHandle->getRunParameters ()->getHydrodynamicCapillaryPressure ();
   m_reservoirDetection = m_projectHandle->getRunParameters ()->getReservoirDetection ();
   m_paleoSeeps = m_projectHandle->getRunParameters ()->getPaleoSeeps ();
   m_overpressuredLeakage = overpressuredLeakage;
   if (!m_advancedMigration)
   {
      m_hdynamicAndCapillary = false;
      m_reservoirDetection = false;
      m_paleoSeeps = false;
   }

   if (!setUpBasinGeometry ()) return false;

   getMinimumColumnHeights ();

   bool overPressureRun = !isHydrostaticCalculation ();

   createFormationNodes();
   if (!computeFormationPropertyMaps(m_projectHandle->getSnapshots()->front(), overPressureRun)) return false;

   if (m_reservoirDetection)
   {
      if (m_ReservoirIoTbl == nullptr) m_ReservoirIoTbl = m_projectHandle->getTable("ReservoirIoTbl");
      assert(m_ReservoirIoTbl);
      m_ReservoirIoTbl->clear();

      m_projectHandle->deleteReservoirs();
   }

   // compute the positions of the reservoirs within the formations
   if (!computeDepthOffsets()) return false;
   if (!computeNetToGross()) return false;

   // delete the maps created for computeNetToGross
   m_propertyManager->removeProperties (m_projectHandle->getSnapshots ()->front ());
   // delete the formation property grid maps
   deleteFormationPropertyMaps ();

   PetscBool minorSnapshots, genexOnTheFly;

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-genex", &genexOnTheFly);
   m_genexOnTheFly = (genexOnTheFly == PETSC_TRUE);

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-minor", &minorSnapshots);

   Interface::SnapshotList * snapshots = m_projectHandle->getSnapshots (minorSnapshots ? (Interface::MAJOR | Interface::MINOR)
                                                                        : Interface::MAJOR);

   Interface::SnapshotList::reverse_iterator snapshotIter;

   const Interface::Snapshot * start;
   const Interface::Snapshot * end = 0;

   // skip the first as it does not contain meaningful data.
   for (snapshotIter = snapshots->rbegin (), ++snapshotIter; snapshotIter != snapshots->rend (); ++snapshotIter)
   {
      start = end;
      end = *snapshotIter;
      if (!start) continue;

      if (!performSnapshotMigration (start, end, overPressureRun))
         return false;
   }


   delete snapshots;

   closeMassBalanceFile ();

   m_projectHandle->finishActivity ();

   //Specify the simulation details
   std::string simulatorMode;

   if (m_advancedMigration and !m_hdynamicAndCapillary)
   {
      simulatorMode += simulationModeStr[3];
      simulatorMode += " ";
   }

   if (!m_advancedMigration)
   {
      simulatorMode += simulationModeStr[0];
      simulatorMode += " ";
   }
   if (m_hdynamicAndCapillary)
   {
      simulatorMode += simulationModeStr[1];
      simulatorMode += " ";
   }
   if (m_reservoirDetection)
   {
      simulatorMode += simulationModeStr[2];
      simulatorMode += " ";
   }

   m_projectHandle->setSimulationDetails ("fastmig", simulatorMode, "");

   bool status = true;
   if (!mergeOutputFiles ())
   {
      PetscPrintf (PETSC_COMM_WORLD, "Basin_Error: Unable to merge output files\n");
      status = false;
   }

   migration::deleteRanks ();

   return status;
}

void Migrator::openMassBalanceFile (void)
{
   if (GetRank () == 0)
   {
      std::string fileName = utils::getProjectBaseName (m_projectHandle->getProjectName ());
      fileName += "_MassBalance";

      m_massBalanceFile.open (fileName.c_str (), ios::out);
      m_massBalance = new MassBalance<ofstream> (m_massBalanceFile);
   }
}

void Migrator::closeMassBalanceFile (void)
{
   if (GetRank () == 0)
   {
      m_massBalanceFile.close ();
      delete m_massBalance;
      m_massBalance = 0;
   }
}

bool Migrator::setUpBasinGeometry (void)
{
   bool HydrostaticCalculation = isHydrostaticCalculation ();

   // From GeoPhysics::ProjectHandle
   if (!m_projectHandle->initialise (true) or
       !m_projectHandle->setFormationLithologies (true, true) or
       !m_projectHandle->initialiseLayerThicknessHistory (!HydrostaticCalculation) or // Backstripping
       !m_projectHandle->applyFctCorrections ())
      return false;
   else
      return true;
}

bool Migrator::isHydrostaticCalculation (void) const
{
   bool hydrostaticCalculation = false;
   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastSimulation ("fastcauldron");

   if (lastFastcauldronRun != 0)
   {
      hydrostaticCalculation = lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" or
         lastFastcauldronRun->getSimulatorMode () == "HydrostaticHighResDecompaction" or
         lastFastcauldronRun->getSimulatorMode () == "HydrostaticDarcy";
   }

   return hydrostaticCalculation;
}

bool Migrator::computeFormationPropertyMaps (const Interface::Snapshot * snapshot, bool isOverPressureRun)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;

   Interface::GridMap * topDepthGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, m_projectHandle->getActivityOutputGrid (), Interface::DefaultUndefinedMapValue, 1);

   if (!getSeaBottomDepths (topDepthGridMap, snapshot))
   {
      return false;
   }

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation *formation = MigrationFormation::CastToFormation (*formationIter);

      assert (formation);
      if (!formation->isActive (snapshot)) continue;

      bool islowResEqualsHighRes = ((*(m_projectHandle->getLowResolutionOutputGrid ())) == (*(m_projectHandle->getHighResolutionOutputGrid ())));

      formation->computePropertyMaps (topDepthGridMap, snapshot, islowResEqualsHighRes, isOverPressureRun, m_projectHandle->getRunParameters ()->getNonGeometricLoop (),
                                      m_projectHandle->getRunParameters ()->getChemicalCompaction ()); // allowed to fail

      formation->computeHCDensityMaps (snapshot);

      if (!formation->computeCapillaryPressureMaps (topDepthGridMap, snapshot))
         return false;
   }

   if (topDepthGridMap) topDepthGridMap->release ();

   return true;

}

bool Migrator::getSeaBottomDepths (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot)
{
   if (!topDepthGridMap->retrieveData ()) return false;
   const Interface::Grid * grid = m_projectHandle->getActivityOutputGrid ();
   for (unsigned int i = (unsigned int)grid->firstI (); i <= (unsigned int) grid->lastI (); ++i)
   {
      for (unsigned int j = (unsigned int) grid->firstJ (); j <= (unsigned int)grid->lastJ (); ++j)
      {
         double seaBottomDepth = m_projectHandle->getSeaBottomDepth (i, j, snapshot->getTime ());

         if ( NumericFunctions::isEqual(seaBottomDepth, Interface::DefaultUndefinedMapValue, DefaultNumericalTolerance) or
              NumericFunctions::isEqual(seaBottomDepth, Interface::DefaultUndefinedScalarValue, DefaultNumericalTolerance))
         {
            topDepthGridMap->setValue (i, j, Interface::DefaultUndefinedMapValue);
         }
         else
         {
             topDepthGridMap->setValue(i, j, seaBottomDepth);
         }
      }
   }
   if (!topDepthGridMap->restoreData (true)) return false;

   return true;
}

Interface::FormationList * Migrator::getAllFormations (void) const
{
   if (!m_formations)
   {
      m_formations = m_projectHandle->getFormations ();
   }
   return m_formations;
}

bool Migrator::createFormationNodes (void)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);

      assert (formation);

      formation->createFormationNodes ();
   }
   return true;
}

bool Migrator::performSnapshotMigration (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool overPressureRun)
{
   bool sourceRockActive = false;
   migration::MigrationFormation * bottomSourceRock = getBottomSourceRockFormation ();

	 if (bottomSourceRock != nullptr)
		 sourceRockActive = bottomSourceRock->isActive(end);

   if ((activeReservoirs (start) or m_reservoirDetection or m_paleoSeeps or end->getTime () == 0.0) and sourceRockActive)
   {
      if (GetRank () == 0)
         std::cout << "Processing snapshot " << end->getTime () << std::endl;

      clearFormationNodeProperties ();

      if (!computeFormationPropertyMaps (end, overPressureRun) or
          !retrieveFormationPropertyMaps (end) or
          !computeFormationNodeProperties (end) or
          !flagTopNodes(end, overPressureRun) or
          !detectReservoirs (start, end, overPressureRun) or
          !computeSMFlowPaths (start, end) or
          !restoreFormationPropertyMaps (end) or
          !loadExpulsionMaps (start, end) or
          !chargeReservoirs (start, end) or
          !calculateSeepage (start, end) or
          !unloadExpulsionMaps (end) or
          !saveSMFlowPaths (start, end))
      {
         return false;
      }

      deleteFormationPropertyMaps ();
   }

   m_projectHandle->continueActivity ();

#ifdef DEBUGMEMORY
   std::stringstream ss;
   ss << " rank " << GetRank( ) << " " << getMemoryUsed( );
   std::string mystring1( " Before deleting properties: " );
   cerr << mystring1 + ss.str( ) << endl;
#endif

   // the properties are computed at the end
   m_propertyManager->removeProperties (end);
   m_projectHandle->deletePropertyValueGridMaps (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE | Interface::RESERVOIR,
                                                 0, end, 0, 0, 0, Interface::MAP | Interface::VOLUME);

   // for extra cleaning we also delete at the start, data are saved in the columns anyway
   m_propertyManager->removeProperties (start);

   m_projectHandle->deletePropertyValueGridMaps (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE | Interface::RESERVOIR,
                                                 0, start, 0, 0, 0, Interface::MAP | Interface::VOLUME);


   return true;
}

/// compute the positions of the reservoirs within the formations
bool Migrator::computeDepthOffsets () const
{
   if (m_reservoirDetection)
    return true;

   for ( const Interface::Reservoir * reservoir : *getReservoirs ())
   {
      const MigrationReservoir * migrationReservoir = dynamic_cast<const MigrationReservoir*>( reservoir );

      if (!migrationReservoir || !migrationReservoir->computeDepthOffsets (m_projectHandle->findSnapshot (0.)))
      {
        return false;
      }
   }

   return true;
}

/// compute the nett/gross fractions
bool Migrator::computeNetToGross (void) const
{
   if (m_reservoirDetection)
      return true;

   for ( const Interface::Reservoir * reservoir : *getReservoirs ())
   {
      const MigrationReservoir * migrationReservoir = dynamic_cast<const MigrationReservoir*>( reservoir );

      if (!migrationReservoir || !migrationReservoir->computeNetToGross ())
      {
        return false;
      }
   }
   return true;
}

migration::MigrationFormation * Migrator::getFormation (int index)
{
   Interface::FormationList * formations = getAllFormations ();
   return MigrationFormation::CastToFormation ((*formations)[index]);
}

int Migrator::getIndex (migration::MigrationFormation * formation)
{
   int index;

   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;

   for (index = 0, formationIter = formations->begin (); formationIter != formations->end (); ++index, ++formationIter)
   {
      if (formation == *formationIter) return index;
   }
   return -1;
}

migration::MigrationReservoir * Migrator::getReservoir (int index)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();
   return (MigrationReservoir *)(*reservoirs)[index];
}

int Migrator::getIndex (migration::MigrationReservoir * reservoir)
{
   int index;

   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;

   for (index = 0, reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++index, ++reservoirIter)
   {
      if (reservoir == *reservoirIter) return index;
   }
   return -1;
}

bool Migrator::activeReservoirs (const Interface::Snapshot * snapshot)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      const MigrationReservoir * reservoir = (const MigrationReservoir *)* reservoirIter;

      assert (reservoir);

      if (reservoir->isActive (snapshot))
      {
         return true;
      }
   }
   return false;
}

void Migrator::clearFormationNodeProperties ()
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);

      formation->clearNodeProperties ();
   }
}

bool Migrator::retrieveFormationPropertyMaps (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   bool retrieveCapillary = (m_hdynamicAndCapillary or m_reservoirDetection);

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);

      if (!formation->isActive (end)) continue;

      if (!formation->retrievePropertyMaps (retrieveCapillary)) return false;
   }

   return true;
}

bool Migrator::restoreFormationPropertyMaps (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   bool restoreCapillary = (m_hdynamicAndCapillary or m_reservoirDetection);

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);
      if (!formation->isActive (end)) continue;

      formation->restorePropertyMaps (restoreCapillary);
   }

   return true;
}


bool Migrator::deleteFormationPropertyMaps ()
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);
      formation->deleteFormationProperties ();
   }

   return true;
}

bool Migrator::computeFormationNodeProperties (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);
      if (!formation->isActive (end)) continue;

      formation->computeFaults (end);
      formation->computeNodeProperties ();
   }

   return true;
}

migration::MigrationFormation * Migrator::getBottomSourceRockFormation ()
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   MigrationFormation * bottomSourceRockFormation = 0;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);

      if (formation->isSourceRock ()) bottomSourceRockFormation = formation;
   }

   return bottomSourceRockFormation;
}

migration::MigrationFormation * Migrator::getTopSourceRockFormation (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   MigrationFormation * topSourceRockFormation = 0;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);

      if (formation->isSourceRock () and formation->isActive (end))
      {
         topSourceRockFormation = formation;
         break;
      }
   }

   return topSourceRockFormation;
}

migration::MigrationFormation * Migrator::getTopActiveFormation (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);

      if (formation->isActive (end)) return formation;
   }

   return 0;
}

// Returns the top reservoir that has been deposited, is sealed and is active at snapshot "end"
migration::MigrationFormation * Migrator::getTopActiveReservoirFormation (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();
   Interface::ReservoirList::iterator reservoirIter;

   MigrationReservoir * topActiveReservoir = 0;
   MigrationFormation * topActiveReservoirFormation = 0;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      MigrationReservoir * reservoir = (MigrationReservoir *)(*reservoirIter);
      MigrationFormation * tempFormation = MigrationFormation::CastToFormation (reservoir->getFormation ());

      bool isDeposited = tempFormation->getTopSurface ()->getSnapshot ()->getTime () > end->getTime ();

      if (reservoir->isActive (start) and isDeposited)
      {
         topActiveReservoir = reservoir;
      }
   }

   if (topActiveReservoir)
   {
      topActiveReservoirFormation = MigrationFormation::CastToFormation (topActiveReservoir->getFormation ());
   }

   return topActiveReservoirFormation;
}

migration::MigrationFormation * Migrator::getBottomMigrationFormation(const Interface::Snapshot * end)
{
	MigrationFormation * bottomFormation;
	MigrationFormation *bottomSourceRockFormation = getBottomSourceRockFormation();
	if (bottomSourceRockFormation == 0) return 0;
	MigrationFormation * belowBelowSourceRockFormation = 0;
	MigrationFormation * belowSourceRockFormation = bottomSourceRockFormation->getBottomFormation();
	if (belowSourceRockFormation)
		belowBelowSourceRockFormation = belowSourceRockFormation->getBottomFormation();
	if (belowBelowSourceRockFormation != 0)
		bottomFormation = belowBelowSourceRockFormation; //+2
	else if (belowSourceRockFormation != 0)
		bottomFormation = belowSourceRockFormation;  // +1
	else
		bottomFormation = bottomSourceRockFormation; // SR

	return bottomFormation;
}

bool Migrator::flagTopNodes(const Interface::Snapshot * end, const bool overPressureRun)
{
	MigrationFormation * bottomFormation = getBottomMigrationFormation(end);
	MigrationFormation * topSealFormation = getTopActiveFormation(end);
	if (topSealFormation == 0 or bottomFormation == 0) return false;
	MigrationFormation *sealFormation;
	MigrationFormation *reservoirFormation;
	bool topSealFormationReached = false;
	for (reservoirFormation = bottomFormation, sealFormation = (MigrationFormation *)reservoirFormation->getTopFormation();
		 sealFormation != 0 and !topSealFormationReached;
		 reservoirFormation = sealFormation, sealFormation = (MigrationFormation *)sealFormation->getTopFormation())
	{
			//check if top seal formation is reached
		 topSealFormationReached = (sealFormation == topSealFormation);
			//Flag all top nodes of each formation
			reservoirFormation->detectReservoir(sealFormation, m_minOilColumnHeight, m_minGasColumnHeight, overPressureRun, topSealFormation);
	}
	return true;
}

/*
  Reservoir definition based on capillary pressure evaluation.
  Begin from bottom most active reservoir. Go up. Looping through
  the uppermost cells of each formation and check the capillary pressure jump
  ( difference between 0% saturation capillary pressure at current formation
  and 100% capillary pressure at lowermost cells of formation above ).
*/
bool Migrator::detectReservoirs (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool overPressureRun)
{
   MigrationFormation * bottomFormation = getBottomMigrationFormation(end);
   MigrationFormation * topSealFormation = getTopActiveFormation(end);
   if (topSealFormation == 0 or bottomFormation == 0) return false;

   MigrationFormation *reservoirFormation;
   MigrationFormation *sealFormation;
   Interface::ReservoirList *  reservoirsBeforeDetection = getReservoirs ();
   int numDetected = 0;


   bool topSealFormationReached = false;
   for (reservoirFormation = bottomFormation, sealFormation = (MigrationFormation *)reservoirFormation->getTopFormation ();
        sealFormation != 0 and !topSealFormationReached;
        reservoirFormation = sealFormation, sealFormation = (MigrationFormation *)sealFormation->getTopFormation ())
   {
      //check if top seal formation is reached
      topSealFormationReached = (sealFormation == topSealFormation);

      assert (reservoirFormation);

      //Assigns nodes of user-selected reservoirs as reservoir nodes, no need to identify the Reservoir here. Do this only for user-defined reservoirs
      std::unique_ptr<Interface::ReservoirList> alreadyReservoirFormation (getReservoirs (reservoirFormation));
      if (!alreadyReservoirFormation->empty () and !reservoirFormation->getDetectedReservoir () and
          alreadyReservoirFormation->front()->isActive(start))
      {
         reservoirFormation->identifyAsReservoir (m_advancedMigration);
         continue;
      }

      if (reservoirFormation->isSourceRock ()) continue;

      if (m_reservoirDetection)
      {
         // If the formation is already detected skip calculations otherwise detect crests that can hold hc
         if (reservoirFormation->detectReservoirCrests ())
         {
            if (!reservoirFormation->getDetectedReservoir ())
            {
               // cerr << "MigrationFormation " << reservoirFormation->getName() << " detected" << endl;
               // if the formation is detected as reservoir, add it in the reservoir list
               reservoirFormation->addDetectedReservoir (start);
               numDetected += 1;
            }
         }
         // print to file information about the reservoirs
         // reservoirFormation->saveReservoir (end);

         //setEndOfPath for detected reservoirs
         if (reservoirFormation->getDetectedReservoir ()) reservoirFormation->setEndOfPath ();
      }
   }

   // in case one or more formations have been detected, sort m_reservoirs
   if (numDetected > 0) sortReservoirs ();

   return true;
}

// Unflag detected reservoirs nodes

bool Migrator::computeSMFlowPaths (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   if (m_advancedMigration)
   {
      MigrationFormation * sourceFormation = getBottomMigrationFormation (end);
      if (!sourceFormation) return false;

      MigrationFormation * topActiveFormation = getTopActiveFormation (end);
      if (!topActiveFormation) return false;

      if (!computeSMFlowPaths (topActiveFormation, sourceFormation, start, end)) return false;

      if (!computeTargetFormationNodes (topActiveFormation, sourceFormation)) return false;
   }
   else
   {
      if (m_paleoSeeps or NumericFunctions::isEqual(end->getTime(), 0.0, DefaultNumericalTolerance))
      {
         MigrationFormation * topActiveFormation = getTopActiveFormation (end);
         if (!topActiveFormation) return false;

         topActiveFormation->setEndOfPath ();
      }
   }

   return true;
}

bool Migrator::saveSMFlowPaths (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   MigrationFormation * sourceFormation = getBottomMigrationFormation (end);
   if (!sourceFormation) return false;

   MigrationFormation * topActiveFormation = getTopActiveFormation (end);
   if (!topActiveFormation) return false;

   sourceFormation->saveComputedSMFlowPaths (topActiveFormation, end);

   return true;
}

bool Migrator::computeSMFlowPaths (migration::MigrationFormation * targetFormation, migration::MigrationFormation * sourceFormation,
                                   const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   if (sourceFormation == 0) return false;

   if (!sourceFormation->computeAnalogFlowDirections (targetFormation, start, end)) return false;

   if (!sourceFormation->computeAdjacentNodes (targetFormation, start, end)) return false;

   return true;
}

bool Migrator::computeTargetFormationNodes (migration::MigrationFormation * targetFormation, migration::MigrationFormation * sourceFormation)
{
   if (!sourceFormation->computeTargetFormationNodes (targetFormation)) return false;

   return true;
}

bool Migrator::chargeReservoirs (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;
   Interface::ReservoirList::iterator reservoirAboveIter;

   MigrationReservoir * reservoir = 0;
   MigrationReservoir * reservoirBelow = 0;
   MigrationReservoir * reservoirAbove = 0;
   for (reservoir = 0, reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      reservoir = (MigrationReservoir *)* reservoirIter;

      reservoirAboveIter = reservoirIter;
      if (++reservoirAboveIter != reservoirs->end ())
         reservoirAbove = (MigrationReservoir *)* reservoirAboveIter;
      else
         reservoirAbove = 0;

      assert (reservoir);

      if (reservoir->isActive (start))
      {
         if (!chargeReservoir (reservoir, reservoirAbove, reservoirBelow, start, end))
            return false;
         reservoirBelow = reservoir;
      }      ReportProgress ("Reservoir: ", reservoir->getName (), "", end->getTime ());
   }

   return true;
}

bool Migrator::chargeReservoir (migration::MigrationReservoir * reservoir, migration::MigrationReservoir * reservoirAbove, migration::MigrationReservoir * reservoirBelow,
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

   reservoir->resetProxiesBeforeRefine ();
   reservoir->refineGeometryZeroThicknessAreas ();
   reservoir->refineGeometrySetFaulStatus ();

   // Wasting the columns that have no element with the reservoir flag
   // Only in the advanced mode.
   migration::MigrationFormation * reservoirFormation = MigrationFormation::CastToFormation (reservoir->getFormation ());
   if (m_advancedMigration)
      reservoir->wasteNonReservoirColumns (end);

   // save only major snapshots results
   const bool saveSnapshot = end->getType () == Interface::MAJOR;
   reservoir->saveComputedInputProperties (saveSnapshot);

   reservoir->computePathways ();
   reservoir->computeTargetColumns ();

   reservoir->computeTraps ();

   Barrier * barrier = 0;
   /// Only allow blocking functionality in the basic mode of the simulator
   if (reservoir->isBlockingEnabled () and !m_advancedMigration)
   {
      barrier = new Barrier (reservoir);
   }

   double totalRetainedBeforeCracking = reservoir->getTotalChargesToBeMigrated ();

#ifdef USEOTGC
   reservoir->crackChargesToBeMigrated (*m_otgc);

   // trap capacities will have changed
   reservoir->recomputeTrapDepthToVolumeFunctions ();
#endif

   double totalRetainedAfterCracking = reservoir->getTotalChargesToBeMigrated ();

   reservoir->migrateChargesToBeMigrated (0, reservoir); // Lateral Migration
   reservoir->clearPreviousProperties ();

   collectAndMigrateExpelledCharges (reservoir, reservoirAbove, reservoirBelow, start, end, barrier);

   double totalExpelled = reservoir->getTotalToBeStoredCharges () - totalRetainedAfterCracking;

   double expelledBlocked = reservoir->getTotalBlocked ();

   if (reservoirBelow) // Collect the leaked HCs from the reservoir below
   {
      if (!m_advancedMigration)
      {
         reservoir->collectLeakedCharges (reservoirBelow, barrier);
      }
      else
      {
         migration::MigrationFormation * leakingReservoir = MigrationFormation::CastToFormation (reservoirBelow->getFormation ());
         leakingReservoir->migrateLeakedChargesToReservoir (reservoir);
      }
      reservoir->migrateChargesToBeMigrated (0, reservoirBelow); // Lateral Migration
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

   if (reservoir->isDiffusionEnabled ())
   {
      /// For each column in the trap set the diffusion starting time
      reservoir->broadcastTrapDiffusionStartTimes ();
      /// For each column in the trap set the new penetration distances
      reservoir->broadcastTrapPenetrationDistances ();
   }

   reservoir->broadcastTrapFillDepthProperties ();
   reservoir->broadcastTrapChargeProperties ();

   double totalBroadcastFromTraps = reservoir->getTotalChargesToBeMigrated ();

   if (GetRank () == 0)
   {
      if (totalStoredInTraps > 0 and Abs (totalStoredInTraps - totalBroadcastFromTraps) / totalStoredInTraps > 0.01)
      {
         cerr << "Basin_Error: Difference between charge as stored in traps and charge as stored in columns is too large" << endl;
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
      std::string str = std::string ("Reservoir: ") + reservoir->getName ();

      m_massBalance->printMassBalance (start, end, str);
   }

   if (GetRank () == 0) m_massBalance->clear ();

   reservoir->saveTrapProperties (saveSnapshot);
   reservoir->computeFluxes ();
   reservoir->saveComputedOutputProperties (saveSnapshot);

   return true;
}

// Depending on the configuration of the basin it calculates leakage from the top reservoir
// and expulsion from all active source rocks above the top reservoir
bool Migrator::calculateSeepage (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   if (!m_paleoSeeps and end->getTime () > 0.0)
      return true;

   MigrationFormation * seepsFormation = getTopActiveFormation (end);
   if (!seepsFormation)
      return false;

   MigrationFormation * topSourceRockFormation = getTopSourceRockFormation (end);
   MigrationFormation * topReservoirFormation = getTopActiveReservoirFormation (start, end);

   // If no source rock and no reservoir there can't be a source for new seepage
   if (!topSourceRockFormation and !topReservoirFormation)
      return true;

   // Index of the position of the top reservoir. If no reservoir exists, then a value well
   // below zero is needed to exclude downward migration and get the right expulsion amounts.
   // Top source rock is treated in a similar way
   int topReservoirIndex, topSourceRockIndex;
   topReservoirIndex = topSourceRockIndex = -10;

   // First calculate leakage from the top active reservoir
   if (topReservoirFormation)
   {
      MigrationFormation * formationOnTop = topReservoirFormation->getTopFormation ();

      // Only calculate leakage seeps if the top-reservoir top surface is not the top of the basin.
      // If it is, the column composition will be registered for seeps as it is currently stored.
      if (formationOnTop and formationOnTop->isActive (end))
      {
         if (!topReservoirFormation->calculateLeakageSeeps (end, m_advancedMigration))
            return false;
      }

      // Determine the position of the top reservoir.
      topReservoirIndex = topReservoirFormation->getDepositionSequence ();
   }

   if (topSourceRockFormation)
      topSourceRockIndex = topSourceRockFormation->getDepositionSequence ();

   int indexDifference = topReservoirIndex - topSourceRockIndex;

   // If the top SR is higher than the top reservoir, calculate expulsion as well
   if (indexDifference < 0)
   {
      Interface::FormationList * formations = getAllFormations ();
      Interface::FormationList::iterator formationIter;

      // There may be more than one source rock above the top reservoir
      for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
      {
         MigrationFormation * formation = MigrationFormation::CastToFormation (*formationIter);
         int formationIndex = formation->getDepositionSequence ();

         // Continue if above the top source rock (no source for seepage here)
         if (formationIndex > topSourceRockIndex)
            continue;
         // Break if at or below the top reservoir (expulsion won't reach the top)
         if (formationIndex <= topReservoirIndex)
            break;

         if (formation->isSourceRock () and formation->isActive (end))
         {
            // Account for the possibility of downward migration
            indexDifference = topReservoirIndex - formation->getDepositionSequence ();
            double fractionOfExpulsion = indexDifference < -2 ? 1.0 : 0.5;
            if (!formation->calculateExpulsionSeeps (end, fractionOfExpulsion, m_advancedMigration))
               return false;
         }
      }
   }

   saveSeepageAmounts (seepsFormation, end);

   return true;
}

void Migrator::saveSeepageAmounts (migration::MigrationFormation * seepsFormation, const Interface::Snapshot * end)
{
   // save only major snapshots results
   const bool saveSnapshot = end->getType () == Interface::MAJOR;
   if (!saveSnapshot)
      return;

   if (!m_ReservoirIoTbl) m_ReservoirIoTbl = m_projectHandle->getTable ("ReservoirIoTbl");
   assert (m_ReservoirIoTbl);
   // Create record without adding it to file
   database::Record * seepsReservoirRecord = m_ReservoirIoTbl->createRecord (false);

   // set the name of the new record object
   database::setReservoirName (seepsReservoirRecord, seepsFormation->getName ());

   // Not a real reservoir, but the I/O functionality of the reservoir class comes in handy
   migration::MigrationReservoir seepsReservoir (getProjectHandle (), this, seepsReservoirRecord);
   seepsReservoir.setEnd (end);

   // Setting formation but in PropertyValue.cpp getting the formation fails
   seepsReservoir.setFormation (dynamic_cast<DataAccess::Interface::Formation *> (seepsFormation));

   // Transfer seepage amounts from nodes to columns
   seepsReservoir.putSeepsInColumns (seepsFormation);

   seepsReservoir.saveSeepageProperties (end);

   return;
}

// collect expelled charges into reservoirs from the formations that are
// above reservoirBelow and not above or just above reservoir
bool Migrator::collectAndMigrateExpelledCharges (MigrationReservoir * reservoir, MigrationReservoir * reservoirAbove, MigrationReservoir * reservoirBelow,
                                                 const Interface::Snapshot * start, const Interface::Snapshot * end, Barrier * barrier)
{
   Interface::FormationList * formations = getAllFormations ();

   bool withinRange = false; // whether we reached the formation just above the reservoir's formation

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      const MigrationFormation* formation = dynamic_cast<const MigrationFormation*>(*formationIter);

      const MigrationFormation * formationBelow = formation->getBottomFormation ();

      const MigrationFormation * formationBelowBelow = 0;

      if (formationBelow)
         formationBelowBelow = formationBelow->getBottomFormation ();
      if (withinRange or formationBelow == reservoir->getFormation () or
          formationBelowBelow == reservoir->getFormation ())
      {
         withinRange = true;
      }
      else
      {
         continue;
      }

      unsigned int directionsToCollect = EXPELLEDNONE; // 0

      if (formationBelow == reservoir->getFormation () or formationBelowBelow == reservoir->getFormation ())
      {
         // source rock at most 2 formations above reservoir

         // check if reservoirAbove is in the way of reservoir with respect to downward migration
         if (!reservoirAbove or
             !reservoirAbove->isActive (start) or
             ((reservoirAbove->getFormation () != formationBelow and
               reservoirAbove->getFormation () != formationBelowBelow) and
              reservoirAbove->getFormation () != formation))
         {
            directionsToCollect |= EXPELLEDDOWNWARD;
         }
      }
      else if (formation == reservoir->getFormation ())
      {
         // reservoir in source rock

         // check if reservoirBelow is also in the source rock
         if (!reservoirBelow or !reservoirBelow->isActive (start) or reservoirBelow->getFormation () != formation)
         {
            directionsToCollect |= EXPELLEDDOWNWARD;
         }
         // check if reservoirAbove is also in the source rock
         if (!reservoirAbove or !reservoirAbove->isActive (start) or reservoirAbove->getFormation () != formation)
         {
            directionsToCollect |= EXPELLEDUPWARD;
         }
      }
      else if (reservoirBelow and
               (formationBelow == reservoirBelow->getFormation () or
                formationBelowBelow == reservoirBelow->getFormation ()))
      {
         // source rock just above reservoir below
         directionsToCollect |= EXPELLEDUPWARD;

         if (!reservoirBelow->isActive (start))
         {
            directionsToCollect |= EXPELLEDDOWNWARD;
         }
      }
      else
      {
         // comfortably below reservoir and above reservoirBelow
         directionsToCollect |= EXPELLEDDOWNWARD;
         directionsToCollect |= EXPELLEDUPWARD;
      }


      if (formation->isSourceRock ())
      {
         if ((directionsToCollect & EXPELLEDUPWARD) and m_advancedMigration)
         {
            formation->migrateExpelledChargesToReservoir (directionsToCollect, reservoir);
         }
         else
         {
            reservoir->collectExpelledCharges (formation, directionsToCollect, barrier);
         }

         reservoir->migrateChargesToBeMigrated (formation, 0); // Lateral Migration
      }

      if (barrier and (directionsToCollect & EXPELLEDUPWARD))
         barrier->updateBlocking (formation, end);

      if (reservoirBelow and reservoirBelow->isActive (start) and
          formationBelow == reservoirBelow->getFormation ())
      {
         break;
      }
   }
   return false;
}

void Migrator::deleteExpelledChargeMaps (const Interface::Snapshot * snapshot)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      const Interface::Formation * formation = *formationIter;
      if (formation->isSourceRock ())
      {
         for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
         {
            if (!ComponentsUsed[componentId]) continue;

            std::string propertyName = CBMGenerics::ComponentManager::getInstance().getSpeciesName( componentId );
            propertyName += "ExpelledCumulative";

            const Interface::GridMap * gridMapStart = getPropertyGridMap (propertyName, snapshot, 0, formation, 0);

            if (gridMapStart) delete gridMapStart;
         }
      }
   }
}

bool Migrator::loadExpulsionMaps (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation *formation = MigrationFormation::CastToFormation (*formationIter);
      if (formation->isSourceRock () and formation->isActive (end))
      {
         formation->loadExpulsionMaps (start, end);
      }
   }

   return true;
}

bool Migrator::unloadExpulsionMaps (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      MigrationFormation *formation = MigrationFormation::CastToFormation (*formationIter);
      if (formation->isSourceRock () and formation->isActive (end))
      {
         formation->unloadExpulsionMaps ();
      }
   }
   return true;
}

Interface::ReservoirList * Migrator::getReservoirs (const Interface::Formation * formation) const
{
   if (formation)
   {
      Interface::ReservoirList * reservoirs = m_projectHandle->getReservoirs (formation);
      return reservoirs;
   }
   else
   {
      if (!m_reservoirs) sortReservoirs ();
      return m_reservoirs;
   }
}

void Migrator::sortReservoirs () const
{
   //if a list is present, delete it because a new one will be created (this occours in ARD)
   if (m_reservoirs) delete m_reservoirs;
   m_reservoirs = m_projectHandle->getReservoirs ();
   sort (m_reservoirs->begin (), m_reservoirs->end (), reservoirSorter);
}


void Migrator::addTrapRecord (migration::MigrationReservoir * reservoir, migration::TrapPropertiesRequest & tpRequest)
{
   if (!m_trapIoTbl) m_trapIoTbl = m_projectHandle->getTable ("TrapIoTbl");
   assert (m_trapIoTbl);
   Record * trapIoRecord = m_trapIoTbl->createRecord ();
   assert (trapIoRecord);

   database::setReservoirName (trapIoRecord, reservoir->getName ());
   database::setAge (trapIoRecord, reservoir->getEnd ()->getTime ());
   database::setTrapID (trapIoRecord, tpRequest.id);

   double posI = 0, posJ = 0;
   m_projectHandle->getActivityOutputGrid ()->getPosition ((unsigned int)tpRequest.i, (unsigned int)tpRequest.j, posI, posJ);

   database::setXCoord (trapIoRecord, posI);
   database::setYCoord (trapIoRecord, posJ);

   database::setTrapCapacity (trapIoRecord, tpRequest.capacity);
   database::setDepth (trapIoRecord, tpRequest.depth);
   database::setSpillDepth (trapIoRecord, tpRequest.spillDepth);

   double spillPointPosI = 0, spillPointPosJ = 0;
   m_projectHandle->getActivityOutputGrid ()->getPosition ((unsigned int)tpRequest.spillPointI, (unsigned int)tpRequest.spillPointJ, spillPointPosI, spillPointPosJ);

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
      std::string fieldName = "Mass";
      fieldName += CBMGenerics::ComponentManager::getInstance().getSpeciesInputName(i);
      trapIoRecord->setValue (fieldName, tpRequest.composition.getWeight ((ComponentId) i));
   }
}

void Migrator::getMinimumColumnHeights ()
{
   // If reservoir detection is ON, the input minimum column heights are the right numbers to look at
   if (m_reservoirDetection)
   {
      m_minOilColumnHeight = m_projectHandle->getReservoirOptions()->getMinOilColumnHeight();
      m_minGasColumnHeight = m_projectHandle->getReservoirOptions()->getMinGasColumnHeight();
   }
   // But if ARD is OFF, we will use the default values, from BasinModellerPropertySpec
   else
   {
      // Get table and indices of of fields
      database::Table * resOptionsTable = m_projectHandle->getTable ("ReservoirOptionsIoTbl");

      const int minHeightVapour = resOptionsTable->getIndex ("MinGasColumnHeight");
      const int minHeightLiquid = resOptionsTable->getIndex ("MinOilColumnHeight");

      // Get table definition, field definition and default values as strings
      const TableDefinition & resOptionsDefinition = (resOptionsTable->getTableDefinition ());
      const std::string & minGasColumnHeight = resOptionsDefinition.getFieldDefinition (minHeightVapour)->defaultValue ();
      const std::string & minOilColumnHeight = resOptionsDefinition.getFieldDefinition (minHeightLiquid)->defaultValue ();

      // Use boost (unlike std::atof, it throws exceptions) to convert to doubles
      try
      {
         m_minGasColumnHeight = boost::lexical_cast<double> (minGasColumnHeight);
         m_minOilColumnHeight = boost::lexical_cast<double> (minOilColumnHeight);
      }
      // If somthing goes wrong, just use the values in the project file
      catch (const boost::bad_lexical_cast & e)
      {
         LogHandler( LogHandler::WARNING_SEVERITY ) << "Something wrong with getting the default values for minimum column heights. Using the ones in the project file\n";

         m_minOilColumnHeight = m_projectHandle->getReservoirOptions()->getMinOilColumnHeight();
         m_minGasColumnHeight = m_projectHandle->getReservoirOptions()->getMinGasColumnHeight();
      }
   }
}

void Migrator::getBlocking ()
{
   m_isBlockingOn = m_projectHandle->getReservoirOptions()->isBlockingOn();

   if (m_isBlockingOn)
   {
      m_blockingPermeability = m_projectHandle->getReservoirOptions()->getBlockingPermeability ();
      m_blockingPorosity     = m_projectHandle->getReservoirOptions()->getBlockingPorosity ();
   }
   else
   {
      m_blockingPermeability = 0.0;
      m_blockingPorosity     = 0.0;
   }
}

/// This function adds a  reservoir record to the ReservoirIoTbl with the values specified in the ReservoirOptionsIoTbl
database::Record * Migrator::addDetectedReservoirRecord (Interface::Formation * formation, const Interface::Snapshot * start)
{
   if (!m_ReservoirIoTbl) m_ReservoirIoTbl = m_projectHandle->getTable ("ReservoirIoTbl");
   assert (m_ReservoirIoTbl);
   Record * reservoirIoRecord = m_ReservoirIoTbl->createRecord ();
   assert (reservoirIoRecord);

   std::string detectedReservoirName = formation->getName ();
   database::setReservoirName (reservoirIoRecord, detectedReservoirName);
   database::setDetectedReservoir (reservoirIoRecord, 1);
   database::setFormationName (reservoirIoRecord, formation->getName ());
   database::setTrapCapacity (reservoirIoRecord, m_projectHandle->getReservoirOptions()->getTrapCapacity());
   database::setActivityMode (reservoirIoRecord, "ActiveFrom");

   database::setActivityStart (reservoirIoRecord, start->getTime ());
   database::setNetToGross (reservoirIoRecord, Interface::DefaultUndefinedScalarValue);
   database::setNetToGrossGrid (reservoirIoRecord, "");
   database::setBioDegradInd (reservoirIoRecord, m_projectHandle->getReservoirOptions()->isBiodegradationOn());
   database::setOilToGasCrackingInd (reservoirIoRecord, m_projectHandle->getReservoirOptions()->isOilToGasCrackingOn());
   database::setDiffusionInd (reservoirIoRecord, m_projectHandle->getReservoirOptions()->isDiffusionOn());
   database::setMinOilColumnHeight (reservoirIoRecord, m_projectHandle->getReservoirOptions()->getMinOilColumnHeight());
   database::setMinGasColumnHeight (reservoirIoRecord, m_projectHandle->getReservoirOptions()->getMinGasColumnHeight());
   database::setBlockingInd (reservoirIoRecord, m_projectHandle->getReservoirOptions()->isBlockingOn());
   database::setBlockingPermeability (reservoirIoRecord, m_projectHandle->getReservoirOptions()->getBlockingPermeability());
   database::setBlockingPorosity (reservoirIoRecord, m_projectHandle->getReservoirOptions()->getBlockingPorosity());

   return reservoirIoRecord;
}

database::Record * Migrator::copyMigrationRecord (database::Record * oldRecord, const std::string & newMigrationProcess)
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
      std::string fieldName = "Mass";
      fieldName += CBMGenerics::ComponentManager::getInstance().getSpeciesInputName( component );

      double mass = oldRecord->getValue<double> (fieldName);
      newRecord->setValue (fieldName, mass);
   }

   return newRecord;
}

void Migrator::addMigrationRecord (const std::string & srcReservoirName, const std::string & srcFormationName,
                                   const std::string & dstReservoirName, migration::MigrationRequest & mr)
{
   assert (GetRank () == 0);

   if (!m_migrationIoTbl) m_migrationIoTbl = m_projectHandle->getTable ("MigrationIoTbl");
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
      std::string fieldName = "Mass";
      fieldName += CBMGenerics::ComponentManager::getInstance().getSpeciesInputName( component );

      double mass = (newlyCreated ? 0 : record->getValue<double> (fieldName));
      mass += mr.composition.getWeight ((ComponentId) component);

      record->setValue (fieldName, mass);
   }
}

database::Record * Migrator::createMigrationRecord (const std::string & srcReservoirName, const std::string & srcFormationName,
                                                    const std::string & dstReservoirName,
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

database::Record * Migrator::findMigrationRecord (const std::string & srcReservoirName, const std::string & srcFormationName,
                                                  const std::string & dstReservoirName,
                                                  MigrationRequest & mr)
{
   assert (GetRank () == 0);

   std::vector<database::Record *> & recordList = m_migrationRecordLists[mr.process];
   std::vector<database::Record *>::iterator iter;
   for (iter = recordList.begin (); iter != recordList.end (); ++iter)
   {
      database::Record * record = *iter;

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

//  this function is used as less operator for the strict weak ordering
bool MigrationIoTblSorter (database::Record * recordL, database::Record * recordR)
{
   static int calls = 0;

   ++calls;

   if (recordL == 0 and recordR == 0)
   {
      cerr << "recordL = 0 and recordR = 0 in iteration " << calls << endl;
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

   return false;
}

void Migrator::sortMigrationRecords (void)
{
   // cerr << "Start sorting MigrationIoTbl" << endl;
   m_migrationIoTbl->stable_sort (MigrationIoTblSorter);
   // cerr << "Finish sorting MigrationIoTbl" << endl;
}

bool MigrationIoTblIsEqual (database::Record * recordL, database::Record * recordR)
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

bool MigrationIoTblMerge (database::Record * recordL, database::Record * recordR)
{
   for (unsigned int component = 0; component < NumComponents; ++component)
   {
      std::string fieldName = "Mass";
      fieldName += CBMGenerics::ComponentManager::getInstance().getSpeciesInputName( component );

      double massL = recordL->getValue<double> (fieldName);
      double massR = recordR->getValue<double> (fieldName);

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
   std::string migrationProcess;

   // cerr << "MigrationIoTbl size = " << m_migrationIoTbl->size () << endl;
   int index = 0;
   for (iter = m_migrationIoTbl->begin (), index = 0; iter != m_migrationIoTbl->end (); ++iter)
   {
      if (*iter == 0)
      {
         cerr << "Basin_Error: MigrationIoTbl[" << index << "] = 0" << endl;
      }
      else if (getSourceAge (*iter) < 0 or getDestinationAge (*iter) < 0)
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
      database::Record * record = *iter;
      const std::string & migrationProcess = getMigrationProcess (record);
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
            nextIter = m_migrationIoTbl->removeRecord (iter);
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

            nextIter = m_migrationIoTbl->removeRecord (iter);
            continue;
         }
         else if (sourceTrapId == destinationTrapId)
         {
            nextIter = m_migrationIoTbl->removeRecord (iter);
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
            nextIter = m_migrationIoTbl->removeRecord (iter);
            continue;
         }
      }
      else if (migrationProcess == MigrationProcessNames[THROUGHLEAKAGE])
      {
      }
      else if (migrationProcess == MigrationProcessNames[BIODEGRADATION] or
               migrationProcess == MigrationProcessNames[DIFFUSION] or
               migrationProcess == MigrationProcessNames[OILTOGASCRACKINGLOST] or
               migrationProcess == MigrationProcessNames[OILTOGASCRACKINGGAINED])
      {
         if (sourceTrapId < 0)
         {
            nextIter = m_migrationIoTbl->removeRecord (iter);
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
   assert (GetRank () == 0);
   for (int i = 0; i < NUMBEROFPROCESSES; ++i)
   {
      std::vector<database::Record *> & recordList = m_migrationRecordLists[i];
      std::vector<database::Record *>::iterator iter;
      for (iter = recordList.begin (); iter != recordList.end (); ++iter)
      {
         database::Record * record = *iter;
         if (getSourceTrapID (record) == oldTrapNumber and m_projectHandle->findSnapshot (getSourceAge (record)) == snapshot)
         {
            setSourceTrapID (record, newTrapNumber);
         }

         if (getDestinationTrapID (record) == oldTrapNumber and m_projectHandle->findSnapshot (getDestinationAge (record)) == snapshot)
         {
            setDestinationTrapID (record, newTrapNumber);
         }
      }
   }
}

const Interface::GridMap * Migrator::getPropertyGridMap (const std::string & propertyName, const Interface::Snapshot * snapshot,
                                                         const Interface::Reservoir * reservoir,
                                                         const Interface::Formation * formation, const Interface::Surface * surface)
{
   int selectionFlags = 0;

   if (reservoir) selectionFlags |= Interface::RESERVOIR;
   if (formation and !surface) selectionFlags |= Interface::FORMATION;
   if (surface and !formation) selectionFlags |= Interface::SURFACE;
   if (formation and surface) selectionFlags |= Interface::FORMATIONSURFACE;

   Interface::PropertyValueList * propertyValues = m_projectHandle->getPropertyValues (selectionFlags,
                                                                                       m_projectHandle->findProperty (propertyName),
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
   return reservoir1->getFormation ()->getDepositionSequence () < reservoir2->getFormation ()->getDepositionSequence ();
}

bool Migrator::mergeOutputFiles ()
{
#ifndef _MSC_VER

   bool status = true;

   // clean mpaCache which can hold read-only opened files
   m_projectHandle->mapFileCacheDestructor();

   const std::string& directoryName = m_projectHandle->getOutputDir ();

   // Merge 3D output
   database::Table::iterator timeTableIter;
   database::Table* snapshotTable =  m_projectHandle->getTable ( "SnapshotIoTbl" );

   assert ( snapshotTable != 0 );
   const std::string timepart = "_";
   const std::string flowpart = flowPathsFileNamePrefix;

   PetscBool minorSnapshots;
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-minor", &minorSnapshots);

   for ( timeTableIter = snapshotTable->begin (); timeTableIter != snapshotTable->end (); ++timeTableIter ) {
      bool isMinor = database::getIsMinorSnapshot( *timeTableIter ) == 1;

      // check if we need to merge minor snapshots
      if( minorSnapshots or ( not minorSnapshots and not isMinor )) {

         // constract the file name to megre
         std::string snapshotFileName = database::getSnapshotFileName ( *timeTableIter );
         if ( !snapshotFileName.empty() ) {
            // find a time part in the snapshot file name
            std::size_t found = snapshotFileName.find(timepart);

            if (found != std::string::npos ) {
               // create the file name
               std::string flowFile = flowpart + snapshotFileName.substr(found);

               // check if the temporary file exists
               std::stringstream tempname;
               tempname << H5_Parallel_PropertyList::getTempDirName() <<  "/" << directoryName << "/" << flowFile << "_" << GetRank();

               ibs::FilePath tempPathName(tempname.str().c_str());

               if( tempPathName.exists()) {

                  ibs::FilePath filePathName( m_projectHandle->getProjectPath () );
                  filePathName << directoryName << flowFile;

                  PetscPrintf ( PETSC_COMM_WORLD,  "Merging of %s\n",  flowFile.c_str() );

                  if( !mergeFiles ( allocateFileHandler( PETSC_COMM_WORLD, filePathName.path(), H5_Parallel_PropertyList::getTempDirName(), CREATE ))) {
                     status = false;
                     PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error: Could not merge the file %s.\n", filePathName.cpath() );
                  }
               }
            }
         }
      }
   }
   // Merge 2D output
   ibs::FilePath localPath  ( m_projectHandle->getProjectPath () );
   localPath <<  directoryName;
   status = H5_Parallel_PropertyList ::mergeOutputFiles ( MigrationActivityName, localPath.path() );

   return status;
#else
   return true;
#endif
}
