#include "topCrustHeatProductionGrid.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

TopCrustHeatProductionGrid::TopCrustHeatProductionGrid() :
   InfluentialParameter
   (
      "Continental crust heat production (A0) grid",
      "\"BasementIoTbl:TopCrustHeatProdGrid\"",
      "Top Crust Heat Production",
      "[-]",
      1,
      InfluentialParameter::Type::TopCrustHeatProductionGrid
   )
{
   arguments().addArgument("Minimum map name", QStringList("Select project3d file"));
   arguments().addArgument("Maximum map name", QStringList("Select project3d file"));
   addPDFArgument();
}

void TopCrustHeatProductionGrid::fillArguments(const ProjectReader& projectReader)
{
   const QStringList mapNames = projectReader.mapNames();
   arguments().setListOptions(0, mapNames);
   arguments().setListOptions(1, mapNames);
}

} // namespace ua

}  // namespace casaWizard
