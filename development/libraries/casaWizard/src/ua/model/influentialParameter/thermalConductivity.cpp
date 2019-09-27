#include "thermalConductivity.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

ThermalConductivity::ThermalConductivity() :
  InfluentialParameter
  (
    "Lithology: Standard TP Thermal Conductivity Coefficient (StpThCond )",
    "\"LithotypeIoTbl:StpThCond\"",
    "StpThCond",
    "[W/m.K]",
    1
  )
{
  arguments().addArgument("Lithology name", QStringList("Select project3d file"));
  arguments().addArgument("Minimum value", 0.0);
  arguments().addArgument("Maximum value", 0.0);
  addPDFArgument();
}

void ThermalConductivity::fillArguments(const ProjectReader& projectReader)
{
  const QStringList lithologyNames = projectReader.lithologyNames();
  arguments().setListOptions(0, lithologyNames);
}

QString ThermalConductivity::nameArgumentBased() const
{
  return QString(labelName() + " (" + arguments().getSelectedValue(0) + ")");
}

} // namespace ua

}  // namespace casaWizard
