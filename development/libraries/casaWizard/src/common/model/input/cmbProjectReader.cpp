//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "cmbProjectReader.h"

#include "cmbAPI.h"
#include "OutputProperty.h"

using namespace DataAccess;
using namespace Interface;

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
    loaded_ = (cmbModel_->loadModelFromProjectFile(projectFile.toStdString()) == ErrorHandler::ReturnCode::NoError);
}

int CMBProjectReader::lowestSurfaceWithTWTData() const
{
  if (!loaded_)
  {
    return DataAccess::Interface::DefaultUndefinedScalarIntValue;
  }

  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  const int nrSurfaces = stratigraphyManager.surfacesIDs().size();

  // For-loop in opposite direction to start with lowest surface
  for (int i = nrSurfaces - 1; i >= 0; i--)
  {
    if ( hasTWTData(i) )
    {
      return i;
    }
  }

  return DataAccess::Interface::DefaultUndefinedScalarIntValue;
}

bool CMBProjectReader::hasTWTData(int surfaceID) const
{
  if (!loaded_)
  {
    return false;
  }
  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  return ( (stratigraphyManager.twtValue(surfaceID) != DataAccess::Interface::DefaultUndefinedScalarValue ||
           ! stratigraphyManager.twtGridName(surfaceID).empty()) &&
           stratigraphyManager.twtGridName(surfaceID).find("Calculated_TWT_for_") == std::string::npos );
}

double CMBProjectReader::getDepth(int surfaceID) const
{
   if (!loaded_)
   {
     return DataAccess::Interface::DefaultUndefinedScalarValue;
   }
   return cmbModel_->tableValueAsDouble("StratIoTbl", size_t(surfaceID), "Depth");
}

QString CMBProjectReader::getDepthGridName(int surfaceID) const
{
  if (!loaded_)
  {
    return QString();
  }
  return QString::fromStdString(cmbModel_->tableValueAsString("StratIoTbl", size_t(surfaceID), "DepthGrid"));
}

double CMBProjectReader::getThickness(int surfaceID) const
{
   if (!loaded_)
   {
     return DataAccess::Interface::DefaultUndefinedScalarValue;
   }
   return cmbModel_->tableValueAsDouble("StratIoTbl", size_t(surfaceID), "Thickness");
}

bool CMBProjectReader::basementSurfaceHasTWT() const
{
  if (!loaded_)
  {
    return false;
  }

  const int basementSurfaceID = cmbModel_->stratigraphyManager().surfacesIDs().back();
  return lowestSurfaceWithTWTData() == basementSurfaceID;
}

bool CMBProjectReader::hasDepthDefinedInAllLayers() const
{
  if (!loaded_)
  {
    return false;
  }

  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  for (const mbapi::StratigraphyManager::LayerID id : stratigraphyManager.layersIDs() )
  {
    if (stratigraphyManager.depthGridName(id).empty() && IsValueUndefined(stratigraphyManager.depthScalarValue(id)))
    {
      return false;
    }
  }
  return true;
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

QStringList casaWizard::CMBProjectReader::surfaceNames() const
{
  if (!loaded_)
  {
    return QStringList();
  }

  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  std::vector<std::string> surfaceNames;
  const std::vector<mbapi::StratigraphyManager::LayerID> ids = stratigraphyManager.surfacesIDs();
  for( const mbapi::StratigraphyManager::LayerID& id : ids)
  {
    surfaceNames.push_back(stratigraphyManager.surfaceName(id));
  }

  return stringVectorToStringList(surfaceNames);
}

void CMBProjectReader::domainRange(double& xMin, double& xMax, double& yMin, double& yMax) const
{
  if (!loaded_)
  {
    return;
  }  

  cmbModel_->highResOrigin(xMin, yMin);
  double xDim, yDim;
  cmbModel_->highResAreaSize(xDim, yDim);
  xMax = xMin + xDim;
  yMax = yMin + yDim;
}

size_t CMBProjectReader::getLayerID(const std::string& layerName) const
{
  if (!loaded_)
  {
    return Utilities::Numerical::NoDataIDValue;
  }

  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  return stratigraphyManager.layerID(layerName);
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

QStringList CMBProjectReader::mapNamesT2Z() const
{
  QStringList maps = mapNames();
  QStringList mapsT2Z;
  for ( QString map : maps )
  {
    if (map.startsWith("T2Z"))
    {
      mapsT2Z.push_back(map);
    }
  }

  return mapsT2Z;
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

QVector<double> CMBProjectReader::lithologyValuesForLayerAtLocation(const int layerIndex, const double xLoc, const double yLoc) const
{
  if (!loaded_)
  {
   return {};
  }

  QVector<double> values;

  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  std::vector<std::string> lithoNames;
  std::vector<double> lithoPercent;
  std::vector<std::string> lithoPercMap;
  stratigraphyManager.layerLithologiesList(layerIndex, lithoNames, lithoPercent, lithoPercMap);
  for (int i = 0; i<2; ++i)
  {
    if (!IsValueUndefined(lithoPercent[i]))
    {
      values.push_back(lithoPercent[i]);
    }
    else if (!lithoPercMap[i].empty())
    {
      mbapi::MapsManager& manager = cmbModel_->mapsManager();
      mbapi::MapsManager::MapID id = manager.findID(lithoPercMap[i]);
      values.push_back(manager.mapGetValue(id, xLoc, yLoc));
    }
    else if (i==1)
    {
      values.push_back(100.0 - values[0]);
    }
  }
  values.push_back(100.0 - values[0] - values[1]);

  return values;
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

QString CMBProjectReader::getLayerUnderSurface(const QString& surfaceName) const
{
   if (!loaded_)
   {
     return "";
   }

   const int surfaceID = surfaceNames().indexOf(surfaceName);
   const QStringList layers = layerNames();
   if (surfaceID >= 0 && surfaceID < layers.size())
   {
      return layers[surfaceID];
   }

   return "";
}

} // namespace casaWizard
