#include "equilibriumOceanicLithosphereThickness.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

EquilibriumOceanicLithosphereThickness::EquilibriumOceanicLithosphereThickness() :
  InfluentialParameter
  (
    "Oceanic mantle thickness",
    "\"BasementIoTbl:EquilibriumOceanicLithosphereThickness\"",
    "Equilibrium Oceanic Lithosphere Thickness",
    "[m]",
    1
  )
{
  arguments().addArgument("Minimum range value", 0.0);
  arguments().addArgument("Maximum range value", 0.0);
  addPDFArgument();
}

void EquilibriumOceanicLithosphereThickness::fillArguments(const ProjectReader& projectReader)
{
  const double lithoThickness = projectReader.equilibriumOceanicLithosphereThickness();
  arguments().setDoubleArguments({0.9*lithoThickness, 1.1*lithoThickness});
}

}  // namespace ua

}  // namespace casaWizard
