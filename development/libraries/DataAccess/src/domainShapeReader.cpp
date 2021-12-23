#include "domainShapeReader.h"

// hdf5 3dparty library
#include "hdf5.h"
#include "hdf5funcs.h"

#include "ConstantsNumerical.h"



#include <cmath>

namespace DataAccess
{

DomainShapeReader::DomainShapeReader(const std::string& inputHDFFileName) :
  fileName_{inputHDFFileName}
{

}

std::vector<std::vector<int>> DomainShapeReader::readShape(const int numI, const int numJ)
{
  std::vector<std::vector<int>> domainShape;

  const hid_t filedId = H5Fopen(fileName_.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  const hid_t groupId = H5Gopen2(filedId, "/", H5P_DEFAULT);

  char* datasetName = new char[1024];
  H5Lget_name_by_idx(groupId, "/", H5_INDEX_NAME, H5_ITER_NATIVE, 0, datasetName, 1024, H5P_DEFAULT);
  const std::string name(datasetName);
  delete[] datasetName;

  if (name.find("Layer=") == 0)
  {
    const hid_t datasetId = H5Dopen2(groupId, name.c_str(), H5P_DEFAULT);
    const hid_t spaceId = H5Dget_space(datasetId);

    const hssize_t bufferSize = H5Sget_simple_extent_npoints(spaceId);
    float* buffer = new float[bufferSize];
    for (int i = 0; i < bufferSize; i++)
    {
      buffer[i] = 0;
    }

    const herr_t statusRead = H5Dread(datasetId, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

    for (unsigned int i = 0; i < numI; ++i)
    {
      std::vector<int> row;

      for (unsigned int j = 0; j < numJ; ++j)
      {
        const float value = buffer[i*numJ + j];

        if (std::fabs(value - Utilities::Numerical::CauldronNoDataValue) < 1e-5)
        {
          row.push_back(0);
        }
        else
        {
          row.push_back(1);
        }
      }
      domainShape.push_back(row);
    }

    H5Dclose(datasetId);
  }

  H5Gclose(groupId);
  H5Fclose(filedId);

  return domainShape;
}

}
