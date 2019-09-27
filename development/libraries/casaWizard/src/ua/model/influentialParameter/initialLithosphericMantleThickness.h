// Influential parameter for the initial lithospheric mantle thickness in the basement IO table
#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class InitialLithosphericMantleThickness : public InfluentialParameter
{
public:
  InitialLithosphericMantleThickness();
  void fillArguments(const casaWizard::ProjectReader& projectReader) override;
};

}  // namespace ua

}  // namespace casaWizard
