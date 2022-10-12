#include "hdf5funcs.h"

#include "h5_file_types.h"

/// Find the name of the DataSet with the given layerIndex
std::string HDF5::findLayerName (const std::string& filePathName, int layerIndex)
{
  H5_PropertyList propertyList;
  return findLayerName(filePathName, layerIndex, &propertyList);
}
