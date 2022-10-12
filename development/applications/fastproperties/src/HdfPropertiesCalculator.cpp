//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "HdfPropertiesCalculator.h"

#include "DerivedPropertyManager.h"
#include "FilePath.h"
#include "GeoPhysicsFormation.h"
#include "h5_parallel_file_types.h"
#include "OutputPropertyValue.h"

#include <sys/stat.h>

using namespace Interface;
using namespace std;
//------------------------------------------------------------//
HdfPropertiesCalculator::HdfPropertiesCalculator(int aRank) : AbstractPropertiesCalculator(aRank)
{

   m_copy              = false;
   m_projectProperties = false;
}

//------------------------------------------------------------//
void HdfPropertiesCalculator::calculateProperties(FormationSurfaceVector& formationItems, Interface::PropertyList properties, Interface::SnapshotList & snapshots)
{
   if (properties.size () == 0)
   {
      return;
   }

   SnapshotFormationSurfaceOutputPropertyValueMap allOutputPropertyValues;

   if (snapshots.empty())
   {
      const Snapshot * zeroSnapshot = getProjectHandle().findSnapshot(0);

      snapshots.push_back(zeroSnapshot);
   }

   struct stat fileStatus;
   int fileError;

   for ( const Interface::Snapshot* snapshot : snapshots )
   {
      displayProgress(snapshot->getFileName (), m_startTime, "Start computing ");

      if (snapshot->getFileName () != "")
      {
         ibs::FilePath fileName(getProjectHandle().getFullOutputDir ());
         fileName << snapshot->getFileName ();
         fileError = stat (fileName.cpath(), &fileStatus);

         ((Snapshot *)snapshot)->setAppendFile (not fileError);
      }


      for ( const FormationSurface& formationIter : formationItems )
      {
         const Interface::Formation * formation = formationIter.first;
         const Interface::Surface   * surface   = formationIter.second;
         const Interface::Snapshot  * bottomSurfaceSnapshot = (formation->getBottomSurface() != 0 ? formation->getBottomSurface()->getSnapshot() : 0);

         if (snapshot->getTime() != 0.0 and surface == 0 and bottomSurfaceSnapshot != 0)
         {
            const double depoAge = bottomSurfaceSnapshot->getTime();
            if (snapshot->getTime() > depoAge or fabs(snapshot->getTime() - depoAge) < snapshot->getTime() * 1e-9)
            {
               continue;
            }
         }

         for ( const Interface::Property* property : properties )
         {
            if (m_no3Dproperties and surface == 0 and property->getPropertyAttribute() != DataModel::FORMATION_2D_PROPERTY)
            {
               continue;
            }

            if (not m_extract2D and surface != 0 and property->getPropertyAttribute() != DataModel::SURFACE_2D_PROPERTY)
            {
               continue;
            }

            if (not m_projectProperties or (m_projectProperties and allowOutput(property->getName(), formation, surface)))
            {
               resetProjectActivityGrid (property);
               OutputPropertyValuePtr outputProperty = DerivedProperties::allocateOutputProperty (getPropertyManager(), property, snapshot, formationIter, m_basement);
               resetProjectActivityGrid ();

               if (outputProperty != 0)
               {
                  if (m_debug && m_rank == 0)
                  {
                     LogHandler(LogHandler::INFO_SEVERITY) << "Snapshot: " << snapshot->getTime() <<
                        " allocate " << property->getName() << " " << (formation != 0 ? formation->getName() : "") << " " <<
                        (surface != 0 ? surface->getName() : "");
                  }
                  allOutputPropertyValues [ snapshot ][ formationIter ][ property ] = outputProperty;
               }
               else
               {
                  if (m_debug && m_rank == 0)
                  {
                     LogHandler(LogHandler::INFO_SEVERITY) << "Could not calculate derived property " << property->getName()
                                                             << " @ snapshot " << snapshot->getTime() << "Ma for formation " <<
                        (formation != 0 ? formation->getName() : "") << " " <<  (surface != 0 ? surface->getName() : "") << ".";
                  }
               }
            }
         }


         DerivedProperties::outputSnapshotFormationData(getProjectHandle(), snapshot, formationIter, properties, allOutputPropertyValues);
      }

      removeProperties(snapshot, allOutputPropertyValues);
      getPropertyManager().removeProperties(snapshot);

      displayProgress(snapshot->getFileName (), m_startTime, "Start saving ");

      getProjectHandle().continueActivity();

      displayProgress(snapshot->getFileName (), m_startTime, "Saving is finished for ");

      getProjectHandle().deletePropertiesValuesMaps (snapshot);

      Utilities::CheckMemory::StatisticsHandler::update ();
   }

   PetscLogDouble End_Time;
   PetscTime(&End_Time);

   displayTime(End_Time - m_startTime, "Total derived properties saving: ");
}

//------------------------------------------------------------//



//------------------------------------------------------------//
bool HdfPropertiesCalculator::copyFiles() {

   if (not H5_Parallel_PropertyList::isPrimaryPodEnabled () or (H5_Parallel_PropertyList::isPrimaryPodEnabled () and not m_copy))
   {
      return true;
   }

   int rank;
   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

   if (rank != 0) return true;

   PetscBool noFileRemove = PETSC_FALSE;
   PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-noremove", &noFileRemove);

   PetscLogDouble StartMergingTime;
   PetscTime(&StartMergingTime);
   bool status = true;

   const std::string& directoryName = getProjectHandle().getOutputDir ();

   PetscPrintf (PETSC_COMM_WORLD, "Copy output files ...\n");

   SnapshotList * snapshots = getProjectHandle().getSnapshots(MAJOR | MINOR);
   SnapshotList::iterator snapshotIter;

   for (snapshotIter = snapshots->begin(); snapshotIter != snapshots->end(); ++snapshotIter)
   {
      const Interface::Snapshot * snapshot = *snapshotIter;

      if (snapshot->getFileName () == "")
      {
         continue;
      }
      ibs::FilePath filePathName(getProjectHandle().getProjectPath ());
      filePathName << directoryName << snapshot->getFileName ();

      displayProgress(snapshot->getFileName (), StartMergingTime, "Copy ");

      status = H5_Parallel_PropertyList::copyMergedFile(filePathName.path(), false);

      // delete the file in the shared scratch
      if (status and not noFileRemove)
      {
         ibs::FilePath fileName(H5_Parallel_PropertyList::getTempDirName());
         fileName << filePathName.cpath ();

         int status = std::remove(fileName.cpath()); //c_str ());
         if (status == -1)
            cerr << fileName.cpath() << " Basin_Warning:  Unable to remove snapshot file, because '"
                 << std::strerror(errno) << "'" << endl;
      }
   }

   string fileName = m_activityName + "_Results.HDF" ;
   ibs::FilePath filePathName(getProjectHandle().getProjectPath ());
   filePathName <<  directoryName << fileName;

   displayProgress(fileName, StartMergingTime, "Copy ");

   status = H5_Parallel_PropertyList::copyMergedFile(filePathName.path(), false);

   // remove the file from the shared scratch
   if (status and  not noFileRemove)
   {

    ibs::FilePath fileName(H5_Parallel_PropertyList::getTempDirName());
      fileName << filePathName.cpath ();
      int status = std::remove(fileName.cpath());

      if (status == -1)
      {
         cerr << fileName.cpath () << " Basin_Warning:  Unable to remove file, because '"
              << std::strerror(errno) << "'" << endl;
      }

     // remove the output directory from the shared scratch
      ibs::FilePath dirName(H5_Parallel_PropertyList::getTempDirName());
      dirName << directoryName;

      displayProgress(dirName.path(), StartMergingTime, "Removing remote output directory ");
      status = std::remove(dirName.cpath());

      if (status == -1)
         cerr << dirName.cpath () << " Basin_Warning:  Unable to remove the directory, because '"
              << std::strerror(errno) << "'" << endl;
   }

   if (status)
   {
      displayTime(StartMergingTime, "Total merging time: ");
   }
   else
   {
      PetscPrintf (PETSC_COMM_WORLD, "  Basin_Error: Could not merge the file %s.\n", filePathName.cpath());
   }

   delete snapshots;

   return status;
}

//------------------------------------------------------------//
bool HdfPropertiesCalculator::parseCommandLine(int argc, char ** argv) {

   bool status = AbstractPropertiesCalculator::parseCommandLine (argc, argv);
   if (status)
   {
      PetscBool parameterDefined = PETSC_FALSE;

      PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-project-properties", &parameterDefined);
      if (parameterDefined) m_projectProperties = true;

      PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-copy", &parameterDefined);
      if (parameterDefined) m_copy = true;

      status = checkParameters();
   }

   return status;
}

//------------------------------------------------------------//
bool HdfPropertiesCalculator::checkParameters() {

   if (m_projectProperties)
   {
      m_all3Dproperties = true;
      m_all2Dproperties = true;
      // Do not extract and save 2D maps from 3d data, define -extract2D option
      // m_extract2D = false;
      m_basement = true;
   }

   return true;
}

