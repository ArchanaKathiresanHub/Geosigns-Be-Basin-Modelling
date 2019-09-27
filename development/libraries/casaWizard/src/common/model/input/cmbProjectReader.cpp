#include "cmbProjectReader.h"

#include "cmbAPI.h"

namespace casaWizard
{

namespace
{

QStringList stringVectorToStringList(const std::vector<std::string>& in)
{
  QStringList out;
  for (const std::string& entry : in)
  {
    out.push_back(QString::fromStdString(entry));
  }
  return out;
}

}

CMBProjectReader::CMBProjectReader() :
  ProjectReader(),
  cmbModel_{new mbapi::Model()},
  loaded_{false}
{
}

CMBProjectReader::~CMBProjectReader()
{
}

void CMBProjectReader::load(const QString& projectFile)
{
  cmbModel_->loadModelFromProjectFile(projectFile.toStdString().c_str());
  loaded_ = true;
}

QStringList CMBProjectReader::layerNames() const
{
  if (!loaded_)
  {
    return QStringList();
  }

  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  std::vector<std::string> layerNames;
  const std::vector<mbapi::StratigraphyManager::LayerID> ids = stratigraphyManager.layersIDs();
  for( const mbapi::StratigraphyManager::LayerID& id : ids)
  {
    layerNames.push_back(stratigraphyManager.layerName(id));
  }
  return stringVectorToStringList(layerNames);
}

QStringList CMBProjectReader::lithologyNames() const
{
  if (!loaded_)
  {
    return QStringList();
  }

  mbapi::LithologyManager& lithologyManager = cmbModel_->lithologyManager();
  std::vector<std::string> lithologyNames;
  const std::vector<mbapi::LithologyManager::LithologyID> ids = lithologyManager.lithologiesIDs();
  for( const mbapi::LithologyManager::LithologyID& id : ids)
  {
    lithologyNames.push_back(lithologyManager.lithologyName(id));
  }
  return stringVectorToStringList(lithologyNames);
}

QStringList CMBProjectReader::mapNames() const
{
  if (!loaded_)
  {
    return QStringList();
  }

  const mbapi::MapsManager& mapsManager = cmbModel_->mapsManager();
  const std::vector<std::string> mapNames = mapsManager.mapNames();
  return stringVectorToStringList(mapNames);
}

QStringList CMBProjectReader::lithologyTypesForLayer(const int layerIndex) const
{
  if (!loaded_)
  {
    return {};
  }
  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  std::vector<std::string> lithoNames;
  std::vector<double> lithoPercent;
  std::vector<std::string> lithoPercMap;
  stratigraphyManager.layerLithologiesList(layerIndex, lithoNames, lithoPercent, lithoPercMap);
  return stringVectorToStringList(lithoNames);
}

double CMBProjectReader::heatProductionRate() const
{
  double heatProd{0.0};
  if (loaded_)
  {
    heatProd = cmbModel_->tableValueAsDouble("BasementIoTbl", 0, "TopCrustHeatProd");
  }
  return heatProd;
}

double CMBProjectReader::initialLithosphericMantleThickness() const
{
  if (!loaded_)
  {
    return 0;
  }
  return cmbModel_->tableValueAsDouble("BasementIoTbl", 0, "InitialLithosphericMantleThickness");
}

double CMBProjectReader::equilibriumOceanicLithosphereThickness() const
{
  if (!loaded_)
  {
    return 0;
  }
  return cmbModel_->tableValueAsDouble("BasementIoTbl", 0, "EquilibriumOceanicLithosphereThickness");
}

QVector<double> CMBProjectReader::agesFromMajorSnapshots() const
{
  if (!loaded_)
  {
    return {};
  }

  const mbapi::SnapshotManager & snapshotManager = cmbModel_->snapshotManager();
  return QVector<double>::fromStdVector(snapshotManager.agesFromMajorSnapshots());
}

} // namespace casaWizard
