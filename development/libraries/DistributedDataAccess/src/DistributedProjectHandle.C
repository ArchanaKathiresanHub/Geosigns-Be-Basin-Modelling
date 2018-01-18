//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined (_WIN64)
#include <direct.h>
#endif

#include <cmath>
#include <algorithm>
#include <cerrno>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>

#include <petsc.h>
#include <mpi.h>

#include "Interface/ProjectHandle.h"
#include "Interface/Grid.h"
#include "Interface/DistributedGrid.h"
#include "Interface/DistributedGridMap.h"
#include "Interface/ObjectFactory.h"
#include "Interface/DistributedMessageHandler.h"
#include "Interface/DistributedApplicationGlobalOperations.h"

#include "h5_parallel_file_types.h"
#include "petscvector_readwrite.h"

#include "FolderPath.h"
#include "FilePath.h"

using namespace DataAccess;
using namespace Interface;
using namespace std;

const double DefaultUndefinedValue = 99999;
struct MapFileCache {
    string fileName;
    H5_ReadOnly_File gridMapFile;
    double undefinedValue;
    double rank;
    int numI, numJ;
    int depth;
};

void ProjectHandle::mapFileCacheConstructor (void)
{
    m_mapFileCache = new struct MapFileCache [4];
    for (int i = 0; i < 4; ++i) 
    {
        static_cast<struct MapFileCache * > (m_mapFileCache) [i].fileName = "";
    }
}
void ProjectHandle::mapFileCacheDestructor (void)
{
   if( m_mapFileCache != nullptr ) {
      for (int i = 0; i < 4; ++i) 
      {
         if (static_cast<struct MapFileCache * > (m_mapFileCache) [i].fileName != "")
         {
            static_cast<struct MapFileCache * > (m_mapFileCache) [i].gridMapFile.close ();
            static_cast<struct MapFileCache * > (m_mapFileCache) [i].fileName = "";
         }
      }
      delete[] static_cast<struct MapFileCache * > (m_mapFileCache);
      m_mapFileCache = nullptr;
   }
}

void ProjectHandle::checkForValidPartitioning (const string & name, int M, int N) const
{
   int m, n;

   int scalingFactor = 2;
   if (name == "CrustalThicknessCalculator" || name == "Genex5" || name == "Unknown")
   {
      scalingFactor = 1;
   }

   int M_ = M / scalingFactor;
   int N_ = N / scalingFactor;

   int size;
   MPI_Comm_size (PETSC_COMM_WORLD, &size);

   if (size == 1) return; // 1 is always okay!

   bool scalingFound = DistributedGrid::CalculatePartitioning (M_, N_, m, n);

   if ( M <= 1 or N <= 1 ) {
      PetscPrintf (PETSC_COMM_WORLD,
                   "\n Basin_Error Unable to partition a %d x %d grid, please select a larger number of grid nodes.\nThere must be at least two nodes in each direction. \n", M, N );

      PetscPrintf(PETSC_COMM_WORLD, "\nExiting ...\n\n");
      
      MPI_Finalize ();
      exit (-1);
   }


   if (!scalingFound)
   {
      PetscPrintf (PETSC_COMM_WORLD,
                   "\nUnable to partition a %d x %d grid using %d cores for activity %s, please select a different number of cores:\n", M, N, size, name.c_str());
      PetscPrintf(PETSC_COMM_WORLD, "\tSelect either 1 core or M * N cores where M <= %d and N <= %d.\n", max (1, M_), max (1, N_));

      if (name == "Unknown")
      {
    PetscPrintf(PETSC_COMM_WORLD, "\tPlease note that these numbers may still be too high (application-dependent)!\n");
      }
      PetscPrintf(PETSC_COMM_WORLD, "\nExiting ...\n\n");
      
      MPI_Finalize ();
      exit (-1);
   }
}

void ProjectHandle::allocateArchitectureRelatedParameters () {
   m_messageHandler = new DistributedMessageHandler;
   m_globalOperations = new DistributedApplicationGlobalOperations;
}

GridMap * ProjectHandle::loadGridMap (const Parent * parent, unsigned int childIndex,
                                      const string & filePathName, const string & dataSetName)
{
   H5Eset_auto (NULL, NULL, NULL);

   hid_t dataSetId = -1;
   hid_t dataSpaceId = -1;

   int numI;
   int numJ;

   struct MapFileCache * mapFileCachePtr = 0;

   if (filePathName.find ("HighResDecompaction_Results") != string::npos)
   {
      mapFileCachePtr = &  static_cast<struct MapFileCache * > (m_mapFileCache) [hrdecompaction];
      
   }
   else if (filePathName.find ("Genex5_Results") != string::npos ||
    filePathName.find ("Genex6_Results") != string::npos)
   {
      mapFileCachePtr = & static_cast<struct MapFileCache * > (m_mapFileCache) [genex];
   }
   else if (filePathName.find ("_Results.HDF") != string::npos) // then it must be fastcauldron...
   {
      mapFileCachePtr = & static_cast<struct MapFileCache * > (m_mapFileCache) [fastcauldron];
   }
   else
   {
      mapFileCachePtr = & static_cast<struct MapFileCache * > (m_mapFileCache) [auxiliary];
   }

   struct MapFileCache & mapFileCache = * mapFileCachePtr;


   if (mapFileCache.fileName != filePathName)
   {
      mapFileCache.rank = -1;   // needs to be invalidated
      if (mapFileCache.fileName != "")
      {
         mapFileCache.gridMapFile.close ();
         mapFileCache.fileName = "";
      }

      H5_Parallel_PropertyList propertyList;

      if (!mapFileCache.gridMapFile.open (filePathName.c_str (), &propertyList))
      {
         cerr << "ERROR in ProjectHandle::loadGridMap (): Could not open " << filePathName << endl;
         return 0;
      }

      mapFileCache.fileName = filePathName;
   }

   H5_ReadOnly_File & gridMapFile = mapFileCache.gridMapFile;
   hid_t fileId = gridMapFile.fileId ();

   double undefinedValue = -1;

   unsigned int depth = 0;

   if (mapFileCache.rank != 2)
   {
      // at least dimensions[2] is out of date.

      dataSetId = H5Dopen (fileId, dataSetName.c_str (), H5P_DEFAULT);

      if (dataSetId < 0)
      {
         return 0;
      }

      dataSpaceId = H5Dget_space (dataSetId);
      assert (dataSpaceId >= 0);

      hsize_t dimensions[3];

      mapFileCache.rank = H5Sget_simple_extent_dims (dataSpaceId, dimensions, 0);

      H5Sclose (dataSpaceId);
      H5Dclose (dataSetId);

      mapFileCache.numI = dimensions[0];
      mapFileCache.numJ = dimensions[1];
      if (mapFileCache.rank == 2)
      {
         mapFileCache.depth = 1;
      }
      else
      {
         mapFileCache.depth = dimensions[2];
      }


      undefinedValue = mapFileCache.undefinedValue = GetUndefinedValue (fileId); // value is in the file.
   }

   numI = mapFileCache.numI;
   numJ = mapFileCache.numJ;
   depth = mapFileCache.depth;
   undefinedValue = mapFileCache.undefinedValue;


   // create petsc type 
   PetscDimensions *petscD;

   if (depth == 1)
   {
      petscD = new Petsc_2D;
   }
   else
   {
      petscD = new Petsc_3D;
   }

   const Grid *grid = findGrid (numI, numJ);
   
   assert (grid);
   
   DistributedGridMap *gridMap = 0;
   
   const Grid *theActivityOutputGrid = getActivityOutputGrid ();
  
   const bool equalGrids = theActivityOutputGrid == 0 || grid == theActivityOutputGrid;

   //create
   if ( equalGrids )
   {
      gridMap = dynamic_cast<DistributedGridMap * > (getFactory ()->produceGridMap (parent, childIndex, grid, undefinedValue, depth));
   } else {
      gridMap = dynamic_cast<DistributedGridMap * > (getFactory ()->produceGridMap (0, 0, grid, undefinedValue, depth));
   }
   //read
   // determine the dataset storage type
   hid_t dataId = gridMapFile.openDataset ( dataSetName.c_str (), fileId );
   if ( dataId <  0 ) return 0;
   hid_t dtype       = H5Dget_type( dataId );
   ssize_t dataSize  = H5Tget_size(dtype);
   bool isDoubleType = ( dataSize == H5_SIZEOF_DOUBLE );
   
   H5Dclose (dataId);
   H5Tclose( dtype );
 
   if( isDoubleType ) {
      PetscVector_ReadWrite < double >reader;
      reader.read (&gridMapFile, fileId, dataSetName.c_str (), gridMap->getDA (), gridMap->getVec (), petscD);
   } else {
      PetscVector_ReadWrite < float >reader;
      reader.read (&gridMapFile, fileId, dataSetName.c_str (), gridMap->getDA (), gridMap->getVec (), petscD);
   }

   if( not equalGrids ) {
      DistributedGridMap *gridMapInActivityOutputGrid = 0;
      
      gridMapInActivityOutputGrid =
         dynamic_cast<DistributedGridMap *> (getFactory ()->produceGridMap (parent, childIndex, theActivityOutputGrid, undefinedValue, depth));
      
      //map gridMap to gridMapInOutputGrid
      //bool ret = mapGridMapAtoGridMapB( gridMap, grid, gridMapInActivityOutputGrid, theActivityOutputGrid ); 
      bool ret = gridMap->convertToGridMap (gridMapInActivityOutputGrid);
      
      delete gridMap;
      gridMap = 0;
      
      if (ret)
      {
         //if the transformation was sucessful return the map otherwise 0. it should throw here...
         gridMap = gridMapInActivityOutputGrid;
      }
         
   }
   delete petscD;

   return gridMap;
}
//------------------------------------------------------------//
bool ProjectHandle::makeOutputDir() const
{
   using namespace ibs;
   if( H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) 
   {  // in case we need a temporary location

      // Create the directory in the temporary location
      FolderPath tmpdir( H5_Parallel_PropertyList::getTempDirName() );
      tmpdir << getFullOutputDir();
      try
      {
         tmpdir.create();
      }
      catch( PathException & e)
      {
         PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error TMPDIR '%s' couldn't be created, because: %s\n", tmpdir.path().c_str(),  e.what() );
         return false;
      }
   }
 
   if( H5_Parallel_PropertyList::isPrimaryPodEnabled() ) {
     // in this case getFullOutputDir() points to the temporary dir (shared scratch) and we need to create the dir in the final location
  
      // Create the directory in the final location
      FolderPath dirpath ( getProjectPath() );
      dirpath << getOutputDir();
      try
      {
         dirpath.create();
      }
      catch( PathException & e)
      {
         PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error Directory at the final location '%s' couldn't be created, because: %s\n", dirpath.path().c_str(),  e.what() );
         return false;
      }
   }

   // Create the directory in the final location
   try
   {
      FolderPath( getFullOutputDir() ).create();
   }
   catch( PathException & e)
   {
      PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error Directory to final write location '%s' couldn't be created, because: %s\n", getFullOutputDir().c_str(),  e.what() );
      return false;
   }
   
   return true;
}
//------------------------------------------------------------//
const std::string ProjectHandle::getFullOutputDir() const
{

   using namespace ibs;
 
   if( H5_Parallel_PropertyList::isPrimaryPodEnabled()) {

      FilePath ppath( H5_Parallel_PropertyList::getTempDirName() );// +  "/" + getProjectPath() + "/" + getOutputDir()  );
      ppath <<  getProjectPath() << getOutputDir();
      return  ppath.path();
   } else {
      FilePath ppath( getProjectPath() );
      ppath << getOutputDir();
      return ppath.path();
   }
}


//------------------------------------------------------------//

void ProjectHandle::barrier () const {
   MPI_Barrier ( PETSC_COMM_WORLD );
}

void ProjectHandle::getMinValue ( double * localMin, double * globalMin ) const {

   MPI_Allreduce ( localMin, globalMin, 1, MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD );
}

void ProjectHandle::getMaxValue ( double * localMax, double * globalMax ) const {

   MPI_Allreduce ( localMax, globalMax, 1, MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD );
}

void ProjectHandle::getMinValue ( int localValue, int& globalValue ) const {
   MPI_Allreduce ( &localValue, &globalValue, 1, MPI_INT, MPI_MIN, PETSC_COMM_WORLD );
}

void ProjectHandle::getMaxValue ( int localValue, int& globalValue ) const {
   MPI_Allreduce ( &localValue, &globalValue, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD );
}


namespace ddd
{
   int GetRank (void)
   {
      static int rank = -1;
      if (rank  < 0)
      {
        int mpiRank;
        MPI_Comm_rank (PETSC_COMM_WORLD, &mpiRank);
        rank = mpiRank;
      }
      return rank;
   }


   int GetSize (void)
   {
      static int size = -1;
      if (size  < 0)
      {
        int mpiSize;
        MPI_Comm_size (PETSC_COMM_WORLD, &mpiSize);
        size = mpiSize;
      }
      return size;
   }

}
