#include "surfaceTemperature.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

SurfaceTemperature::SurfaceTemperature() :
  InfluentialParameter
  (
    "Surface temperature history",
    "\"SurfaceTempIoTbl:Temperature\"",
    "Temperature",
    "[C]",
    1
  )
{
  arguments().addArgument("Age", QStringList("Select project3d file"));
  arguments().addArgument("Minimum temperature", 0);
  arguments().addArgument("Maximum temperature", 30);
  addPDFArgument();
}

void SurfaceTemperature::fillArguments(const ProjectReader& projectReader)
{
  const QVector<double> ages = projectReader.agesFromMajorSnapshots();
  QStringList ageStrings;
  for (const double d : ages)
  {
    ageStrings << QString::number(d, 'g');
  }
  arguments().setListOptions(0, ageStrings);
}

QString SurfaceTemperature::nameArgumentBased() const
{
  return QString(labelName() + " (age = " + arguments().getSelectedValue(0) + " Ma)");
}

}  // namespace ua

}  // namespace casaWizard
