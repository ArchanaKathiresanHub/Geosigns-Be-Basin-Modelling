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

#include "ProjectHandle.h"
#include "Grid.h"
#include "DistributedGrid.h"
#include "DistributedGridMap.h"
#include "ObjectFactory.h"
#include "DistributedMessageHandler.h"
#include "DistributedApplicationGlobalOperations.h"
#include "domainShapeReader.h"
#include "LogHandler.h"

#include "cauldronschemafuncs.h"
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
    bool isDoubleType;
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

void ProjectHandle::mapFileCacheCloseFiles(void)
{
    if (m_mapFileCache != nullptr) {
        for (int i = 0; i < 4; ++i)
        {
            if (static_cast<struct MapFileCache*> (m_mapFileCache)[i].fileName != "")
            {
                static_cast<struct MapFileCache*> (m_mapFileCache)[i].gridMapFile.close();
                static_cast<struct MapFileCache*> (m_mapFileCache)[i].fileName = "";
            }
        }
    }
}

void ProjectHandle::getDomainShape(const int windowMinI, const int windowMaxI, const int windowMinJ, const int windowMaxJ, std::vector<std::vector<int> >& domainShape ) const
{
  char* dynamicDecomposition = getenv("DECOMPOSITION_METHOD");
  if (dynamicDecomposition && std::string(dynamicDecomposition) == "static")
  {
    LogHandler(LogHandler::INFO_SEVERITY) << "The Decomposition method in the Configuration is set to Static Domain Decomposition.";
    return;
  }
  else if (dynamicDecomposition && std::string(dynamicDecomposition) == "dynamic")
  {
    LogHandler(LogHandler::INFO_SEVERITY) << "The Decomposition method in the Configuration is set to Dynamic Domain Decomposition, which will be used if applicable.";
  }
  else
  {
    LogHandler(LogHandler::INFO_SEVERITY) << "The Decomposition method is not set in the Configuration, therefore the default Dynamic Domain Decomposition is used if applicable.";
  }

  if (m_rank == 0)
  {
    database::Table* gridMapTbl = getTable( "GridMapIoTbl" );
    const auto record = gridMapTbl->getRecord(0);
    if (record)
    {
      DomainShapeReader reader(getMapFileName(record));
      Window window(windowMinI, windowMaxI, windowMinJ, windowMaxJ);
      reader.readShape(window, domainShape);
    }
  }
}

void ProjectHandle::checkForValidPartitioning (const string & name, int M, int N) const
{
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

   bool scalingFound = DistributedGrid::checkForValidPartitioning (M_, N_);

   if ( M <= 1 or N <= 1 ) {
      PetscPrintf (PETSC_COMM_WORLD,
                   "\n Basin_Error: Unable to partition a %d x %d grid, please select a larger number of grid nodes.\nThere must be at least two nodes in each direction. \n", M, N );

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
   else // i.e. 3D
   {
      mapFileCachePtr = & static_cast<struct MapFileCache * > (m_mapFileCache) [auxiliary];
   }

   struct MapFileCache & mapFileCache = * mapFileCachePtr;

   H5_ReadOnly_File & gridMapFile = mapFileCache.gridMapFile;

   if (mapFileCache.fileName != filePathName)
   {
      mapFileCache.rank = -1;   // needs to be invalidated
      if (mapFileCache.fileName != "")
      {
         gridMapFile.close ();
         mapFileCache.fileName = "";
      }      
            
      // It would be better to use a parallel mpio property list, but it drops performance for some cases on non-parallel file system (but works fine)
      H5_Parallel_PropertyList parPropertyList;
      H5_PropertyList propertyList;
      H5_PropertyList* propertyListPointer = &propertyList;

      int fileOpen = 0;
      int globalFileOpen = 0;

      if (!gridMapFile.open (filePathName.c_str (), propertyListPointer))
      {
          fileOpen = 1;
      }

      MPI_Allreduce(&fileOpen, &globalFileOpen, 1, MPI_INT, MPI_SUM, PETSC_COMM_WORLD);

      if (globalFileOpen > 0)
      {
          gridMapFile.close();
          propertyListPointer = &parPropertyList;

          if (!gridMapFile.open(filePathName.c_str(), propertyListPointer))
          {
              cerr << "ERROR in ProjectHandle::loadGridMap (): Could not open " << filePathName << endl;
              return nullptr;
          }
      }

      mapFileCache.fileName = filePathName;
      mapFileCache.undefinedValue = gridMapFile.GetUndefinedValue(); // value is in the file.
   }   

   if (mapFileCache.rank != 2)
   {
      // at least dimensions[2] is out of date.

      hid_t dataSetId = gridMapFile.openDataset(dataSetName.c_str());

      if (dataSetId < 0)
      {
         return nullptr;
      }

      hid_t dataSpaceId = H5Dget_space (dataSetId);
      assert (dataSpaceId >= 0);

      hsize_t dimensions[3];

      mapFileCache.rank = H5Sget_simple_extent_dims (dataSpaceId, dimensions, 0);      

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

      // determine the dataset storage type
      hid_t dtype       = H5Dget_type( dataSetId );
      ssize_t dataSize  = H5Tget_size(dtype);
      mapFileCache.isDoubleType = ( dataSize == H5_SIZEOF_DOUBLE );

      H5Sclose (dataSpaceId);
      gridMapFile.closeDataset(dataSetId);
   }

   const int numI = mapFileCache.numI;
   const int numJ = mapFileCache.numJ;
   unsigned int depth = mapFileCache.depth;
   const double undefinedValue = mapFileCache.undefinedValue;
   const bool isDoubleType = mapFileCache.isDoubleType;

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

   if( isDoubleType ) {
      PetscVector_ReadWrite < double >reader;
      reader.read (&gridMapFile, dataSetName.c_str (), gridMap->getDA (), gridMap->getVec (), petscD);
   } else {
      PetscVector_ReadWrite < float >reader;
      reader.read (&gridMapFile, dataSetName.c_str (), gridMap->getDA (), gridMap->getVec (), petscD);
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
         PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error: Directory at the final location '%s' couldn't be created, because: %s\n", dirpath.path().c_str(),  e.what() );
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
      PetscPrintf ( PETSC_COMM_WORLD, "  Basin_Error: Directory to final write location '%s' couldn't be created, because: %s\n", getFullOutputDir().c_str(),  e.what() );
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
