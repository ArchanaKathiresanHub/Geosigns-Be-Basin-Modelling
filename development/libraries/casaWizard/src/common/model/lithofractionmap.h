#pragma once

#include <vector>

namespace casaWizard
{

class LithofractionMap
{
public:
  explicit LithofractionMap(const std::vector<std::vector<double>> mapData);
  const std::vector<std::vector<double>>& getData() const;
  LithofractionMap operator+(const LithofractionMap& lithoMap) const;
  LithofractionMap operator+(const double rhs) const;
  LithofractionMap operator*(const double rhs) const;

private:
  std::vector<std::vector<double>> mapData_;
};

} // namespace casaWizard
