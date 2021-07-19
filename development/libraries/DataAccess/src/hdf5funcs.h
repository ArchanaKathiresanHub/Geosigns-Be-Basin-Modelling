#include "hdf5.h"

#include <iostream>
#include <string>

class H5_PropertyList;

namespace HDF5
{
   struct LayerInfo
   {
      std::string name;
      int index;
   };

   bool writeData1D (hid_t fileHandle, long size, const char *datasetName, hid_t dataType, const void *data);
   bool writeData2D (hid_t fileHandle, long sizeI, long sizeJ, const char *datasetName, hid_t dataType, const void *data, bool & newDataset );
   bool writeAttribute (hid_t fileHandle, const char *datasetName, const char *attributeName, const hid_t attributeType, const hsize_t dims, void *data);

   /// Function used to iterate through a HDF5 file to find the name of the DataSet with the given layerIndex
   herr_t checkForLayerName (hid_t groupId, const char * layerName, LayerInfo * layerInfo);
   /// Find the name of the DataSet with the given layerIndex
   std::string findLayerName(const std::string & filePathName, int layerIndex, H5_PropertyList* plist);
   std::string findLayerName(const std::string & filePathName, int layerIndex);

}
