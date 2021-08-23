#include "vectorvectormap.h"

#include "ConstantsNumerical.h"

#include <cmath>

namespace casaWizard
{

VectorVectorMap::VectorVectorMap(const std::vector<std::vector<double>> mapData) :
  mapData_(mapData)
{

}

const std::vector<std::vector<double>>& VectorVectorMap::getData() const
{
  return mapData_;
}

VectorVectorMap VectorVectorMap::operator+(const VectorVectorMap& lithoMap) const
{
  const std::vector<std::vector<double>>& rhsData = lithoMap.getData();
  if (mapData_.size() != rhsData.size() || mapData_[0].size() != rhsData[0].size())
  {
    return VectorVectorMap({});
  }

  std::vector<std::vector<double>> resultData;
  for (int i = 0; i < mapData_.size(); i++)
  {
    std::vector<double> resultRow;
    for (int j = 0; j < mapData_[0].size(); j++)
    {
      resultRow.push_back(sum(mapData_[i][j], rhsData[i][j]));
    }
    resultData.push_back(resultRow);
  }

  return VectorVectorMap(resultData);
}

VectorVectorMap VectorVectorMap::operator+(const double rhs) const
{
  std::vector<std::vector<double>> resultData;
  for (int i = 0; i < mapData_.size(); i++)
  {
    std::vector<double> resultRow;
    for (int j = 0; j < mapData_[0].size(); j++)
    {
      resultRow.push_back(sum(mapData_[i][j], rhs));
    }
    resultData.push_back(resultRow);
  }

  return VectorVectorMap(resultData);
}

VectorVectorMap VectorVectorMap::operator*(const double rhs) const
{
  std::vector<std::vector<double>> resultData;
  for (int i = 0; i < mapData_.size(); i++)
  {
    std::vector<double> resultRow;
    for (int j = 0; j < mapData_[0].size(); j++)
    {
      resultRow.push_back(multiplication(mapData_[i][j], rhs));
    }
    resultData.push_back(resultRow);
  }

  return VectorVectorMap(resultData);
}

double VectorVectorMap::sum(const double input1, const double input2) const
{
  if (isUndefined(input1) || isUndefined(input2))
  {
    return Utilities::Numerical::CauldronNoDataValue;
  }
  return input1 + input2;
}

double VectorVectorMap::multiplication(const double input1, const double input2) const
{
  if (isUndefined(input1) || isUndefined(input2))
  {
    return Utilities::Numerical::CauldronNoDataValue;
  }
  return input1 * input2;
}

bool VectorVectorMap::isUndefined(const double input) const
{
  return (std::fabs(input - Utilities::Numerical::CauldronNoDataValue) < 1e-5);
}

} // namespace casaWizard
