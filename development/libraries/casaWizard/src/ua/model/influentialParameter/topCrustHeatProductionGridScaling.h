#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class TopCrustHeatProductionGridScaling : public InfluentialParameter
{
public:
  TopCrustHeatProductionGridScaling();
  void fillArguments(const ProjectReader& projectReader) override;
};

} // namespace ua

} // namespace casaWizard
