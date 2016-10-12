#include <iostream>
using namespace std;

#include <assert.h>

#include "hdf5funcs.h"

#include "h5_file_types.h"


/// Find the name of the DataSet with the given layerIndex
const string & HDF5::findLayerName (const string & filePathName, int layerIndex)
{
   static LayerInfo layerInfo;
   layerInfo.index = layerIndex;
   layerInfo.name = "";

   hid_t fileId;
   if ((fileId = H5Fopen (filePathName.c_str (), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
   {
      cerr << "ERROR in HDF5::findLayerName (): Could not open " << filePathName << endl;
   }
   else
   {
      int ret = H5Giterate (fileId, "/", NULL, (H5G_iterate_t) checkForLayerName, &layerInfo);

      H5Fclose (fileId);
   }


   return layerInfo.name;
}

