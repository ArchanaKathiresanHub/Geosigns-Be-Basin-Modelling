#include "topCrustHeatProduction.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

TopCrustHeatProduction::TopCrustHeatProduction() :
   InfluentialParameter
   (
      "Continental crust heat production (A0)",
      "\"BasementIoTbl:TopCrustHeatProd\"",
      "Top Crust Heat Production",
      "[uW/m^3]",
      1,
      InfluentialParameter::Type::TopCrustHeatProduction
   )
{
   arguments().addArgument("Minimum range value", 0.0);
   arguments().addArgument("Maximum range value", 0.0);
   addPDFArgument();
}

void TopCrustHeatProduction::fillArguments(const ProjectReader& projectReader)
{
   const double heatProductionRate = projectReader.heatProductionRate();
   arguments().setDoubleArguments({0.9*heatProductionRate, 1.1*heatProductionRate});
}

} // namespace ua

}  // namespace casaWizard
