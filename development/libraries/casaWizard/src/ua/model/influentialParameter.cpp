#include "influentialParameter.h"

#include "model/influentialParameter/equilibriumOceanicLithosphereThickness.h"
#include "model/influentialParameter/crustThinning.h"
#include "model/influentialParameter/crustThinningOneEvent.h"
#include "model/influentialParameter/initialLithosphericMantleThickness.h"
#include "model/influentialParameter/surfaceTemperature.h"
#include "model/influentialParameter/thermalConductivity.h"
#include "model/influentialParameter/topCrustHeatProduction.h"
#include "model/influentialParameter/topCrustHeatProductionGrid.h"
#include "model/influentialParameter/topCrustHeatProductionGridScaling.h"

#include "model/input/projectReader.h"

namespace casaWizard
{

namespace ua
{

InfluentialParameter::InfluentialParameter(const QString& name, const QString& casaName, const QString& label, const QString& unitSI, const int totalNumberOfParameters) :
  name_{name},
  casaName_{casaName},
  labelName_{label},
  unitSI_{unitSI},
  arguments_{},
  totalNumberOfParameters_{totalNumberOfParameters}
{
}

int InfluentialParameter::version() const
{
  return 0;
}

void InfluentialParameter::addPDFArgument()
{
  const QString pdfName{"Probability density function"};
  const QStringList pdfType{"Block", "Normal", "Triangle"};
  arguments().addArgument(pdfName, pdfType);
}

InfluentialParameter* InfluentialParameter::createFromIndex(int index)
{
  if (index >= types().size() || index < 0)
  {
    index = 0;
  }
  return create(types()[index]);
}

InfluentialParameter* InfluentialParameter::createFromList(const int version, const QStringList& list)
{
  if (list.isEmpty())
  {
    return createFromIndex(0);
  }

  const int index = list[0].toInt();
  InfluentialParameter* ip = createFromIndex(index);
  if (list.size() != 2)
  {
    return ip;
  }

  ip->read(version, list[1]);
  return ip;
}

InfluentialParameter* InfluentialParameter::create(const Type& type)
{
  switch (type)
  {
    case Type::TopCrustHeatProduction:
      return new TopCrustHeatProduction();
    case Type::TopCrustHeatProductionGrid:
      return new TopCrustHeatProductionGrid();
    case Type::TopCrustHeatProductionGridScaling:
      return new TopCrustHeatProductionGridScaling();

    case Type::CrustThinning:
      return new CrustThinning();
    case Type::CrustThinningOneEvent:
      return new CrustThinningOneEvent();
    case Type::ThermalConductivity:
      return new ThermalConductivity();

    case Type::EquilibriumOceanicLithosphereThickness:
      return new EquilibriumOceanicLithosphereThickness();
    case Type::InitialLithosphericMantleThickness:
      return new InitialLithosphericMantleThickness();

    case Type::SurfaceTemperature:
      return new SurfaceTemperature();
    default:
      return new TopCrustHeatProduction();
  }
}

QStringList InfluentialParameter::list()
{
  QStringList list;

  for (const Type& type : types())
  {
    InfluentialParameter* tmp = create(type);
    list.append(tmp->name());
    delete tmp;
  }

  return list;
}

QStringList InfluentialParameter::write() const
{
  QStringList out;
  const int index = list().indexOf(name_);
  out << QString::number(index)
      << arguments_.write();
  return out;
}

void InfluentialParameter::read(const int version, const QString& input)
{
  arguments_.read(version, input);
}

QVector<InfluentialParameter::Type> InfluentialParameter::types()
{
  return
  {
    Type::TopCrustHeatProduction,
    Type::TopCrustHeatProductionGrid,
    Type::TopCrustHeatProductionGridScaling,
    Type::CrustThinning,
    Type::CrustThinningOneEvent,
    Type::ThermalConductivity,
    Type::EquilibriumOceanicLithosphereThickness,
    Type::InitialLithosphericMantleThickness,
    Type::SurfaceTemperature
  };
}

QString InfluentialParameter::label() const
{
  return QString(nameArgumentBased() + " " + unitSI_);
}

QString InfluentialParameter::nameArgumentBased() const
{
  return name();
}

QString InfluentialParameter::casaName() const
{
  return casaName_;
}

QString InfluentialParameter::name() const
{
  return name_;
}

QString InfluentialParameter::labelName() const
{
  return labelName_;
}

QString InfluentialParameter::unitSI() const
{
  return unitSI_;
}

int InfluentialParameter::totalNumberOfParameters() const
{
  return totalNumberOfParameters_;
}

ArgumentList& InfluentialParameter::arguments()
{
  return arguments_;
}

const ArgumentList& InfluentialParameter::arguments() const
{
  return arguments_;
}

} // namespace ua

} // namespace casaWizard

