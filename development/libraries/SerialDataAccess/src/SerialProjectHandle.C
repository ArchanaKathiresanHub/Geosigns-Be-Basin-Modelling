#include <stdafx.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <cmath>

#if defined(_WIN32) || defined (_WIN64)
#include <direct.h>
#endif

#include <algorithm>

#include <errno.h>
#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include <vector>
#include <list>

using namespace std;

#include "Interface/ProjectHandle.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/ObjectFactory.h"
#include "Interface/SerialMessageHandler.h"
#include "Interface/SerialApplicationGlobalOperations.h"

#include "array.h"

using namespace DataAccess;
using namespace Interface;

//float GetUndefinedValue (hid_t fileId);

const double DefaultUndefinedValue = 99999;

void ProjectHandle::mapFileCacheConstructor (void){ }

void ProjectHandle::mapFileCacheDestructor  (void){ }

void ProjectHandle::checkForValidPartitioning (int M, int N) const {}

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

   if ((fileId = H5Fopen (filePathName.c_str (), H5F_ACC_RDONLY, H5P_DEFAULT)) >= 0)
   {

# if H5_VERS_MINOR != 6
      if ((dataSetId = H5Dopen (fileId, dataSetName.c_str (), H5P_DEFAULT)) >= 0)
#else 
      if ((dataSetId = H5Dopen (fileId, dataSetName.c_str ())) >= 0)
#endif      
      {
	 dataTypeId = H5Tcopy (H5T_NATIVE_FLOAT);
	 H5T_class_t HDFclass = H5Tget_class (dataTypeId);

	 assert (HDFclass == H5T_FLOAT);

	 if ((dataSpaceId = H5Dget_space (dataSetId)) >= 0)
	 {
	    hsize_t dimensions[3];
	    int rank = H5Sget_simple_extent_dims (dataSpaceId, dimensions, 0);

	    if (rank == 2) dimensions[2] = 1;

	    double undefinedValue;
	    if (rank == 3)
	    {
	       undefinedValue = DefaultUndefinedValue; // not in the file!!!
	    }
	    else
	    {
	       undefinedValue = GetUndefinedValue (fileId);
	    }

	    const Grid *grid = findGrid (dimensions[0], dimensions[1]);

	    if (grid)
	    {

	       float ***array = Array < float >::create3d (dimensions[0], dimensions[1], dimensions[2]);

	       // This only works because, due to the implementation of Array<>::create3d (),
	       // &array[0][0][0] points to the appropriate 1D array.
	       herr_t status = H5Dread (dataSetId, dataTypeId, H5S_ALL, H5S_ALL, H5P_DEFAULT, &array[0][0][0]);

	       if (status >= 0)
	       {
		  const Grid *theActivityOutputGrid = getActivityOutputGrid ();

		  if (theActivityOutputGrid == 0 || grid == theActivityOutputGrid)
		  {
		     gridMap = getFactory ()->produceGridMap (parent, childIndex, grid, undefinedValue, (unsigned int) dimensions[2], array);
		  }
		  else
		  {
		     gridMap = getFactory ()->produceGridMap (0, -1, grid, undefinedValue, (unsigned int) dimensions[2], array);

		     GridMap * gridMapInActivityOutputGrid =
			getFactory ()->produceGridMap (parent, childIndex, theActivityOutputGrid, undefinedValue, (unsigned int) dimensions[2]);

		     bool ret = gridMap->convertToGridMap (gridMapInActivityOutputGrid);

		     delete gridMap;
		     gridMap = 0;

		     if (ret)
		     {
			//if the transformation was sucessful return the map otherwise 0. it should throw here...
			gridMap = gridMapInActivityOutputGrid;
		     }
		  }
	       }
	       Array < float >::delete3d (array);

	       H5Sclose (dataSpaceId);
	    }
	    else
	    {
	       cerr << "ERROR: Could not find grid with dimensions " << dimensions[0]
		  << " x " << dimensions[1] << endl;
	    }
	 }
	 H5Tclose (dataTypeId);
	 H5Dclose (dataSetId);
      }
      else
      {
	 cerr << "ERROR: Could not open dataset " << dataSetName
	    << " in file " << filePathName << endl;
      }
      H5Fclose (fileId);
   }
   else
   {
      cerr << "ERROR: Could not open " << filePathName << endl;
   }
   return gridMap;
}

void ProjectHandle::barrier () const {
   // Do nothing.
}

void ProjectHandle::getMinValue ( double * localMin, double * globalMin ) const {
   // Do nothing.
}

void ProjectHandle::getMaxValue ( double * localMax, double * globalMax ) const {

   // Do nothing.

}

#include<time.h>

namespace ddd
{
   int NumProcessors (void)
   {
      return 1;
   }

   int GetRank (void)
   {
      return 0;
   }

   int GetSize (void)
   {
      return 1;
   }

   string & GetRankString (void)
   {
      static string fullRankString;

#if defined(_WIN32) || defined (_WIN64)
	  fullRankString = "no time";
#else

      timespec tp;
      clock_gettime(CLOCK_REALTIME, &tp);

      char timestr[32];
      sprintf (timestr, "%9d.%9ld\t", tp.tv_sec, tp.tv_nsec);

      fullRankString = "";
      fullRankString += timestr;      
#endif

	  return fullRankString;
   }
}
