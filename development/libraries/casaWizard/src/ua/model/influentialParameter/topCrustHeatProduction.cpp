#include "topCrustHeatProduction.h"

#include "model/input/projectReader.h"
#include "UndefinedValues.h"

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
   if (Utilities::isValueUndefined(heatProductionRate))
   {
      throw std::runtime_error("TopCrustHeatProduction can't be used as it is not defined in the base project.");
   }
   arguments().setDoubleArguments({0.9*heatProductionRate, 1.1*heatProductionRate});
}

} // namespace ua

}  // namespace casaWizard
