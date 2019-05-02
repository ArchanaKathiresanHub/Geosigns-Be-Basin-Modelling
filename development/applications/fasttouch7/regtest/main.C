// Program to be used in the NVGShilohVXSNew_2facies regression test for fasttouch7
// 2 HDF files are compared, but only the part for which the map given in the facies.HDF file is 1

#include "hdf5.h"
#include "string.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

herr_t file_info1(hid_t loc_id, const char* name, const H5L_info_t* info, void* opdata);
herr_t file_info2(hid_t loc_id, const char* name, const H5L_info_t* info, void* opdata);

std::vector<std::string> dataSets1, dataSets2;

int main()
{
  float facies[112][131];
  hid_t fileFacies = H5Fopen("../../Input/facies.HDF", H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t datasetFacies = H5Dopen2(fileFacies, "Layer=0", H5P_DEFAULT );
  H5Dread(datasetFacies, H5T_IEEE_F32BE, H5S_ALL, H5S_ALL, H5P_DEFAULT, facies);
  H5Dclose(datasetFacies);
  H5Fclose(fileFacies);

  hid_t fileSingle = H5Fopen("./Valid/FastTouch_Results_Single.HDF", H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t fileDouble = H5Fopen("./Test/NVGShilohVXSNew_2facies_CauldronOutputDir/FastTouch_Results.HDF", H5F_ACC_RDONLY, H5P_DEFAULT);

  H5Literate( fileSingle, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, file_info1, NULL);
  H5Literate( fileDouble, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, file_info2, NULL);

  assert(dataSets1.size() == dataSets2.size());

  for (int d = 0; d < dataSets1.size(); ++d)
  {
    float data1[19][22];
    hid_t datasetSingle = H5Dopen2(fileSingle, dataSets1[d].c_str(), H5P_DEFAULT );
    H5Dread(datasetSingle, H5T_IEEE_F32LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data1);
    H5Dclose(datasetSingle);

    float data2[19][22];
    hid_t datasetDouble = H5Dopen2(fileDouble, dataSets2[d].c_str(), H5P_DEFAULT );
    H5Dread(datasetDouble, H5T_IEEE_F32LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data2);
    H5Dclose(datasetDouble);

    for (int i = 0; i < 19; ++i)
    {
      for (int j = 0 ; j < 22; ++j)
      {
        // if we are in the first area but we find a difference we have an error
        // use 6 for subsampling the facies data
        if ((facies[i*6][j*6] == 1) && std::fabs(data1[i][j] - data2[i][j]) > std::numeric_limits<float>::epsilon() )
        {
          std::cerr << "Single and double facies values are not equal within the first facies for " << dataSets1[d]
                       << " in i " <<  i << " j " << j << std::endl;
          return 1;
        }
      }
    }
  }

  H5Fclose(fileDouble);
  H5Fclose(fileSingle);

  return 0;
}

herr_t file_info1(hid_t /*loc_id*/, const char* name, const H5L_info_t* /*info*/, void* /*opdata*/)
{
  if (strncmp(name, "Layer=", 6) == 0)
  {
    dataSets1.push_back(std::string(name));
  }
  return 0;
}

herr_t file_info2(hid_t /*loc_id*/, const char* name, const H5L_info_t* /*info*/, void* /*opdata*/)
{
  if (strncmp(name, "Layer=", 6) == 0)
  {
    dataSets2.push_back(std::string(name));
  }
  return 0;
}
