//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "dataExtractor.h"

#include <string>
#include <vector>
#include <memory>

namespace DataAccess
{
namespace Mining
{
class ProjectHandle;
class ObjectFactory;
}
}

namespace GeoPhysics
{
class GeoPhysicsFormation;
}

namespace DataExtraction
{

class HDFReadManager;

class DataDriller : public DataExtractor
{
public:
  explicit DataDriller( const std::string& inputProjectFileName );
  virtual ~DataDriller();

  void run( const bool doCalculateTrapAndMissingProperties );
  void saveToFile( const std::string& outputProjectFileName );
  bool readPropertyFromHDF(const std::string& surfaceName, const std::string& formationName, const double x, const double y, const double z,
                             const DataAccess::Interface::Snapshot* snapshot, const DataAccess::Interface::Property* property, double &value) const;

  const DataAccess::Interface::Grid* getGridLowResolution() const;
  DataAccess::Interface::ProjectHandle& getProjectHandle() const;

  const GeoPhysics::GeoPhysicsFormation* getFormation(const double i, const double j, const double z, const DataAccess::Interface::Snapshot* snapshot) const;

private:
  void performDirectDataDrilling();
  void perform3DDataMining();
  bool allDataObtained();

  bool get2dPropertyFromHDF( const double i, const double j, const DataAccess::Interface::Surface* surface, const DataAccess::Interface::Formation* formation,
                             const DataAccess::Interface::Property* property, const DataAccess::Interface::Snapshot* snapshot, double& value ) const;

  bool checkDataGroupInHDFFile( HDFReadManager& hdfReadManager, const std::string& dataGroup, const std::string& snapshotFileName ) const;
  bool get3dPropertyFromHDF( const double i,  const double j, const double z, const DataAccess::Interface::Property* property,
                             const DataAccess::Interface::Snapshot* snapshot, double& value ) const;
  bool get3dPropertyFromHDF( const double i, const double j, const std::string& mangledName, const bool isSurfaceTop, const DataAccess::Interface::Property* property,
                             const DataAccess::Interface::Snapshot* snapshot, double& value) const;

  double interpolate1d(const double u, const double l, const double k) const;
  double getKfraction(const double u, const double l, const double v) const;

  std::vector<bool> m_obtained;
  DataAccess::Mining::ObjectFactory* m_objectFactory;
  std::unique_ptr<DataAccess::Mining::ProjectHandle> m_projectHandle;
  const DataAccess::Interface::Grid* m_gridHighResolution;
  const DataAccess::Interface::Grid* m_gridLowResolution;
};

} // namespace DataExtraction
