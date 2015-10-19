//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


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
#include "Interface/SimulationDetails.h"
#include "Interface/RunParameters.h"
#include "MassBalance.h"
#include "utils.h"

#include "h5_parallel_file_types.h"

#ifndef _MSC_VER
#include "h5merge.h"
#endif

using namespace DataAccess;

static bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2);

extern string NumProcessorsArg;

Migrator::Migrator (const string & name)//, DataAccess::Interface::ObjectFactory* objectFactory)
{
   ObjectFactory * objectFactory = new ObjectFactory(this);

   m_projectHandle.reset (dynamic_cast<GeoPhysics::ProjectHandle *> (Interface::OpenCauldronProject(name, "rw", objectFactory) ));

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

      if (!m_trapIoTbl) m_trapIoTbl = m_projectHandle->getTable ("TrapIoTbl");
      m_trapIoTbl->clear ();

      if (!m_migrationIoTbl) m_migrationIoTbl = m_projectHandle->getTable ("MigrationIoTbl");
      m_migrationIoTbl->clear ();
   }
   
   bool started = m_projectHandle->startActivity (activityName, m_projectHandle->getHighResolutionOutputGrid ());
   if (!started) return false;

   ios::fmtflags f (std::cout.flags ());
   std::cout << std::setfill (' ');
   std::cout.flags ( f );
      
   if (!started) return false;
  
   openMassBalanceFile ();

   ComputeRanks (m_projectHandle->getActivityOutputGrid ());

   if (!setUpBasinGeometry ()) return false;

   m_verticalMigration = m_projectHandle->getRunParameters ()->getVerticalSecondaryMigration ();
   m_hdynamicAndCapillary = m_projectHandle->getRunParameters ()->getHydrodynamicCapillaryPressure ();
   m_reservoirDetection = m_projectHandle->getRunParameters ()->getReservoirDetection ();

   bool pressureRun = isPressureRun ();

   // pressureRun set to false only here to avoid calculating overpressure before formation nodes are created.
   computeFormationPropertyMaps (m_projectHandle->getSnapshots ()->front (), false);

   createFormationNodes ();

   // compute the positions of the reservoirs within the formations
   computeDepthOffsets ();
   computeNetToGross ();

   removeComputedFormationPropertyMaps ();

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

      if (!performSnapshotMigration (start, end, pressureRun))
         return false;
   }
      
   m_propertyManager->removeProperties (end);
   m_projectHandle->deletePropertyValueGridMaps (Interface::SURFACE | Interface::FORMATION | Interface::FORMATIONSURFACE | Interface::RESERVOIR,
                                                 0, end, 0, 0, 0, Interface::MAP | Interface::VOLUME);

   delete snapshots;

   closeMassBalanceFile ();

   m_projectHandle->finishActivity ();

   m_projectHandle->setSimulationDetails("fastmig", "Default", "");

   bool status = true;
   if (!mergeOutputFiles ()) {
      PetscPrintf (PETSC_COMM_WORLD, "MeSsAgE ERROR Unable to merge output files\n");
      status = false;
   }

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
	bool HydrostaticCalculation = false;
	const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastSimulation ("fastcauldron");

	if (lastFastcauldronRun != 0)
	{
		HydrostaticCalculation = lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" or
			lastFastcauldronRun->getSimulatorMode () == "HydrostaticHighResDecompaction" or
			lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" or
			lastFastcauldronRun->getSimulatorMode () == "HydrostaticDarcy";
	}

	// From GeoPhysics::ProjectHandle
	if (!m_projectHandle->initialise () ||
		!m_projectHandle->setFormationLithologies (true, true) ||
		!m_projectHandle->initialiseLayerThicknessHistory (!HydrostaticCalculation) || // Backstripping
		!m_projectHandle->applyFctCorrections ())
		return false;
	else
		return true;
}

bool Migrator::computeFormationPropertyMaps (const Interface::Snapshot * snapshot, bool isPressureRun)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;

   Interface::GridMap * topDepthGridMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, m_projectHandle->getActivityOutputGrid (), Interface::DefaultUndefinedMapValue, 1);

   if (!getSeaBottomDepths(topDepthGridMap, snapshot))
   {
      return false;
   }

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation *formation = Formation::CastToFormation (*formationIter);

      assert (formation);
      if (!formation->isActive (snapshot)) continue;

      bool lowResEqualsHighRes = ((*(m_projectHandle->getLowResolutionOutputGrid ())) == (*(m_projectHandle->getHighResolutionOutputGrid ())));

      formation->computePropertyMaps (topDepthGridMap, snapshot, lowResEqualsHighRes, isPressureRun, m_projectHandle->getRunParameters ()->getNonGeometricLoop (),
                                      m_projectHandle->getRunParameters ()->getChemicalCompaction ()); // allowed to fail

      formation->computeHCDensityMaps ();

      if (m_hdynamicAndCapillary or m_reservoirDetection)
      {
         if (!formation->computeCapillaryPressureMaps (topDepthGridMap, snapshot))
            return false;
      }

   }
   return true;

}

bool Migrator::removeComputedFormationPropertyMaps (void)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);

      assert (formation);

      formation->removeComputedPropertyMaps ();
   }
   return true;
}

bool Migrator::getSeaBottomDepths (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot)
{
   if (!topDepthGridMap->retrieveData ()) return false;
   const Interface::Grid * grid = m_projectHandle->getActivityOutputGrid ();
   for (unsigned int i = grid->firstI (); i <= grid->lastI (); ++i)
   {
      for (unsigned int j = grid->firstJ (); j <= grid->lastJ (); ++j)
      {
         double seaBottomDepth = m_projectHandle->getSeaBottomDepth (i, j, snapshot->getTime ());

         if (seaBottomDepth == Interface::DefaultUndefinedMapValue || seaBottomDepth == Interface::DefaultUndefinedScalarValue)
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

bool Migrator::performSnapshotMigration (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool pressureRun)
{
   if (activeReservoirs (end)||m_reservoirDetection)
   {
      clearFormationNodeProperties ();

      if (!computeFormationPropertyMaps (end, pressureRun) ||
          !retrieveFormationCapillaryPressureMaps (end) ||
          !retrieveFormationPropertyMaps (end) ||
          !computeFormationNodeProperties (end) ||
          !detectReservoirs (end, pressureRun) ||
          !computeSMFlowPaths (start, end) ||
          !restoreFormationPropertyMaps (end) ||
          !loadExpulsionMaps (start, end) ||
          !chargeReservoirs (start, end) ||
          !restoreFormationCapillaryPressureMaps (end) ||
          !unloadExpulsionMaps (end) ||
          !saveSMFlowPaths (start, end) ||
          !removeComputedFormationPropertyMaps ())
      {
         return false;
      }
   }

   m_projectHandle->continueActivity ();

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

	for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
	{
		Reservoir * reservoir = (Reservoir *) * reservoirIter;

		assert (reservoir);

		reservoir->computeDepthOffsets (m_projectHandle->findSnapshot (0.));
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
   return (Reservoir *) (*reservoirs)[index];
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
      const Reservoir * reservoir = (const Reservoir *) * reservoirIter;
      
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

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);

      if (!formation->isActive (end)) continue;

      if (!formation->retrievePropertyMaps ()) return false;
   }

   return true;
}

bool Migrator::retrieveFormationCapillaryPressureMaps (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);

      if (!formation->isActive (end)) continue;

      if (!formation->retrieveCapillaryPressureMaps ()) return false;
   }

   return true;
}

bool Migrator::restoreFormationPropertyMaps (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);
      if (!formation->isActive (end)) continue;

      formation->restorePropertyMaps ();
   }

   return true;
}

bool Migrator::restoreFormationCapillaryPressureMaps (const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();
   Interface::FormationList::iterator formationIter;

   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation * formation = Formation::CastToFormation (*formationIter);
      if (!formation->isActive (end)) continue;

      formation->restoreCapillaryPressureMaps ();
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

migration::Formation * Migrator::getTopActiveReservoirFormation (const Interface::Snapshot * end)
{
   Interface::ReservoirList * reservoirs = getReservoirs ();
   Interface::ReservoirList::iterator reservoirIter;

   Reservoir * topActiveReservoir = 0;
   Formation * topActiveReservoirFormation = 0;

   for (reservoirIter = reservoirs->begin (); reservoirIter != reservoirs->end (); ++reservoirIter)
   {
      Reservoir * reservoir = (Reservoir *) (*reservoirIter);
      if (reservoir->isActive (end))
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
      Reservoir * reservoir = (Reservoir *) (*reservoirIter);
      if (reservoir->isActive (end))
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

// If getMinOilColumnHeight and getMinGasColumnHeight get moved to RunOptionsIoTbl these functions can be used
/*
  double Migrator::getMinOilColumnHeight (void) const
  {
  return m_projectHandle->getRunParameters ()->getMinOilColumnHeight ();
  }

  double Migrator::getMinGasColumnHeight (void) const
  {

  return m_projectHandle->getRunParameters ()->getMinGasColumnHeight ();
  }
*/

/*
  Reservoir definition based on capillary pressure evaluation.
  Begin from bottom most active reservoir. Go up. Looping through
  the uppermost cells of each formation and check the capillary pressure jump
  ( difference between 0% saturation capillary pressure at current formation
  and 100% capillary pressure at lowermost cells of formation above ).
*/
bool Migrator::detectReservoirs (const Interface::Snapshot * end, const bool pressureRun)
{

   // first, find the the bottommost RESERVOIR formation where HC can go
   Formation *bottomSourceRockFormation = getBottomSourceRockFormation ();

   if (bottomSourceRockFormation == 0)
      return false;

   Formation * bottomFormation = 0;
   Formation * belowBelowSourceRockFormation = 0;
   Formation * belowSourceRockFormation = bottomSourceRockFormation->getBottomFormation ();
   if (belowSourceRockFormation)
      belowBelowSourceRockFormation = belowSourceRockFormation->getBottomFormation ();

   if (belowBelowSourceRockFormation != 0)
      bottomFormation = belowBelowSourceRockFormation; //+2
   else
   {
      if (belowSourceRockFormation != 0)
         bottomFormation = belowSourceRockFormation;  // +1
      else
         bottomFormation = bottomSourceRockFormation; // SR
   }

   if (bottomFormation == 0) return false;

   //cerr << " bottomFormation ard is "<< bottomFormation->getName () << endl;

   // Second, find the topmost SEAL formation
   Formation *topActiveFormation = getTopActiveFormation (end);
   Formation * topSealFormation = 0;

   if (topActiveFormation)
   {
      topSealFormation = topActiveFormation;
      assert (topActiveFormation);
   }

   //cerr << " topActiveFormation ard is "<< topActiveFormation->getName () << endl;

   //Loop over the formations, to identify the Reservoirs, with these requirements:
   // 1. they must be not source rock formations?

   Formation *reservoirFormation;
   Formation *sealFormation;
   Interface::ReservoirList *  reservoirsBeforeDetection = getReservoirs ();

   int numDetectedReservoirs = 0;

   bool topSealFormationReached = false;
   for (reservoirFormation = bottomFormation, sealFormation = (Formation *) reservoirFormation->getTopFormation ();
        sealFormation != 0 && !topSealFormationReached;
        reservoirFormation = sealFormation, sealFormation = (Formation *) sealFormation->getTopFormation ())
   {
      //check if top seal formation is reached
      topSealFormationReached = (sealFormation == topSealFormation);

      assert (reservoirFormation);

      //Assigns top-row nodes of user-selected reservoirs as reservoir nodes, no need to identify the Reservoir here. Do this only for user-defined reservoirs
      if (!getReservoirs (reservoirFormation)->empty ())
      {
         reservoirFormation->identifyAsReservoir ();
         continue;
      }

      if (reservoirFormation->isSourceRock ()) continue;

      if (m_reservoirDetection)
      {
         // In Reservoir formation flag specified nodes as reservoirs for oil or gas
         reservoirFormation->detectReservoir (sealFormation, MinColumnHeight, MinColumnHeight, pressureRun);

         // If the formation is already detected skip calculations otherwise detect crests that can hold hc
         if (!reservoirFormation->getDetectedReservoir () && reservoirFormation->detectReservoirCrests ())
         {
            // cerr << "Formation " << reservoirFormation->getName() << " detected" << endl;
            // if the formation is detected as reservoir, add it in the reservoir list 
            reservoirFormation->addDetectedReservoir ();
            numDetectedReservoirs += 1;
         }
         // print to file information about the reservoirs 
         // reservoirFormation->saveReservoir (end);
      }
   }


   if (numDetectedReservoirs > 0)
   {
      // connect the reservoirs
      m_projectHandle->connectReservoirs ();

      Interface::ReservoirList *  reservoirsAfterDetection = getReservoirs ();

      // if there were no reservoirs before the detection, computeDepthOffsets and computeNetToGross for all new detected reservoirs (numDetectedReservoirs > 0)
      if ( reservoirsBeforeDetection || !reservoirsBeforeDetection->empty ())
      {
         // find the new reservoirs
         Interface::ReservoirList::iterator iter;
         Interface::ReservoirList::iterator reservoirIter;
         for (iter = reservoirsAfterDetection->begin (); iter != reservoirsAfterDetection->end (); ++iter)
         {
            // the reservoir was not in the list
            if (!binary_search (reservoirsBeforeDetection->begin (), reservoirsBeforeDetection->end (), *iter, reservoirSorter))
            {
               Reservoir * reservoir = (Reservoir *) * iter;
               reservoir->computeDepthOffsets (m_projectHandle->findSnapshot (0.));
               reservoir->computeNetToGross ();
            }
         }
      }
      else
      {
         // computeDepthOffsets and computeNetToGross for all detected reservoirs
         computeDepthOffsets ();
         computeNetToGross ();
      }
   }

   return true;
}

// Unflag detected reservoirs nodes 

bool Migrator::computeSMFlowPaths (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Formation * bottomSourceRockFormation = getBottomSourceRockFormation ();
   if (!bottomSourceRockFormation) return false;

   Formation * topActiveFormation = getTopActiveFormation (end);
   if (!topActiveFormation) return false;

   if (!computeSMFlowPaths (topActiveFormation, bottomSourceRockFormation, start, end)) return false;

   if (!m_verticalMigration)
   {
      if (!computeTargetFormationNodes (topActiveFormation, bottomSourceRockFormation)) return false;
   }
   return true;
}

bool Migrator::saveSMFlowPaths (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Formation * bottomSourceRockFormation = getBottomSourceRockFormation ();
   if (!bottomSourceRockFormation) return false;

   Formation * topActiveFormation = getTopActiveFormation (end);
   if (!topActiveFormation) return false;

   bottomSourceRockFormation->saveComputedSMFlowPaths (topActiveFormation, end);

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
      reservoir = (Reservoir *) * reservoirIter;

      reservoirAboveIter = reservoirIter;
      if (++reservoirAboveIter != reservoirs->end ())
         reservoirAbove = (Reservoir *) * reservoirAboveIter;
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

   reservoir->refineGeometry ();

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

   reservoir->migrateChargesToBeMigrated (0, reservoir);
   reservoir->clearPreviousProperties ();

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

   /// For each column in the trap set the diffusion starting time
   reservoir->broadcastTrapDiffusionStartTimes ();

   /// For each column in the trap set the new penetration distances
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
      string str = string ("Reservoir: ") + reservoir->getName ();
      
      m_massBalance->printMassBalance (start, end, str); 
   }

   if (GetRank () == 0) m_massBalance->clear ();

   reservoir->saveTrapProperties (saveSnapshot);
   reservoir->computeFluxes ();
   reservoir->saveComputedOutputProperties (saveSnapshot);

   return true;
}

// collect expelled charges into reservoirs from the formations that are
// above reservoirBelow and not above or just above reservoir
bool Migrator::collectAndMigrateExpelledCharges (migration::Reservoir * reservoir, migration::Reservoir * reservoirAbove, migration::Reservoir * reservoirBelow,
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
      if (withinRange || formationBelow == reservoir->getFormation () ||
          formationBelowBelow == reservoir->getFormation ())
      {
         withinRange = true;
      }
      else
      {
         continue;
      }

      unsigned int directionsToCollect = EXPELLEDNONE; // 0

      if (formationBelow == reservoir->getFormation () || formationBelowBelow == reservoir->getFormation ())
      {
         // source rock atmost 2 formations above reservoir

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
         if ((directionsToCollect & EXPELLEDUPWARD) && !m_verticalMigration)
         {
            formation->migrateChargesToReservoir (directionsToCollect, reservoir);
         }
         else
         {
            reservoir->collectExpelledCharges (formation, directionsToCollect, barrier);
         }

         reservoir->migrateChargesToBeMigrated (formation, 0);
      }

      if (barrier && (directionsToCollect & EXPELLEDUPWARD))
         barrier->updateBlocking (formation, end);

		if (reservoirBelow && reservoirBelow->isActive (end) &&
			formationBelow == reservoirBelow->getFormation () &&
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
         for (int componentId = pvtFlash::FIRST_COMPONENT; componentId < pvtFlash::NUM_COMPONENTS; ++componentId)
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

bool Migrator::loadExpulsionMaps (const Interface::Snapshot * start, const Interface::Snapshot * end)
{
   Interface::FormationList * formations = getAllFormations ();

   Interface::FormationList::iterator formationIter;
   for (formationIter = formations->begin (); formationIter != formations->end (); ++formationIter)
   {
      Formation *formation = Formation::CastToFormation (*formationIter);
      if (formation->isSourceRock () && formation->isActive (end))
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
      if (formation->isSourceRock () && formation->isActive (end))
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
		// Sort evertime a detected reservoir is added/removed to the list 
		if (!m_reservoirs || m_reservoirs->empty () || m_reservoirs->size () != m_projectHandle->getReservoirs (0)->size())
		{
			m_reservoirs = m_projectHandle->getReservoirs (0);

			if (!m_reservoirs->empty ())
				std::sort (m_reservoirs->begin (), m_reservoirs->end (), reservoirSorter);
		}
		return m_reservoirs;
	}
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
   m_projectHandle->getActivityOutputGrid ()->getPosition ((unsigned int) tpRequest.i, (unsigned int) tpRequest.j, posI, posJ);

   database::setXCoord (trapIoRecord, posI);
   database::setYCoord (trapIoRecord, posJ);

   database::setTrapCapacity (trapIoRecord, tpRequest.capacity);
   database::setDepth (trapIoRecord, tpRequest.depth);
   database::setSpillDepth (trapIoRecord, tpRequest.spillDepth);

   double spillPointPosI = 0, spillPointPosJ = 0;
   m_projectHandle->getActivityOutputGrid ()->getPosition ((unsigned int) tpRequest.spillPointI, (unsigned int) tpRequest.spillPointJ, spillPointPosI, spillPointPosJ);

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
      trapIoRecord->setValue (fieldName, tpRequest.composition.getWeight ((pvtFlash::ComponentId) i));
   }
}


/// \brief This function adds a  reservoir record to ReservoirIoTbl with default parameters. Note that this will change, bacause we want the user to specify 
/// these parameters for all detected reservoirs.

database::Record * Migrator::addDetectedReservoirRecord (migration::Formation * formation)
{
	if (!m_ReservoirIoTbl) m_ReservoirIoTbl = m_projectHandle->getTable("ReservoirIoTbl");
	assert (m_ReservoirIoTbl);
	Record * reservoirIoRecord = m_ReservoirIoTbl->createRecord ();
	assert (reservoirIoRecord);

	string detectedReservoirName = formation->getName () + "_det";

	database::setReservoirName (reservoirIoRecord, detectedReservoirName);
	database::setFormationName (reservoirIoRecord, formation->getName ()); 
	database::setTrapCapacity (reservoirIoRecord, 1); 
	database::setActivityMode (reservoirIoRecord, "AlwaysActive"); 

	database::setActivityStart (reservoirIoRecord, 0); 
	database::setDepthOffset (reservoirIoRecord,-9999);
	database::setDepthOffsetGrid (reservoirIoRecord,"");
	database::setThickness (reservoirIoRecord,-9999);
	database::setThicknessGrid (reservoirIoRecord,"");
	database::setNetToGross (reservoirIoRecord,-9999);
	database::setNetToGrossGrid (reservoirIoRecord,"");
	database::setMicroTraps (reservoirIoRecord,-9999);
	database::setLeakProperty (reservoirIoRecord,"");
	database::setLeakRate (reservoirIoRecord,-9999);
	database::setLithotype1 (reservoirIoRecord,"");
	database::setPercent1 (reservoirIoRecord,-9999);
	database::setPercent1Grid (reservoirIoRecord,"");
	database::setLithotype2 (reservoirIoRecord,"");
	database::setPercent2 (reservoirIoRecord,-9999);
	database::setPercent2Grid (reservoirIoRecord,"");
	database::setLithotype3 (reservoirIoRecord,"");
	database::setLayerFrequency (reservoirIoRecord,-9999);
	database::setLayerFrequencyGrid (reservoirIoRecord,"");
	database::setBioDegradInd (reservoirIoRecord,0);
	database::setOilToGasCrackingInd (reservoirIoRecord,1);
	database::setDiffusionInd (reservoirIoRecord,1);
	database::setMinOilColumnHeight (reservoirIoRecord,0.5);
	database::setMinGasColumnHeight (reservoirIoRecord,0.5);
	database::setBlockingInd (reservoirIoRecord,0);
	database::setBlockingPermeability (reservoirIoRecord,1e-9);
	database::setBlockingPorosity (reservoirIoRecord,0);
	database::setErrDepthOffset (reservoirIoRecord,0);
	database::setErrThickness (reservoirIoRecord,0);
	database::setErrMicroTraps (reservoirIoRecord,0);
	database::setErrLayerFrequency (reservoirIoRecord,0);

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
      fieldName += TableComponentNames[component];

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
      fieldName += TableComponentNames[component];

      double mass = (newlyCreated ? 0 : record->getValue<double> (fieldName));
      mass += mr.composition.getWeight ((pvtFlash::ComponentId) component);

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
      fieldName += TableComponentNames[component];

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
      else if (getSourceAge (*iter) < 0 || getDestinationAge (*iter) < 0)
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
      else if (migrationProcess == MigrationProcessNames[BIODEGRADATION] ||
               migrationProcess == MigrationProcessNames[DIFFUSION] ||
               migrationProcess == MigrationProcessNames[OILTOGASCRACKINGLOST] ||
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
         if (getSourceTrapID (record) == oldTrapNumber && m_projectHandle->findSnapshot (getSourceAge (record)) == snapshot)
         {
            setSourceTrapID (record, newTrapNumber);
         }

         if (getDestinationTrapID (record) == oldTrapNumber && m_projectHandle->findSnapshot (getDestinationAge (record)) == snapshot)
         {
            setDestinationTrapID (record, newTrapNumber);
         }
      }
   }
}

bool Migrator::isPressureRun (void) const
{
   const Property* prop = m_projectHandle->findProperty ("FCTCorrection");
   assert (prop);

   Interface::PropertyValueList * propertyValues =
      m_projectHandle->getPropertyValues (Interface::FORMATION, prop, m_projectHandle->getSnapshots ()->front (), 0, 0, 0, Interface::MAP);

   bool isNotEmpty = !propertyValues->empty ();

   delete propertyValues;
   return isNotEmpty;
}

const Interface::GridMap * Migrator::getPropertyGridMap (const string & propertyName, const Interface::Snapshot * snapshot,
                                                         const Interface::Reservoir * reservoir,
                                                         const Interface::Formation * formation, const Interface::Surface * surface)
{
   int selectionFlags = 0;

   if (reservoir) selectionFlags |= Interface::RESERVOIR;
   if (formation && !surface) selectionFlags |= Interface::FORMATION;
   if (surface && !formation) selectionFlags |= Interface::SURFACE;
   if (formation && surface) selectionFlags |= Interface::FORMATIONSURFACE;

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

void Migrator::clearMigrationRecordLists (void)
{
   assert (GetRank () == 0);
   for (int i = 0; i < NUMBEROFPROCESSES; ++i)
   {
      m_migrationRecordLists[i].clear ();
   }
}

bool reservoirSorter (const Interface::Reservoir * reservoir1, const Interface::Reservoir * reservoir2)
{
   return reservoir1->getFormation()->getDepositionSequence() < reservoir2->getFormation()->getDepositionSequence();
}

bool Migrator::mergeOutputFiles () {

   if (!H5_Parallel_PropertyList::isOneFilePerProcessEnabled ())
   {
      return true;
   }

#ifndef _MSC_VER
   PetscBool noFileCopy = PETSC_FALSE;

   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );

   string filePathName = m_projectHandle->getProjectPath () + "/" + m_projectHandle->getOutputDir () + "/" + MigrationActivityName + "_Results.HDF";

   bool status = mergeFiles ( allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), ( noFileCopy ? CREATE : REUSE ) ));

   if( status )
   {
      status = H5_Parallel_PropertyList::copyMergedFile( filePathName ); 
   }

   if( ! status )
   {
      PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not merge the file %s.\n", filePathName.c_str() );               
   }

   else
   {
      ReportProgress ("Merged Output Maps");
   }

   return status;
#else
   return true;
#endif
      }


