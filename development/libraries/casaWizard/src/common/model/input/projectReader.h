// Interface for interacting with Cauldron case
#pragma once

#include <QStringList>

namespace casaWizard
{

class ProjectReader
{
public:
  ProjectReader() = default;
  virtual ~ProjectReader() = default;

  virtual void load(const QString& projectFile) = 0;

  virtual QStringList layerNames() const = 0;
  virtual QStringList lithologyNames() const = 0;
  virtual QStringList mapNames() const = 0;
  virtual QStringList lithologyTypesForLayer(const int layerIndex) const = 0;

  virtual double heatProductionRate() const = 0;
  virtual double initialLithosphericMantleThickness() const = 0;
  virtual double equilibriumOceanicLithosphereThickness() const = 0;
  virtual QVector<double> agesFromMajorSnapshots() const = 0;
};

} // namespace casaWizard
