#include "topCrustHeatProductionGridScaling.h"

namespace casaWizard
{

namespace ua
{

TopCrustHeatProductionGridScaling::TopCrustHeatProductionGridScaling() :
  InfluentialParameter
  (
    "Continental crust heat production (A0) grid scaling",
    "\"TopCrustHeatProductionGridScaling\"",
    "Map scaling factor",
    "[-]",
    1
    )
{
  arguments().addArgument("Minimum map scaling factor", 0.5);
  arguments().addArgument("Maximum map scaling factor", 2.0);
  addPDFArgument();
}

void TopCrustHeatProductionGridScaling::fillArguments(const ProjectReader& /*projectReader*/)
{
}

} // namespace ua

}  // namespace casaWizard
