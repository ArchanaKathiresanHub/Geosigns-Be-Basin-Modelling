#include "lithofractionmap.h"

namespace casaWizard
{

LithofractionMap::LithofractionMap(const std::vector<std::vector<double>> mapData) :
  mapData_(mapData)
{

}

const std::vector<std::vector<double>>& LithofractionMap::getData() const
{
  return mapData_;
}

LithofractionMap LithofractionMap::operator+(const LithofractionMap& lithoMap) const
{
  const std::vector<std::vector<double>>& rhsData = lithoMap.getData();
  if (mapData_.size() != rhsData.size() || mapData_[0].size() != rhsData[0].size())
  {
    return LithofractionMap({});
  }

  std::vector<std::vector<double>> resultData;
  for (int i = 0; i < mapData_.size(); i++)
  {
    std::vector<double> resultRow;
    for (int j = 0; j < mapData_[0].size(); j++)
    {
      resultRow.push_back(mapData_[i][j] + rhsData[i][j]);
    }
    resultData.push_back(resultRow);
  }

  return LithofractionMap(resultData);
}

LithofractionMap LithofractionMap::operator+(const double rhs) const
{
  std::vector<std::vector<double>> resultData;
  for (int i = 0; i < mapData_.size(); i++)
  {
    std::vector<double> resultRow;
    for (int j = 0; j < mapData_[0].size(); j++)
    {
      resultRow.push_back(mapData_[i][j] + rhs);
    }
    resultData.push_back(resultRow);
  }

  return LithofractionMap(resultData);
}

LithofractionMap LithofractionMap::operator*(const double rhs) const
{
  std::vector<std::vector<double>> resultData;
  for (int i = 0; i < mapData_.size(); i++)
  {
    std::vector<double> resultRow;
    for (int j = 0; j < mapData_[0].size(); j++)
    {
      resultRow.push_back(mapData_[i][j] * rhs);
    }
    resultData.push_back(resultRow);
  }

  return LithofractionMap(resultData);
}

} // namespace casaWizard
