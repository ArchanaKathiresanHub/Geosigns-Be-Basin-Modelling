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
  bool isUndefined(const double input) const;
  double multiplication(const double input1, const double input2) const;
  double sum(const double input1, const double input2) const;

  std::vector<std::vector<double>> mapData_;
};

} // namespace casaWizard
