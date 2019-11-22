//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ExportToHDF.h"
#include "VisualizationIO_native.h"
#include "VisualizationUtils.h"
#include "DataStore.h"
#include "FilePath.h"
#include "FolderPath.h"
#include "Formation.h"
#include "Snapshot.h"
#include "Surface.h"
#include "Property.h"
#include "PropertyValue.h"
#include "CrustFormation.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>
#include "hdf5.h"

using namespace CauldronIO;

CauldronIO::ExportToHDF::ExportToHDF(const ibs::FilePath& absPath, const ibs::FilePath& relPath, size_t numThreads, const bool includeBasement, GeoPhysics::ProjectHandle& projectHandle, AbstractPropertiesCalculator * propCalculator)
   : m_fullPath(absPath), m_relPath(relPath), m_numThreads(numThreads), m_basement(includeBasement), m_projectHandle( projectHandle ), m_propCalculator( propCalculator )

{
    m_fullPath << relPath.path();
}

bool CauldronIO::ExportToHDF::exportToHDF(std::shared_ptr<Project>& project, const std::string& absPath, size_t numThreads, const bool includeBasement, GeoPhysics::ProjectHandle& projectHandle, AbstractPropertiesCalculator * propCalculator)
{
   H5Eset_auto( H5E_DEFAULT, 0, 0);

   ibs::FilePath outputPath(absPath);
   ibs::FilePath folderPath = outputPath.filePath();
   std::string filename = outputPath.fileName();
   std::string filenameNoExtension = outputPath.fileNameNoExtension();
   filenameNoExtension += "_vizIO_output";
   folderPath << filenameNoExtension;

   // Create output directory if not existing
   if (!folderPath.exists())
   {
      ibs::FolderPath(folderPath.path()).create();
   }

   ExportToHDF newExport(outputPath.filePath(), filenameNoExtension, numThreads, includeBasement, projectHandle, propCalculator);

   // Create xml property tree and write datastores
   newExport.addProject(project);

   return true;
}

void CauldronIO::ExportToHDF::addProject(std::shared_ptr<Project>& project )
{
   m_project = project;

   // Write all snapshots
   const SnapShotList snapShotList = project->getSnapShots();
   for (auto& snapShot : snapShotList)
   {
      addSnapShot(snapShot);
   }

}

void CauldronIO::ExportToHDF::addSurface( const std::shared_ptr<SnapShot>& snapshot, const std::shared_ptr<Surface>& surfaceIO)
{
   const std::string name = surfaceIO->getName();
   const std::string topFormName = ( surfaceIO->getTopFormation() ?  surfaceIO->getTopFormation()->getName() : "" );
   const std::string botFormName = ( surfaceIO->getBottomFormation() ?  surfaceIO->getBottomFormation()->getName() : "" );

#ifdef DEBUG
   std::cout << snapshot->getAge() <<":Adding surface " << name << " " << topFormName << " " << botFormName << std::endl;
#endif

   if (!surfaceIO->isRetrieved() )
      surfaceIO->retrieve();

   const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();

   if (valueMaps.size() > 0)
   {
      for (const PropertySurfaceData& propertySurfaceData: valueMaps)
      {
         writeMapsToHDF(snapshot, surfaceIO, propertySurfaceData);
      }
   }
}

void CauldronIO::ExportToHDF::addSnapShot(const std::shared_ptr<SnapShot>& snapshot)
{
   std::vector < VisualizationIOData* > data = snapshot->getAllRetrievableData();
   if( data.size() > 0 ) {
      std::cout << "Writing to hdf snapshot " << snapshot->getAge() << std::endl;
   }
   CauldronIO::VisualizationUtils::retrieveAllData(data, 1);

   // Add surfaces
   const SurfaceList surfaces = snapshot->getSurfaceList();
   if (surfaces.size() > 0)
   {
      for (const std::shared_ptr<Surface>& surfaceIO : surfaces)
      {
         // Data storage
         addSurface(snapshot, surfaceIO);
      }
   }

   // Add the continuous volume
   const std::shared_ptr<Volume> volume = snapshot->getVolume();
   if (volume)
   {
      writeContVolToHDF( snapshot, volume );
   }

   // Add a volume per formation, with discontinuous properties
   FormationVolumeList formVolumes = snapshot->getFormationVolumeList();
   if (formVolumes.size() > 0)
   {
      for(FormationVolume& formVolume: formVolumes)
      {
         // Only add a volume if it contains something
         if (formVolume.second->getPropertyVolumeDataList().size() > 0)
         {

            const std::shared_ptr<Volume> subVolume = formVolume.second;
            const std::shared_ptr<const Formation> subFormation = formVolume.first;

            writeDiscVolToHDF( snapshot, subFormation, subVolume );
         }
      }
   }
   // Release all data
   snapshot->release();
}

void CauldronIO::ExportToHDF::writeMapsToHDF( const std::shared_ptr<SnapShot>& snapShot,  const std::shared_ptr<Surface>& surfaceIO, const PropertySurfaceData &propertySurfaceData ) {

   boost::filesystem::path pathToOutput(m_relPath.cpath());
   pathToOutput /= "Maps.HDF";

   const std::string fileName = pathToOutput.string();

   hid_t   file_id, dataset_id, dataspace_id;

   /* Open an existing file. */
   file_id = H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
   if( file_id < 0 ) {
      file_id = H5Fcreate(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
   }

   const std::string name = surfaceIO->getName();
   const std::string topFormName = ( surfaceIO->getTopFormation() ?  surfaceIO->getTopFormation()->getName() : "" );
   const std::string botFormName = ( surfaceIO->getBottomFormation() ?  surfaceIO->getBottomFormation()->getName() : "" );
   std::shared_ptr< CauldronIO::SurfaceData> valueMap = propertySurfaceData.second;
   std::string pname = propertySurfaceData.first->getCauldronName();

   hsize_t dims[2];

   std::string fname = (valueMap->getFormation() ? valueMap->getFormation()->getName() : "" );
   std::string fnameCopy = fname;

   std::string::size_type pos;
   for (pos=0; pos < fnameCopy.length(); pos++) {

      if (fnameCopy[pos] == ' ') {
         fnameCopy[pos] = '_';
      }

   }
   std::string nameCopy = name;

   for (pos=0; pos < nameCopy.length(); pos++) {

      if (nameCopy[pos] == ' ') {
         nameCopy[pos] = '_';
      }

   }

   std::string datasetName = "/Layer=" + pname + "_" + std::to_string((int)snapShot->getAge()) + "_" + nameCopy + "_" + fnameCopy;

   std::shared_ptr<const Geometry2D> geometry = valueMap->getGeometry();
   dims[0] = geometry->getNumI();
   dims[1] = geometry->getNumJ();

#ifdef DEBUG
   std::cout << snapShot->getAge() << ": Save to hdf " << pname << " for " << "surface " << name << " form " << fname << std::endl;
#endif

   valueMap->retrieve();

   float * values = new float[dims[0] * dims[1]];
   unsigned int kk = 0;

   for (unsigned i = 0; i < dims[0]; ++ i ) {
      for (unsigned j = 0; j < dims[1]; ++ j ) {
         values[kk++] =  valueMap->getValue(i, j);
      }
   }

   valueMap->release();

   dataspace_id = H5Screate_simple(2, dims, NULL);
   dataset_id = H5Dcreate(file_id, datasetName.c_str(), H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

   /* Write the dataset. */
   H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, values);

   /* Close the dataset. */
   H5Sclose(dataspace_id);
   H5Dclose(dataset_id);

   /* Close the file. */
   H5Fclose(file_id);
}

void CauldronIO::ExportToHDF::writeDiscVolToHDF( const std::shared_ptr<SnapShot>& snapShot,  const std::shared_ptr<const Formation> formation, const std::shared_ptr<Volume> volume ) {

   boost::filesystem::path pathToOutput(m_relPath.cpath());
   const std::string snapshotFileName = "./Time_" + std::to_string(snapShot->getAge()) + ".h5";

   pathToOutput /= snapshotFileName;
   const std::string fileName = pathToOutput.string();

   hid_t       file_id, dataset_id, dataspace_id, group_id;

   PropertyVolumeDataList& propVolList = volume->getPropertyVolumeDataList();
   if( propVolList.size() > 0 ) {
      /* Open an existing file. */
      file_id = H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
      if( file_id < 0 ) {
         file_id = H5Fcreate(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
      }
      const std::string name = formation->getName();
      std::string fname = name;

      std::string::size_type pos;
      for (pos=0; pos < fname.length(); pos++) {

         if (fname[pos] == ' ') {
            fname[pos] = '_';
         }

      }
#ifdef DEBUG
      std::cout << snapShot->getAge() << " " << name << " " <<  propVolList.size() << std::endl;
#endif
      for(PropertyVolumeData& propVolume: propVolList) {
         std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
         std::string pname = propVolume.first->getName();
#ifdef DEBUG
         std::cout << "     " << pname << std::endl;
#endif
         hsize_t dims[3];

         std::string groupName = "/" + pname;
         std::string datasetName =  groupName + "/" + fname;

         std::shared_ptr<const Geometry3D> geometry = valueMap->getGeometry();
         dims[0] = geometry->getNumI();
         dims[1] = geometry->getNumJ();
         dims[2] = geometry->getNumK();

#ifdef DEBUG
         std::cout << snapShot->getAge() << ": Save to hdf " << pname << " for " << " form " << fname << std::endl;
#endif
         valueMap->retrieve();

         float * values = new float[dims[0] * dims[1] * dims[2]];
         unsigned int kk = 0;
         int firstK = static_cast<int>(geometry->getFirstK());
         int lastK = static_cast<int>(geometry->getFirstK() + dims[2] - 1);

         for (unsigned i = 0; i < dims[0]; ++ i ) {
            for (unsigned j = 0; j < dims[1]; ++ j ) {
               for (int k =  lastK; k >= firstK; -- k ) {
                  values[kk++] =  valueMap->getValue(i, j, k);
               }
            }
         }

         valueMap->release();

         group_id = H5Gopen( file_id, groupName.c_str(), H5P_DEFAULT );

         if( group_id < 0 ) {
            group_id =  H5Gcreate2( file_id, groupName.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
         }

         dataspace_id = H5Screate_simple(3, dims, NULL);
         dataset_id = H5Dopen( file_id,  datasetName.c_str(), H5P_DEFAULT);
         if( dataset_id < 0 ) {
            dataset_id = H5Dcreate(file_id, datasetName.c_str(), H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
         }

         /* Write the dataset. */
         H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, values);
         delete [] values;

         /* Close the dataset */
         H5Sclose(dataspace_id);
         H5Dclose(dataset_id);
         /* Close the group */
         H5Gclose(group_id);
      }
      /* Close the file. */
      H5Fclose(file_id);
   }
}

void CauldronIO::ExportToHDF::writeContVolToHDF( const std::shared_ptr<SnapShot>& snapShot, const std::shared_ptr<Volume> volume ) {

   boost::filesystem::path pathToOutput(m_relPath.cpath());
   const std::string snapshotFileName = "./Time_" + std::to_string(snapShot->getAge()) + ".h5";

   pathToOutput /= snapshotFileName;
   const std::string fileName = pathToOutput.string();

   hid_t   file_id, dataset_id, dataspace_id, group_id;

   double snapshotAge = snapShot->getAge();
   const DataAccess::Interface::Snapshot * nextSnapshot = m_projectHandle.findNextSnapshot( snapShot->getAge(), DataAccess::Interface::MAJOR );
   if( nextSnapshot != 0 ) {
     snapshotAge = nextSnapshot->getTime();
   }

   const FormationList& formations = m_project->getFormations();

   // Create a list of formations, check if basement should be included
   FormationList snapshotFormations;
   for(const std::shared_ptr<Formation>& formation: formations)
   {
      if( formation->getName() == "Crust" or  formation->getName() == "Mantle" ) {
         if( m_basement ) {
            snapshotFormations.push_back(formation);
#ifdef DEBUG
            std::cout <<  snapShot->getAge() << " Adding basement " << formation->getName() << std::endl;
#endif
         }
         continue;
      }
      const DataAccess::Interface::Formation * pformation = m_projectHandle.findFormation( formation->getName());
      if (pformation->getTopSurface() and pformation->getTopSurface()->getSnapshot()) {
         if( pformation->getTopSurface()->getSnapshot()->getTime() >= snapshotAge ) {

            if(( pformation->kind() == DataAccess::Interface::BASEMENT_FORMATION and pformation->getName() == "Crust" ) and not m_basement ) {
               continue;
            }
            snapshotFormations.push_back(formation);
#ifdef DEBUG
            std::cout <<  snapShot->getAge() << " Adding " << formation->getName() << std::endl;
#endif
         }
      } else {
         // add Mantle or Crust
         if( m_basement ) {
            snapshotFormations.push_back(formation);
#ifdef DEBUG
            std::cout <<  snapShot->getAge() << " Adding basement " << formation->getName() << std::endl;
#endif
         }
      }
   }

   PropertyVolumeDataList& propVolList = volume->getPropertyVolumeDataList();
   if( propVolList.size() > 0 ) {
      /* Open an existing file. */
      file_id = H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
      if( file_id < 0 ) {
         file_id = H5Fcreate(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
      }
      int  offset = 0;
      bool firstFormation = true;

      for(const std::shared_ptr<Formation>& formation: snapshotFormations) {
         const std::string name = formation->getName();
         std::string fname = name;

         std::string::size_type pos;
         for (pos=0; pos < fname.length(); pos++) {

            if (fname[pos] == ' ') {
               fname[pos] = '_';
            }

         }
#ifdef DEBUG
         std::cout << snapShot->getAge() << " " << name << " " <<  propVolList.size() << std::endl;
#endif
         // Find formation's part of the property volume and save as HDF
         for(PropertyVolumeData& propVolume: propVolList) {
            std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
            std::string pname = propVolume.first->getName();

            // Check if the property was calculated for the basement
            if ((name == "Crust" or name == "Mantle" ) and not m_propCalculator->allowBasementOutput(pname)) {
               continue;
            }

#ifdef DEBUG
            std::cout << "     " << pname << std::endl;
#endif
            hsize_t dims[3];

            std::string groupName = "/" + pname;
            std::string datasetName =  groupName + "/" + fname;

            std::shared_ptr<const Geometry3D> geometry = valueMap->getGeometry();

            int k_range_start, k_range_end;
            formation->getK_Range(k_range_start, k_range_end);
            if( firstFormation) {
               offset = k_range_start;
               firstFormation = false;
            }
            k_range_start -= offset;
            k_range_end -= offset;

            dims[0] = geometry->getNumI();
            dims[1] = geometry->getNumJ();
            dims[2] = k_range_end - k_range_start + 1;

            valueMap->retrieve();
            int firstK =  k_range_start;
            int lastK = k_range_end;

#ifdef DEBUG
            std::cout << snapShot->getAge() << ": Save to hdf " << pname << " for " << " form " << fname << " dims= " << dims[0] << "," <<  dims[1] <<
               "," << dims[2] << " (" << k_range_end << ", " <<  k_range_start << ")" << " geometry numK = " <<  geometry->getNumK() << " firstK = " << firstK << " lastK " << lastK << std::endl;
#endif
            float * values = new float[dims[0] * dims[1] * dims[2]];
            unsigned int kk = 0;

            for (unsigned int i = 0; i < dims[0]; ++ i ) {
               for (unsigned int j = 0; j < dims[1]; ++ j ) {
                  for (int k = lastK; k >= firstK; -- k ) {
                     values[kk++] = valueMap->getValue(i, j, k);
                  }
               }
            }

            group_id = H5Gopen( file_id, groupName.c_str(), H5P_DEFAULT );

            if( group_id < 0 ) {
               group_id =  H5Gcreate2( file_id, groupName.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            }

            dataspace_id = H5Screate_simple(3, dims, NULL);
            dataset_id = H5Dopen( file_id,  datasetName.c_str(), H5P_DEFAULT);
            if( dataset_id < 0 ) {
               dataset_id = H5Dcreate(file_id, datasetName.c_str(), H5T_NATIVE_FLOAT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            }

            /* Write the dataset. */
            H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, values);
            delete [] values;

            /* Close the dataset. */
            H5Sclose(dataspace_id);
            H5Dclose(dataset_id);
            /* Close the group. */
            H5Gclose(group_id);
         }
      }

      /* Close the file. */
      H5Fclose(file_id);
   }
}
