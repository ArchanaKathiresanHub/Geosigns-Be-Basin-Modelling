#include "hdf5funcs.h"

#include "h5_parallel_file_types.h"
using namespace std;

/// Find the name of the DataSet with the given layerIndex
string HDF5::findLayerName (const string & filePathName, int layerIndex)
{
  H5_Parallel_PropertyList propertyList;
  return findLayerName(filePathName, layerIndex, &propertyList);
}
