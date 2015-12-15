#include "hdf5funcs.h"

#include <iostream>
using namespace std;

#include <assert.h>


#include "h5_parallel_file_types.h"
#include "petscvector_readwrite.h"


/// Find the name of the DataSet with the given layerIndex
const string & HDF5::findLayerName (const string & filePathName, int layerIndex)
{
   static LayerInfo layerInfo;

   layerInfo.index = layerIndex;
   layerInfo.name = "";

   H5_Parallel_PropertyList propertyList;

   H5_ReadOnly_File gridMapFile;
   if (!gridMapFile.open (filePathName.c_str (), &propertyList))
   {
      cerr << "ERROR in HDF5::findLayerName (): Could not open " << filePathName << endl;
   }
   else
   {
      hid_t fileId = gridMapFile.fileId ();

      int ret = H5Giterate (fileId, "/", NULL, (H5G_iterate_t) checkForLayerName, &layerInfo);

      H5Fclose (fileId);
      gridMapFile.close ();
   }

   return layerInfo.name;
}

