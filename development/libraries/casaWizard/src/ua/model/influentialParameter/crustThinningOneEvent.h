#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class CrustThinningOneEvent : public InfluentialParameter
{
public:
  CrustThinningOneEvent();
  void fillArguments(const ProjectReader& projectReader) override;
};


} // namespace ua

}  // namespace casaWizard
