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

namespace DataExtraction
{

class DataDriller : public DataExtractor
{
public:
  explicit DataDriller( const std::string& inputProjectFileName );
  virtual ~DataDriller();

  void run( const bool doCalculateTrapAndMissingProperties );
  void saveToFile( const std::string& outputProjectFileName );

protected:
  DataAccess::Interface::ProjectHandle& getProjectHandle() const;

private:
  void readDataFromHDF();
  bool allDataReadFromHDF();
  void calculateTrapAndMissingProperties();

  double get2dPropertyFromHDF( const double i, const double j, const DataAccess::Interface::Surface* surface, const DataAccess::Interface::Formation* formation,
                               const DataAccess::Interface::Property* property, const DataAccess::Interface::Snapshot* snapshot, const unsigned int recordIndex );

  double get3dPropertyFromHDF( const double i,  const double j, const double z,
                               const DataAccess::Interface::Property* property, const DataAccess::Interface::Snapshot* snapshot, const unsigned int recordIndex );

  double interpolate1d(const double u, const double l, const double k);
  double getKfraction(const double u, const double l, const double v);

  DataAccess::Mining::ObjectFactory* m_objectFactory;
  std::unique_ptr<DataAccess::Mining::ProjectHandle> m_projectHandle;
  const DataAccess::Interface::Grid* m_gridHighResolution;
  const DataAccess::Interface::Grid* m_gridLowResolution;
  std::vector<bool> m_readFromHDF;
};

} // namespace DataExtraction
