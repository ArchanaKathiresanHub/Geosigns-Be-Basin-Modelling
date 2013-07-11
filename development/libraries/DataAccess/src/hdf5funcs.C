#include <iostream>
using namespace std;

#include <string.h>

#ifdef linux
extern int IBS_Use_ADIOI_Locking;
extern bool IBS_SerializeIO;
#endif

#include <assert.h>

#include "hdf5funcs.h"


bool HDF5::writeData1D (hid_t fileHandle, long size, const char * datasetName, hid_t dataType, const void * data)
{
   if (size <= 0)
      return false;

   hsize_t dims[1];

   dims[0] = size;
   int rank = 1;
   hid_t dataspace = H5Screate_simple (rank, dims, 0);

   herr_t status = 0;
   // Create a dataset
   hid_t dataset = H5Dcreate (fileHandle,
                              datasetName,
                              dataType,
                              dataspace,
# if H5_VERS_MINOR != 6
								H5P_DEFAULT,
								H5P_DEFAULT,
#endif
                              H5P_DEFAULT);


   if (dataset != -1)
   {
#ifdef linux
      if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = false;
#endif
      status = H5Dwrite (dataset, dataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
#ifdef linux
      if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = true;
#endif
   }

   // close the dataset/dataspace we just created
   if (dataset != 0)
   {
      H5Dclose (dataset);
   }

   if (dataspace != 0)
   {
      H5Sclose (dataspace);
   }

   return (status >= 0);
}

bool HDF5::writeData2D (hid_t fileHandle, long sizeI, long sizeJ, const char * datasetName, hid_t dataType, const void * data)
{
   if (sizeI <= 0 || sizeJ <= 0)
      return false;

   hsize_t dims[2];

   dims[0] = sizeI;
   dims[1] = sizeJ;

   int rank = 2;

   hid_t dataspace = H5Screate_simple (rank, dims, 0);
   
   herr_t status = 0;

   // Create a dataset
   hid_t dataset = H5Dcreate (fileHandle,
                              datasetName,
                              dataType,
                              dataspace,
# if H5_VERS_MINOR != 6
								H5P_DEFAULT,
								H5P_DEFAULT,
#endif
                              H5P_DEFAULT);


   if (dataset != -1)
   {
#ifdef linux
      if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = false;
#endif
      status = H5Dwrite (dataset, dataType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
#ifdef linux
      if (IBS_SerializeIO) IBS_Use_ADIOI_Locking = true;
#endif
   }

   // close the dataset/dataspace we just created
   if (dataset != 0)
   {
      H5Dclose (dataset);
   }

   if (dataspace != 0)
   {
      H5Sclose (dataspace);
   }
   return (status >= 0);
}

bool HDF5::writeAttribute(hid_t fileHandle,
      const char* datasetName, const char* attributeName, const hid_t attributeType, 
      const hsize_t dims, void* data)
{
   // get the dataset from the file
   hid_t datasetId = H5Dopen (fileHandle, datasetName
# if H5_VERS_MINOR != 6
								, H5P_DEFAULT
#endif
	   );

   if (datasetId < 0)
   {
      return 0;
   }

   // Create the data space for the attribute.
   hid_t dataspaceId = H5Screate_simple (1, &dims, 0);

   if (dataspaceId < 0)
   {
      H5Dclose (datasetId);
      return 0;
   }

   // currently has no significance
   hid_t attributeId = H5Acreate (datasetId,
                                   attributeName,
                                   attributeType,
                                   dataspaceId,
# if H5_VERS_MINOR != 6
								 H5P_DEFAULT,
#endif
                                   H5P_DEFAULT);

   if (attributeId < 0)
   {
      H5Dclose (datasetId);
      return 0;
   }

   // Write the attribute data.
   hid_t writeStatus = H5Awrite (attributeId, attributeType, data);

   // close the attribute
   hid_t status = H5Aclose (attributeId);

   // close the dataspace
   status = H5Sclose (dataspaceId);
   // close the dataset
   status = H5Dclose (datasetId);

   return (writeStatus <= 0);
   //return 0;
}

int HDF5::readData (hid_t fileHandle, const char * datasetName, hsize_t * dimensions, void * data)
{
   // get the dataset from the file
   hid_t dataset = H5Dopen (fileHandle, datasetName
# if H5_VERS_MINOR != 6
								, H5P_DEFAULT
#endif
	   );

   if (dataset < 0)
   {
      return 0;
   }

   hid_t datatype = H5Dget_type (dataset); // datatype identifier

   if (datatype < 0)
   {
      return 0;
   }

   hid_t dataspace = H5Dget_space (dataset); // dataspace identifier

   if (dataspace < 0)
   {
      cerr << "Failed to get Dataspace for Dataset " << datasetName << endl;
      return 0;
   }

   int rank = H5Sget_simple_extent_ndims (dataspace);
   if (rank < 0) return 0;

   int status_n = H5Sget_simple_extent_dims (dataspace, dimensions, 0);
   if (status_n < 0) return 0;

   // read the data
   herr_t status = H5Dread (dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

   // clean up the dataset and dataspace
   if (dataset != 0)
   {
      H5Dclose (dataset);
   }
   if (dataspace != 0)
   {
      H5Sclose (dataspace);
   }

   return rank;
//return 0;
}

/// Function used to iterate through a HDF5 file to find the name of the DataSet with the given layerIndex
herr_t HDF5::checkForLayerName (hid_t groupId, const char * layerName, HDF5::LayerInfo * layerInfo)
{
   if (strncmp (layerName, "Layer=", 6) == 0)
   {
      int layerIndex;
      sscanf (layerName, "Layer=%d", &layerIndex);
      if (layerIndex == layerInfo->index)
      {
	 layerInfo->name = layerName;
	 return 1;
      }
   }
   return 0;
}
