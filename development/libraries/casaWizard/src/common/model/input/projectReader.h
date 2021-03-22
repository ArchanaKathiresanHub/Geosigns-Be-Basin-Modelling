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
  virtual QStringList surfaceNames() const = 0;
  virtual QStringList lithologyNames() const = 0;
  virtual QStringList mapNames() const = 0;
  virtual QStringList lithologyTypesForLayer(const int layerIndex) const = 0;
  virtual QVector<double> lithologyValuesForLayerAtLocation(const int layerIndex, const double xLoc, const double yLoc) const = 0;
  virtual size_t getLayerID(const std::string& layerName) const = 0;
  virtual void domainRange(double& xMin, double& xMax, double& yMin, double& yMax) const = 0;

  virtual double heatProductionRate() const = 0;
  virtual double initialLithosphericMantleThickness() const = 0;
  virtual double equilibriumOceanicLithosphereThickness() const = 0;
  virtual QVector<double> agesFromMajorSnapshots() const = 0;

  virtual void setRelevantOutputParameters(const QStringList& activeProperties, const std::string& saveName = "") = 0;
  virtual void setScaling(int scaleX, int scaleY, const std::string& saveName = "") = 0;
  virtual int lowestSurfaceWithTWTData() const = 0;
  virtual bool basementSurfaceHasTWT() const = 0;

};

} // namespace casaWizard
