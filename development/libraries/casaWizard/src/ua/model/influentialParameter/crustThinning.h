#pragma once

#include "model/influentialParameter.h"
namespace casaWizard
{

namespace ua
{

class CrustThinning : public InfluentialParameter
{
public:
  CrustThinning();
  void fillArguments(const ProjectReader& projectReader) override;
};

} // namespace ua

} // namespace casaWizard
