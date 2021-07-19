#include <sys/types.h>
#include <sys/stat.h>


#if defined(_WIN32) || defined (_WIN64)
#include <direct.h>
#endif

#include <algorithm>
#include <cassert>
#include <iostream>
#include <cerrno>

#include "ProjectHandle.h"
#include "Grid.h"
#include "GridMap.h"
#include "ObjectFactory.h"
#include "SerialMessageHandler.h"
#include "SerialApplicationGlobalOperations.h"
#include "FilePath.h"

#include "h5_file_types.h"

#include "array.h"

using namespace DataAccess;
using namespace Interface;
using namespace std;
using namespace ibs;

const double DefaultUndefinedValue = 99999;

void ProjectHandle::mapFileCacheConstructor (void){ }

void ProjectHandle::mapFileCacheDestructor  (void){ }

void ProjectHandle::checkForValidPartitioning (const string & , int , int ) const {}

void ProjectHandle::allocateArchitectureRelatedParameters () {
   m_messageHandler = new SerialMessageHandler;
   m_globalOperations = new SerialApplicationGlobalOperations;
}

GridMap * ProjectHandle::loadGridMap (const Parent * parent, unsigned int childIndex,
      const string & filePathName, const string & dataSetName)
{
   GridMap *gridMap = 0;
   hid_t fileId = -1;
   hid_t dataSetId = -1;
   hid_t dataTypeId = -1;
   hid_t dataSpaceId = -1;

   H5_ReadOnly_File gridMapFile;
   H5_PropertyList pList;

   if ((fileId = gridMapFile.open(filePathName.c_str(), &pList)) >= 0)
   {
       if ((dataSetId = gridMapFile.openDataset( dataSetName.c_str())) >= 0)
       {
           dataTypeId = H5Tcopy(H5T_NATIVE_FLOAT);
           H5T_class_t HDFclass = H5Tget_class(dataTypeId);

           assert(HDFclass == H5T_FLOAT);

           if ((dataSpaceId = H5Dget_space(dataSetId)) >= 0)
           {
               hsize_t dimensions[3];
               int rank = H5Sget_simple_extent_dims(dataSpaceId, dimensions, 0);

               if (rank == 2) dimensions[2] = 1;

               double undefinedValue;
               if (rank == 3)
               {
                   undefinedValue = DefaultUndefinedValue; // not in the file!!!
               }
               else
               {
                   undefinedValue = gridMapFile.GetUndefinedValue();
               }

               const Grid *grid = findGrid(dimensions[0], dimensions[1]);

               if (grid)
               {

                   float ***array = Array < float >::create3d(dimensions[0], dimensions[1], dimensions[2]);

                   // This only works because, due to the implementation of Array<>::create3d (),
                   // &array[0][0][0] points to the appropriate 1D array.
                   herr_t status = H5Dread(dataSetId, dataTypeId, H5S_ALL, H5S_ALL, H5P_DEFAULT, &array[0][0][0]);

                   if (status >= 0)
                   {
                       const Grid *theActivityOutputGrid = getActivityOutputGrid();

                       if (theActivityOutputGrid == 0 || grid == theActivityOutputGrid)
                       {
                           gridMap = getFactory()->produceGridMap(parent, childIndex, grid, undefinedValue, (unsigned int)dimensions[2], array);
                       }
                       else
                       {
                           gridMap = getFactory()->produceGridMap(0, 0, grid, undefinedValue, (unsigned int)dimensions[2], array);

                           GridMap * gridMapInActivityOutputGrid =
                               getFactory()->produceGridMap(parent, childIndex, theActivityOutputGrid, undefinedValue, (unsigned int)dimensions[2]);

                           bool ret = gridMap->convertToGridMap(gridMapInActivityOutputGrid);

                           delete gridMap;
                           gridMap = 0;

                           if (ret)
                           {
                               //if the transformation was sucessful return the map otherwise 0. it should throw here...
                               gridMap = gridMapInActivityOutputGrid;
                           }
                       }
                   }
                   Array < float >::delete3d(array);

                   H5Sclose(dataSpaceId);
               }
               else
               {
                   cerr << "ERROR: Could not find grid with dimensions " << dimensions[0]
                       << " x " << dimensions[1] << endl;
               }
           }
           H5Tclose(dataTypeId);
           gridMapFile.closeDataset(dataSetId);
       }
       else
       {
           cerr << "ERROR: Could not open dataset " << dataSetName
               << " in file " << filePathName << endl;
       }
       gridMapFile.close();
   }
   else
   {
      cerr << "ERROR: Could not open " << filePathName << endl;
   }
   return gridMap;
}

const std::string ProjectHandle::getFullOutputDir() const
{
   ibs::FilePath ppath( getProjectPath() );
   ppath << getOutputDir();
   return ppath.path();
}

bool ProjectHandle::makeOutputDir() const
{
   // Need to create output directory if it does not exist.
#if defined(_WIN32) || defined (_WIN64)
   int status = mkdir ( getFullOutputDir().c_str () );
#else
   int status = mkdir ( getFullOutputDir().c_str (), S_IRWXU | S_IRGRP | S_IXGRP );
#endif
   if ( status != 0 and errno == ENOTDIR ) {
      return false;
   }
   return true;
}

void ProjectHandle::barrier () const {
   // Do nothing.
}

void ProjectHandle::getMinValue ( double * localMin, double * globalMin ) const {

   if( localMin != 0 and globalMin != 0 ) {
      * globalMin = * localMin;
   }
}

void ProjectHandle::getMaxValue ( double * localMax, double * globalMax ) const {

   if( localMax != 0 and globalMax != 0 ) {
      * globalMax = * localMax;
   }
}


void ProjectHandle::getMinValue ( int localValue, int& globalValue ) const {
   globalValue = localValue;
}


void ProjectHandle::getMaxValue ( int localValue, int& globalValue ) const {
   globalValue = localValue;
}


namespace ddd
{
   int GetRank (void)
   {
      return 0;
   }

   int GetSize (void)
   {
      return 1;
   }
}
