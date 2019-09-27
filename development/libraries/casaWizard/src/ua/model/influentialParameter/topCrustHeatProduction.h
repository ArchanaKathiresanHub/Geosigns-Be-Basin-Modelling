#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class TopCrustHeatProduction : public InfluentialParameter
{
public:
  TopCrustHeatProduction();
  void fillArguments(const casaWizard::ProjectReader& projectReader) override;
};

} // namespace ua

} // namespace casaWizard
