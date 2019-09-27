// Influential parameter for the surface temperature history, which can be set at a specific age
#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class SurfaceTemperature : public InfluentialParameter
{
public:
  SurfaceTemperature();
  void fillArguments(const casaWizard::ProjectReader& projectReader) override;
  QString nameArgumentBased() const override;
};

}  // namespace ua

}  // namespace casaWizard
