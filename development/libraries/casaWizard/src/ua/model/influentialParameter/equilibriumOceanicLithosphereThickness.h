// Influential parameter for the equilibrium oceanic lithosphere thickness in the basement IO table
#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class EquilibriumOceanicLithosphereThickness : public InfluentialParameter
{
public:
  EquilibriumOceanicLithosphereThickness();
  void fillArguments(const casaWizard::ProjectReader& projectReader) override;
};

}  // namespace ua

}  // namespace casaWizard
