#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class TopCrustHeatProductionGrid : public InfluentialParameter
{
public:
  TopCrustHeatProductionGrid();
  void fillArguments(const ProjectReader& projectReader) override;
};

} // namespace ua

} // namespace casaWizard
