//
// Copyright (C) 2010-2016 Shell International Exploration & Production.
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

#include "GeoPhysicsProjectHandle.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Reservoir.h"
#include "Interface/PropertyValue.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/SimulationDetails.h"
#include "Interface/RunParameters.h"
#include "MassBalance.h"
#include "utils.h"

#include "h5_parallel_file_types.h"
#include "FilePath.h"

#ifndef _MSC_VER
#include "h5merge.h"
#endif

using namespace DataAccess;

static bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2);

extern string NumProcessorsArg;

Migrator::Migrator (const string & name)
{
   ObjectFactory * objectFactory = new ObjectFactory (this);

   m_projectHandle.reset (dynamic_cast<GeoPhysics::ProjectHandle *> (Interface::OpenCauldronProject (name, "rw", objectFactory)));

   if (!m_projectHandle.get ())
   {
      std::cerr << "Can't open input project file: " << name << std::endl;
      exit (-1);
   }

   m_massBalance = 0;
   m_propertyManager.reset (new MigrationPropertyManager (m_projectHandle.get ()));

   m_reservoirs = 0;
   m_formations = 0;


   InitializeRequestTypes ();
   if (GetRank () == 0)
   {
      m_migrationRecordLists = new vector<database::Record *>[NUMBEROFPROCESSES];
   }

   m_migrationIoTbl = 0;
   m_trapIoTbl = 0;
   m_ReservoirIoTbl = 0;
   m_detectedReservoirIoTbl = 0;
   m_detectedReservoirIoRecord = 0;

   getMinimumColumnHeights ();

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

bool Migrator::saveTo (const string & outputFileName)
{
   return m_projectHandle->saveToFile (outputFileName);
}

GeoPhysics::ProjectHandle * Migrator::getProjectHandle (void)
{
   return m_projectHandle.get ();
}

bool Migrator::compute (void)
{
   string activityName = MigrationActivityName;
   activityName += NumProcessorsArg;
   H5_Parallel_PropertyList::setOneFilePerProcessOption ();

   // let's first clean out the mess from a previous run
   if (GetRank () == 0)
   {
      m_projectHandle->deletePropertyValues (DataAccess::Interface::RESERVOIR);

      // Commented out only for May 2016 release. When 3d grid is used, FlowDirectionIJK will be output and then it must be deleted
      // m_projectHandle->deletePropertyValues (DataAccess::Interface::FORMATION, m_projectHandle->findProperty("FlowDirectionIJK"), 0, 0, 0, 0, DataAccess::Interface::VOLUME);

      if (!m_trapIoTbl) m_trapIoTbl = m_projectHandle->getTable ("TrapIoTbl");
      m_trapIoTbl->clear ();

      if (!m_migrationIoTbl) m_migrationIoTbl = m_projectHandle->getTable ("MigrationIoTbl");
      m_migrationIoTbl->clear ();
   }

   bool started = m_projectHandle->startActivity (activityName, m_projectHandle->getHighResolutionOutputGrid ());
   if (!started) return false;

   ios::fmtflags f (std::cout.flags ());
   std::cout << std::setfill (' ');
   std::cout.flags (f);

   if (!started) return false;

   openMassBalanceFile ();

   ComputeRanks (m_projectHandle->getActivityOutputGrid ());

   m_verticalMigration = m_projectHandle->getRunParameters ()->getVerticalSecondaryMigration ();
   m_hdynamicAndCapillary = m_projectHandle->getRunParameters ()->getHydrodynamicCapillaryPressure ();
   if (m_verticalMigration)
      m_hdynamicAndCapillary = 0;
   m_reservoirDetection = m_projectHandle->getRunParameters ()->getReservoirDetection ();
   m_paleoSeeps = m_projectHandle->getRunParameters ()->getPaleoSeeps ();
   m_legacyMigration = m_projectHandle->getRunParameters ()->getLegacy ();

   if (!setUpBasinGeometry ()) return false;

   if (m_legacyMigration)
   {
      m_verticalMigration = true;
      m_hdynamicAndCapillary = false;
      m_reservoirDetection = false;
      m_paleoSeeps = false;
   }

   bool overPressureRun = !isHydrostaticCalculation ();

   if (!m_legacyMigration)
   {
      createFormationNodes();
      if (!computeFormationPropertyMaps(m_projectHandle->getSnapshots()->front(), overPressureRun)) return false;
   }

   //By default the m_topDepthOffset= m_bottomDepthOffset = 0 
   if (!computeDepthOffsets()) return false;
   if (!computeNetToGross()) return false;

   // delete the maps created for computeDepthOffsets and computeNetToGross
   m_propertyManager->removeProperties (m_projectHandle->getSnapshots ()->front ());
   // delete the formation property grid maps
   deleteFormationPropertyMaps ();

   PetscBool minorSnapshots;

   PetscOptionsHasName (PETSC_NULL, "-minor", &minorSnapshots);

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
   string simulatorMode;

   if (!m_verticalMigration and !m_hdynamicAndCapillary)
   {
      simulatorMode += simulationModeStr[3];
      simulatorMode += " ";
   }

   if (m_verticalMigration)
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
      PetscPrintf (PETSC_COMM_WORLD, "MeSsAgE ERROR Unable to merge output files\n");
      status = false;
   }

   migration::deleteRanks ();

   return status;
}

void Migrator::openMassBalanceFile (void)
{
   if (GetRank () == 0)
   {
      string fileName = utils::getProjectBaseName (m_projectHandle->getProjectName ());
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

   if (!m_legacyMigration)
   {
      // From GeoPhysics::ProjectHandle
      if (!m_projectHandle->initialise (true) or
         !m_projectHandle->setFormationLithologies (true, true) or
         !m_projectHandle->initialiseLayerThicknessHistory (!HydrostaticCalculation) or // Backstripping
         !m_projectHandle->applyFctCorrections ())
         return false;
      else
         return true;
   }
   else
   {
      if (!m_projectHandle->initialise (true) or
         !m_projectHandle->setFormationLithologies (true, true))
         return false;
      else
         return true;
   }
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
      Formation *formation = Formation::CastToFormation (*formationIter);

      assert (formation);
      if (!formation->isActive (snapshot)) continue;

      bool lowResEqualsHighRes = ((*(m_projectHandle->getLowResolutionOutputGrid ())) == (*(m_projectHandle->getHighResolutionOutputGrid ())));

      formation->computePropertyMaps (topDepthGridMap, snapshot, lowResEqualsHighRes, isOverPressureRun, m_projectHandle->getRunParameters ()->getNonGeometricLoop (),
         m_projectHandle->getRunParameters ()->getChemicalCompaction ()); // allowed to fail

      formation->computeHCDensityMaps (snapshot);

      if (m_hdynamicAndCapillary or m_reservoirDetection)
      {
         if (!formation->computeCapillaryPressureMaps (topDepthGridMap, snapshot))
            return false;
      }
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

         if (seaBottomDepth == Interface::DefaultUndefinedMapValue or seaBottomDepth == Interface::DefaultUndefinedScalarValue)
         {
            topDepthGridMap->setValue (i, j, Interface::DefaultUndefinedMapValue);
         }
         else
         {
            topDepthGridMap->setValue (i, j, m_projectHandle->getSeaBottomDepth (i, j, snapshot->getTime ()));
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
      Formation * formation = Formation::CastToFormation (*formationIter);

      assert (formation);

      formation->createFormationNodes ();
   }
   return true;
}

bool Migrator::performSnapshotMigration (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool overPressureRun)
{
   bool sourceRockActive = false;
   migration::Formation * bottomSourceRock = getBottomSourceRockFormation ();

   if (bottomSourceRock != nullptr)
	   sourceRockActive = bottomSourceRock->isActive(end);

   if ((activeReservoirs (end) or m_reservoirDetection or m_paleoSeeps or end->getTime () == 0.0) and sourceRockActive)
   {
      if (GetRank () == 0)
         std::cout << "Processing snapshot " << end->getTime () << std::endl;

      if (m_legacyMigration)
      {
         if (!chargeReservoirs (start, end))
            return false;
      }
      else
      {

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
            !calculateSeepage (end) or
            !unloadExpulsionMaps (end) or
            !saveSMFlowPaths (start, end))
         {
            return false;
         }

         deleteFormationPropertyMaps ();
      }
   }

   m_projectHandle->continueActivity ();

#ifdef DEBUGMEMORY   
   stringstream ss;
   ss << " rank " << GetRank( ) << " " << getMemoryUsed( );
   string mystring1( " Before deleting properties: " );
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
bool Migrator::computeDepthOffsets ()
{
   Interface::ReservoirList * reservoirs = getReservoirs ();

   Interface::ReservoirList::iterator reservoirIter;

   for (reservoirIter = reservoirs->begin(); reservoirIter != reservoirs->end(); ++reservoirIter)
   {
	   Reservoir * reservoir = (Reservoir *)* reservoirIter;

	   assert(reservoir);

	   if (!reservoir->computeDepthOffsets(m_projectHandle->findSnapshot(0.))) return false;
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
      Reservoir * reservoir = (Reservoir *)* reservoirIter;

      assert (reservoir);

	  if (!reservoir->computeNetToGross ()) return false;
   }
   return true;
}

migration::Formation * Migrator::getFormation (int index)
{
   Interface::FormationList * formations = getAllFormations ();
   return Formation::CastToFormation ((*formations)[index]);
}

int Migrator::getIndex (migration::Formation * formation)
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

migration::Reservoir * Migrator::getReservoir (int index)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();
   return (Reservoir *)(*reservoirs)[index];
}

int Migrator::getIndex (migration::Reservoir * reservoir)
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
      const Reservoir * reservoir = (const Reservoir *)* reservoirIter;

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
      Formation * formation = Formation::CastToFormation (*formationIter);

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
      Formation * formation = Formation::CastToFormation (*formationIter);

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
      Formation * formation = Formation::CastToFormation (*formationIter);
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
      Formation * formation = Formation::CastToFormation (*formationIter);
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
      Formation * formation = Formation::CastToFormation (*formationIter);
      if (!formation->isActive (end)) continue;

      formation->computeFaults (end);
      formation->computeNodeProperties ();
   }

   return true;
}

migration::Formation * Migrator::getBottomSourceRockFormation ()
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   Formation * bottomSourceRockFormation = 0;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);

      if (formation->isSourceRock ()) bottomSourceRockFormation = formation;
   }

   return bottomSourceRockFormation;
}

migration::Formation * Migrator::getTopSourceRockFormation (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   Formation * topSourceRockFormation = 0;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);

      if (formation->isSourceRock () and formation->isActive (end))
      {
         topSourceRockFormation = formation;
         break;
      }
   }

   return topSourceRockFormation;
}

migration::Formation * Migrator::getTopActiveFormation (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);

      if (formation->isActive (end)) return formation;
   }

   return 0;
}

// Returns the top reservoir that has been deposited, is sealed and is active at snapshot "end"
migration::Formation * Migrator::getTopActiveReservoirFormation (const Interface::Snapshot * end)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();
   Interface::ReservoirList::iterator reservoirIter;

   Reservoir * topActiveReservoir = 0;
   Formation * topActiveReservoirFormation = 0;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *)(*reservoirIter);
      Formation * tempFormation = Formation::CastToFormation (reservoir->getFormation ());

      bool isDeposited = tempFormation->getTopSurface ()->getSnapshot ()->getTime () > end->getTime ();

      if (reservoir->isActive (end) and isDeposited)
      {
         topActiveReservoir = reservoir;
      }
   }

   if (topActiveReservoir)
   {
      topActiveReservoirFormation = Formation::CastToFormation (topActiveReservoir->getFormation ());
   }

   return topActiveReservoirFormation;
}

migration::Formation * Migrator::getBottomActiveReservoirFormation (const Interface::Snapshot * end)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();
   Interface::ReservoirList::iterator reservoirIter;

   Reservoir * bottomActiveReservoir = 0;
   Formation * bottomActiveReservoirFormation = 0;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *)(*reservoirIter);
      Formation * tempFormation = Formation::CastToFormation (reservoir->getFormation ());

      bool isDeposited = tempFormation->getTopSurface ()->getSnapshot ()->getTime () >= end->getTime ();
      if (reservoir->isActive (end) and isDeposited)
      {
         bottomActiveReservoir = reservoir;
         break;
      }
   }

   if (bottomActiveReservoir)
   {
      bottomActiveReservoirFormation = Formation::CastToFormation (bottomActiveReservoir->getFormation ());
   }
   return bottomActiveReservoirFormation;
}

migration::Formation * Migrator::getBottomMigrationFormation(const Interface::Snapshot * end)
{
	Formation * bottomFormation;
	Formation *bottomSourceRockFormation = getBottomSourceRockFormation();
	if (bottomSourceRockFormation == 0) return 0;
	Formation * belowBelowSourceRockFormation = 0;
	Formation * belowSourceRockFormation = bottomSourceRockFormation->getBottomFormation();
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
	Formation * bottomFormation = getBottomMigrationFormation(end);
	Formation * topSealFormation = getTopActiveFormation(end);
	if (topSealFormation == 0 or bottomFormation == 0) return false;
	Formation *sealFormation;
	Formation *reservoirFormation;
	bool topSealFormationReached = false;
	for (reservoirFormation = bottomFormation, sealFormation = (Formation *)reservoirFormation->getTopFormation();
		sealFormation != 0 and !topSealFormationReached;
		reservoirFormation = sealFormation, sealFormation = (Formation *)sealFormation->getTopFormation())
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
   Formation * bottomFormation = getBottomMigrationFormation(end);
   Formation * topSealFormation = getTopActiveFormation(end);
   if (topSealFormation == 0 or bottomFormation == 0) return false;

   Formation *reservoirFormation;
   Formation *sealFormation;
   Interface::ReservoirList *  reservoirsBeforeDetection = getReservoirs ();
   int numDetected = 0;


   bool topSealFormationReached = false;
   for (reservoirFormation = bottomFormation, sealFormation = (Formation *)reservoirFormation->getTopFormation ();
      sealFormation != 0 and !topSealFormationReached;
      reservoirFormation = sealFormation, sealFormation = (Formation *)sealFormation->getTopFormation ())
   {
      //check if top seal formation is reached
      topSealFormationReached = (sealFormation == topSealFormation);

      assert (reservoirFormation);

      //Assigns nodes of user-selected reservoirs as reservoir nodes, no need to identify the Reservoir here. Do this only for user-defined reservoirs
      std::unique_ptr<Interface::ReservoirList> alreadyReservoirFormation (getReservoirs (reservoirFormation));
      if (!alreadyReservoirFormation->empty () and !reservoirFormation->getDetectedReservoir ())
      {
         reservoirFormation->identifyAsReservoir ();
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
               // cerr << "Formation " << reservoirFormation->getName() << " detected" << endl;
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
   if (!m_verticalMigration)
   {
      Formation * sourceFormation = getBottomMigrationFormation (end);
      if (!sourceFormation) return false;

      Formation * topActiveFormation = getTopActiveFormation (end);
      if (!topActiveFormation) return false;

      if (!computeSMFlowPaths (topActiveFormation, sourceFormation, start, end)) return false;

      if (!computeTargetFormationNodes (topActiveFormation, sourceFormation)) return false;
   }
   else
   {
      if (m_paleoSeeps or end->getTime () == 0.0)
      {
         Formation * topActiveFormation = getTopActiveFormation (end);
         if (!topActiveFormation) return false;

         topActiveFormation->setEndOfPath ();
      }
   }

   return true;
}

bool Migrator::saveSMFlowPaths (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Formation * sourceFormation = getBottomMigrationFormation (end);
   if (!sourceFormation) return false;

   Formation * topActiveFormation = getTopActiveFormation (end);
   if (!topActiveFormation) return false;

   sourceFormation->saveComputedSMFlowPaths (topActiveFormation, end);

   return true;
}

bool Migrator::computeSMFlowPaths (migration::Formation * targetFormation, migration::Formation * sourceFormation,
   const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   if (sourceFormation == 0) return false;

   if (!sourceFormation->computeAnalogFlowDirections (targetFormation, start, end)) return false;

   if (!sourceFormation->computeAdjacentNodes (targetFormation, start, end)) return false;

   return true;
}

bool Migrator::computeTargetFormationNodes (migration::Formation * targetFormation, migration::Formation * sourceFormation)
{
   if (!sourceFormation->computeTargetFormationNodes (targetFormation)) return false;

   return true;
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
      reservoir = (Reservoir *)* reservoirIter;

      reservoirAboveIter = reservoirIter;
      if (++reservoirAboveIter != reservoirs->end ())
         reservoirAbove = (Reservoir *)* reservoirAboveIter;
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

bool Migrator::chargeReservoir (migration::Reservoir * reservoir, migration::Reservoir * reservoirAbove, migration::Reservoir * reservoirBelow,
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
   // Only for detected reservoirs.
   migration::Formation * reservoirFormation = Formation::CastToFormation (reservoir->getFormation ());
   if (reservoirFormation->getDetectedReservoir ())
      reservoir->wasteNonReservoirColumns (end);

   // save only major snapshots results
   const bool saveSnapshot = end->getType () == Interface::MAJOR;
   reservoir->saveComputedInputProperties (saveSnapshot);

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
      // Non-Vertical Migration: collectLeakedCharges () assumes vertical. We want something like 
      // migrateExpelledChargesToReservoir () as in the case of expulsion. If vertical then collectLeakedCHarges () is OK.
      if (m_verticalMigration)
      {
         reservoir->collectLeakedCharges (reservoirBelow, barrier);
      }
      else
      {
         migration::Formation * leakingReservoir = Formation::CastToFormation (reservoirBelow->getFormation ());
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

   if (!m_legacyMigration and reservoir->isDiffusionOn ())
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
      string str = string ("Reservoir: ") + reservoir->getName ();

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
bool Migrator::calculateSeepage (const Interface::Snapshot * end)
{
   if (!m_paleoSeeps and end->getTime () > 0.0)
      return true;

   Formation * seepsFormation = getTopActiveFormation (end);
   if (!seepsFormation)
      return false;

   Formation * topSourceRockFormation = getTopSourceRockFormation (end);
   Formation * topReservoirFormation = getTopActiveReservoirFormation (end);

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
      Formation * formationOnTop = topReservoirFormation->getTopFormation ();

      // Only calculate leakage seeps if the top-reservoir top surface is not the top of the basin.
      // If it is, the column composition will be registered for seeps as it is currently stored.
      if (formationOnTop and formationOnTop->isActive (end))
      {
         if (!topReservoirFormation->calculateLeakageSeeps (end, m_verticalMigration))
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
         Formation * formation = Formation::CastToFormation (*formationIter);
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
            if (!formation->calculateExpulsionSeeps (end, fractionOfExpulsion, m_verticalMigration))
               return false;
         }
      }
   }

   saveSeepageAmounts (seepsFormation, end);

   return true;
}

void Migrator::saveSeepageAmounts (migration::Formation * seepsFormation, const Interface::Snapshot * end)
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

   DataAccess::Interface::ProjectHandle * dataAccessProjectHandle = dynamic_cast<DataAccess::Interface::ProjectHandle *> (getProjectHandle ());
   // Not a real reservoir, but the I/O functionality of the reservoir class comes in handy
   migration::Reservoir seepsReservoir (dataAccessProjectHandle, this, seepsReservoirRecord);
   seepsReservoir.setEnd (end);

   // Setting formation but in PropertyValue.C getting the formation fails
   seepsReservoir.setFormation (dynamic_cast<DataAccess::Interface::Formation *> (seepsFormation));

   // Transfer seepage amounts from nodes to columns
   seepsReservoir.putSeepsInColumns (seepsFormation);

   seepsReservoir.saveSeepageProperties (end);

   return;
}

// collect expelled charges into reservoirs from the formations that are
// above reservoirBelow and not above or just above reservoir
bool Migrator::collectAndMigrateExpelledCharges (Reservoir * reservoir, Reservoir * reservoirAbove, Reservoir * reservoirBelow,
   const Interface::Snapshot * start, const Interface::Snapshot * end, Barrier * barrier)
{
   Interface::FormationList * formations = getAllFormations ();

   bool withinRange = false; // whether we reached the formation just above the reservoir's formation

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      const Formation* formation = dynamic_cast<const Formation*>(*formationIter);

      const Formation * formationBelow = formation->getBottomFormation ();

      const Formation * formationBelowBelow = 0;

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
            !reservoirAbove->isActive (end) or
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
         if (!reservoirBelow or !reservoirBelow->isActive (end) or reservoirBelow->getFormation () != formation)
         {
            directionsToCollect |= EXPELLEDDOWNWARD;
         }
         // check if reservoirAbove is also in the source rock
         if (!reservoirAbove or !reservoirAbove->isActive (end) or reservoirAbove->getFormation () != formation)
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

         if (!reservoirBelow->isActive (end))
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
         if ((directionsToCollect & EXPELLEDUPWARD) and !m_verticalMigration)
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

      if (reservoirBelow and reservoirBelow->isActive (end) and
         formationBelow == reservoirBelow->getFormation () and
         !performHDynamicAndCapillary ()) //as in sec-mig 
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

            string propertyName = CBMGenerics::ComponentManager::getInstance().getSpeciesName( componentId );
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
      Formation *formation = Formation::CastToFormation (*formationIter);
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
      Formation *formation = Formation::CastToFormation (*formationIter);
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


void Migrator::addTrapRecord (migration::Reservoir * reservoir, migration::TrapPropertiesRequest & tpRequest)
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
      string fieldName = "Mass";
      fieldName += CBMGenerics::ComponentManager::getInstance().getSpeciesInputName(i);
      trapIoRecord->setValue (fieldName, tpRequest.composition.getWeight ((ComponentId) i));
   }
}

// this function sets the minimum gas and column heights and reads the DetectedReservoirIoTbl
void Migrator::getMinimumColumnHeights ()
{

   if (!m_detectedReservoirIoTbl) m_detectedReservoirIoTbl = m_projectHandle->getTable ("DetectedReservoirIoTbl");
   assert (m_detectedReservoirIoTbl);
   if (!m_detectedReservoirIoRecord)
   {
      Table::iterator  iterator = m_detectedReservoirIoTbl->begin ();
      m_detectedReservoirIoRecord = m_detectedReservoirIoTbl->getRecord (iterator);
      if (!m_detectedReservoirIoRecord) m_detectedReservoirIoRecord = m_detectedReservoirIoTbl->createRecord ();
   }
   assert (m_detectedReservoirIoRecord);

   m_minOilColumnHeight = database::getMinOilColumnHeight (m_detectedReservoirIoRecord);
   m_minGasColumnHeight = database::getMinGasColumnHeight (m_detectedReservoirIoRecord);

}

/// This function adds a  reservoir record to the ReservoirIoTbl with the values specified in the DetectedReservoirIoTbl
database::Record * Migrator::addDetectedReservoirRecord (Interface::Formation * formation, const Interface::Snapshot * start)
{
   if (!m_ReservoirIoTbl) m_ReservoirIoTbl = m_projectHandle->getTable ("ReservoirIoTbl");
   assert (m_ReservoirIoTbl);
   Record * reservoirIoRecord = m_ReservoirIoTbl->createRecord ();
   assert (reservoirIoRecord);

   string detectedReservoirName = formation->getName ();
   database::setReservoirName (reservoirIoRecord, detectedReservoirName);
   database::setDetectedReservoir (reservoirIoRecord, 1);
   database::setFormationName (reservoirIoRecord, formation->getName ());
   database::setTrapCapacity (reservoirIoRecord, database::getTrapCapacity (m_detectedReservoirIoRecord));
   database::setActivityMode (reservoirIoRecord, "ActiveFrom");

   database::setActivityStart (reservoirIoRecord, start->getTime ());
   database::setDepthOffset (reservoirIoRecord, database::getDepthOffset (m_detectedReservoirIoRecord));
   database::setDepthOffsetGrid (reservoirIoRecord, database::getDepthOffsetGrid (m_detectedReservoirIoRecord));
   database::setThickness (reservoirIoRecord, database::getThickness (m_detectedReservoirIoRecord));
   database::setThicknessGrid (reservoirIoRecord, database::getThicknessGrid (m_detectedReservoirIoRecord));
   database::setNetToGross (reservoirIoRecord, database::getNetToGross (m_detectedReservoirIoRecord));
   database::setNetToGrossGrid (reservoirIoRecord, database::getNetToGrossGrid (m_detectedReservoirIoRecord));
   database::setMicroTraps (reservoirIoRecord, database::getMicroTraps (m_detectedReservoirIoRecord));
   database::setLeakProperty (reservoirIoRecord, database::getLeakProperty (m_detectedReservoirIoRecord));
   database::setLeakRate (reservoirIoRecord, database::getLeakRate (m_detectedReservoirIoRecord));
   database::setLithotype1 (reservoirIoRecord, database::getLithotype1 (m_detectedReservoirIoRecord));
   database::setPercent1 (reservoirIoRecord, database::getPercent1 (m_detectedReservoirIoRecord));
   database::setPercent1Grid (reservoirIoRecord, database::getPercent1Grid (m_detectedReservoirIoRecord));
   database::setLithotype2 (reservoirIoRecord, database::getLithotype2 (m_detectedReservoirIoRecord));
   database::setPercent2 (reservoirIoRecord, database::getPercent2 (m_detectedReservoirIoRecord));
   database::setPercent2Grid (reservoirIoRecord, database::getPercent2Grid (m_detectedReservoirIoRecord));
   database::setLithotype3 (reservoirIoRecord, database::getLithotype3 (m_detectedReservoirIoRecord));
   database::setLayerFrequency (reservoirIoRecord, database::getLayerFrequency (m_detectedReservoirIoRecord));
   database::setLayerFrequencyGrid (reservoirIoRecord, database::getLayerFrequencyGrid (m_detectedReservoirIoRecord));
   database::setBioDegradInd (reservoirIoRecord, database::getBioDegradInd (m_detectedReservoirIoRecord));
   database::setOilToGasCrackingInd (reservoirIoRecord, database::getOilToGasCrackingInd (m_detectedReservoirIoRecord));
   database::setDiffusionInd (reservoirIoRecord, database::getDiffusionInd (m_detectedReservoirIoRecord));
   database::setMinOilColumnHeight (reservoirIoRecord, database::getMinOilColumnHeight (m_detectedReservoirIoRecord));
   database::setMinGasColumnHeight (reservoirIoRecord, database::getMinGasColumnHeight (m_detectedReservoirIoRecord));
   database::setBlockingInd (reservoirIoRecord, database::getBlockingInd (m_detectedReservoirIoRecord));
   database::setBlockingPermeability (reservoirIoRecord, database::getBlockingPermeability (m_detectedReservoirIoRecord));
   database::setBlockingPorosity (reservoirIoRecord, database::getBlockingPorosity (m_detectedReservoirIoRecord));
   database::setErrDepthOffset (reservoirIoRecord, database::getErrDepthOffset (m_detectedReservoirIoRecord));
   database::setErrThickness (reservoirIoRecord, database::getErrThickness (m_detectedReservoirIoRecord));
   database::setErrMicroTraps (reservoirIoRecord, database::getErrMicroTraps (m_detectedReservoirIoRecord));
   database::setErrLayerFrequency (reservoirIoRecord, database::getErrLayerFrequency (m_detectedReservoirIoRecord));

   return reservoirIoRecord;
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
      fieldName += CBMGenerics::ComponentManager::getInstance().getSpeciesInputName( component );

      double mass = oldRecord->getValue<double> (fieldName);
      newRecord->setValue (fieldName, mass);
   }

   return newRecord;
}

void Migrator::addMigrationRecord (const string & srcReservoirName, const string & srcFormationName,
   const string & dstReservoirName, migration::MigrationRequest & mr)
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
      string fieldName = "Mass";
      fieldName += CBMGenerics::ComponentManager::getInstance().getSpeciesInputName( component );

      double mass = (newlyCreated ? 0 : record->getValue<double> (fieldName));
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
      string fieldName = "Mass";
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
   string migrationProcess;

   // cerr << "MigrationIoTbl size = " << m_migrationIoTbl->size () << endl;
   int index = 0;
   for (iter = m_migrationIoTbl->begin (), index = 0; iter != m_migrationIoTbl->end (); ++iter)
   {
      if (*iter == 0)
      {
         cerr << "Error: MigrationIoTbl[" << index << "] = 0" << endl;
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
      vector<database::Record *> & recordList = m_migrationRecordLists[i];
      vector<database::Record *>::iterator iter;
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

const Interface::GridMap * Migrator::getPropertyGridMap (const string & propertyName, const Interface::Snapshot * snapshot,
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
   if (m_projectHandle->getModellingMode () == Interface::MODE1D) return true;
#ifndef _MSC_VER
   ibs::FilePath localPath  ( m_projectHandle->getProjectPath () );
   localPath <<  m_projectHandle->getOutputDir ();
   const bool status = H5_Parallel_PropertyList ::mergeOutputFiles ( MigrationActivityName, localPath.path() );

   return status;
#else
   return true;
#endif
}
