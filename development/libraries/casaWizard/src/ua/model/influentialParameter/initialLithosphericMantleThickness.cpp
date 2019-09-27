#include "initialLithosphericMantleThickness.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

InitialLithosphericMantleThickness::InitialLithosphericMantleThickness() :
  InfluentialParameter
  (
    "Basement: Initial mantle thickness",
    "\"BasementIoTbl:InitialLithosphericMantleThickness\"",
    "Initial Mantle Thickness",
    "[m]",
    1
  )
{
  arguments().addArgument("Minimum range value", 0.0);
  arguments().addArgument("Maximum range value", 0.0);
  addPDFArgument();
}

void InitialLithosphericMantleThickness::fillArguments(const ProjectReader& projectReader)
{
  const double mantleThickness = projectReader.initialLithosphericMantleThickness();
  arguments().setDoubleArguments({0.9*mantleThickness, 1.1*mantleThickness});
}

} // namespace ua

}  // namespace casaWizard
