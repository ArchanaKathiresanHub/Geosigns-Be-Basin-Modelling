#include "domainShapeReader.h"

#include "ConstantsNumerical.h"
#include "LogHandler.h"

// hdf5 3dparty library
#include "hdf5.h"
#include "hdf5funcs.h"

#include <algorithm>
#include <cmath>
#include <memory>


namespace DataAccess
{

DomainShapeReader::DomainShapeReader(const std::string& inputHDFFileName) :
  m_fileName{inputHDFFileName}
{

}

void DomainShapeReader::readShape(const Window& window, std::vector<std::vector<int>>& domainShape) const
{
   H5Eset_auto( H5E_DEFAULT, 0, 0);

   const hid_t fileId = H5Fopen(m_fileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
   const hid_t groupId = H5Gopen2(fileId, "/", H5P_DEFAULT);

   if (fileId < 0 || groupId < 0)
   {
      H5Gclose(groupId);
      H5Fclose(fileId);
   }

   const std::string name = getDataSetName(groupId);

   if (name.find("Layer=") == 0)
   {
      extractData(window, groupId, name, domainShape);
   }

   H5Gclose(groupId);
   H5Fclose(fileId);
}

std::string DomainShapeReader::getDataSetName(const hid_t groupId) const
{
   std::vector<char> modifiable(1023);
   modifiable.push_back('\0');//make sure that the end of string is the NULL char
   H5Lget_name_by_idx(groupId, "/", H5_INDEX_NAME, H5_ITER_NATIVE, 0, &modifiable[0], 1024, H5P_DEFAULT);

   return std::string(modifiable.begin(),modifiable.end());
}

void DomainShapeReader::extractData(const Window& window, const hid_t groupId, const std::string& datasetName, std::vector<std::vector<int>>& domainShape) const
{
   const hid_t datasetId = H5Dopen2(groupId, datasetName.c_str(), H5P_DEFAULT);

   const hid_t spaceId = H5Dget_space(datasetId);
   const hssize_t bufferSize = H5Sget_simple_extent_npoints(spaceId);
   hsize_t ndims = H5Sget_simple_extent_ndims(spaceId);
   hsize_t dims[ndims];
   H5Sget_simple_extent_dims(spaceId, dims, nullptr);
   const hsize_t numI = dims[0];
   const hsize_t numJ = dims[1];

   if (window.minI < 0 || window.minJ < 0 || window.maxI >= numI || window.maxJ >= numJ)
   {
      LogHandler(LogHandler::DEBUG_SEVERITY) << "Could not read domain shape for dynamic decomposition";
      return;
   }

   std::unique_ptr<float[]> myBuffer(new float[bufferSize]());
   H5Dread(datasetId, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &myBuffer[0]);

   for (unsigned int i = window.minI; i <= window.maxI; ++i)
   {
      std::vector<int> row;

      for (unsigned int j = window.minJ; j <= window.maxJ; ++j)
      {
         row.push_back(
                  (std::fabs(myBuffer[i * numJ + j] - Utilities::Numerical::CauldronNoDataValue) > Utilities::Numerical::DefaultNumericalTolerance)
               );
      }
      domainShape.push_back(row);
   }

   H5Dclose(datasetId);
}

}
