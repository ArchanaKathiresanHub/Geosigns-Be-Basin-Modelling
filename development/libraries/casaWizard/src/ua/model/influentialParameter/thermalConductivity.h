#pragma once

#include "model/influentialParameter.h"

namespace casaWizard
{

namespace ua
{

class ThermalConductivity : public InfluentialParameter
{
public:
  ThermalConductivity();
  void fillArguments(const ProjectReader& projectReader) override;
  QString nameArgumentBased() const override;
};

} // namespace ua

}  // namespace casaWizard
