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
  cmbModel_->loadModelFromProjectFile(projectFile.toStdString().c_str());
  loaded_ = true;
}

void CMBProjectReader::setRelevantOutputParameters(const QStringList& activeProperties, const std::string& saveName)
{
  if (loaded_)
  {
    int tableSize = cmbModel_->tableSize("FilterTimeIoTbl");
    for (size_t i = 0; i < tableSize; i++)
    {
      cmbModel_->setTableValue("FilterTimeIoTbl", i, "OutputOption", "None");
    }

    for (int i = 0; i < activeProperties.size(); i++)
    {
      // Mapping to Project3d names if necessary
      if (activeProperties[i] == QString("BulkDensity"))
      {
        cmbModel_->propertyManager().requestPropertyInSnapshots("BulkDensityVec", "SedimentsOnly");
      }
      else if (activeProperties[i] == QString("SonicSlowness"))
      {
        cmbModel_->propertyManager().requestPropertyInSnapshots("SonicVec", "SedimentsOnly");
      }
      else if (activeProperties[i] == QString("Velocity"))
      {
        cmbModel_->propertyManager().requestPropertyInSnapshots("VelocityVec", "SedimentsOnly");
      }
      else if (activeProperties[i] == QString("VRe"))
      {
        cmbModel_->propertyManager().requestPropertyInSnapshots("VrVec", "SedimentsOnly");
      }
      else
      {
        cmbModel_->propertyManager().requestPropertyInSnapshots(activeProperties[i].toStdString(), "SedimentsOnly");
      }
    }

    // Always output the Depth
    cmbModel_->propertyManager().requestPropertyInSnapshots("Depth", "SedimentsPlusBasement");

    //Save
    if (saveName == "")
    {
      cmbModel_->saveModelToProjectFile(cmbModel_->projectFileName());
    }
    else
    {
      cmbModel_->saveModelToProjectFile(saveName);
    }
  }
}

std::map<std::string, std::string> CMBProjectReader::readOutputProperties() const
{
  std::map<std::string, std::string> outputProperties;
  DataAccess::Interface::OutputPropertyList* outputPropertyList = cmbModel_->projectHandle()->getTimeOutputProperties();
  for (const OutputProperty* property : *outputPropertyList)
  {
    const std::string name = property->getName();
    const PropertyOutputOption option = property->getOption();
    std::string optionString;
    switch (option)
    {
      case PropertyOutputOption::NO_OUTPUT:
        optionString = "None";
        break;
      case PropertyOutputOption::SEDIMENTS_ONLY_OUTPUT:
        optionString = "SedimentsOnly";
        break;
      case PropertyOutputOption::SEDIMENTS_AND_BASEMENT_OUTPUT:
        optionString = "SedimentsPlusBasement";
        break;
      case PropertyOutputOption::SHALE_GAS_ONLY_OUTPUT:
        optionString = "ShaleGasOnly";
        break;
      case PropertyOutputOption::SOURCE_ROCK_ONLY_OUTPUT:
        optionString = "SourceRockOnly";
        break;
    }

    outputProperties.insert({name, optionString});
  }

  return outputProperties;
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

void CMBProjectReader::domainRange(double& xMin, double& xMax, double& yMin, double& yMax) const
{
  if (!loaded_)
  {
    return;
  }

  cmbModel_->origin(xMin, yMin);
  double xDim, yDim;
  cmbModel_->arealSize(xDim, yDim);
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

} // namespace casaWizard
