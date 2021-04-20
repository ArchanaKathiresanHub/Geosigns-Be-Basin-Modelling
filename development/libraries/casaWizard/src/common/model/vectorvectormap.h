#pragma once

#include <vector>

namespace casaWizard
{

class VectorVectorMap
{
public:
  explicit VectorVectorMap(const std::vector<std::vector<double>> mapData);
  const std::vector<std::vector<double>>& getData() const;
  VectorVectorMap operator+(const VectorVectorMap& lithoMap) const;
  VectorVectorMap operator+(const double rhs) const;
  VectorVectorMap operator*(const double rhs) const;

private:
  std::vector<std::vector<double>> mapData_;
};

} // namespace casaWizard
