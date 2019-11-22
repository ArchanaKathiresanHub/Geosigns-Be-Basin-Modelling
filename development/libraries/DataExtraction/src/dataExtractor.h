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

#include <memory>
#include <string>

namespace DataAccess
{
namespace Interface
{
class Formation;
class Grid;
class Property;
class Snapshot;
class Surface;
class ProjectHandle;
}
}

namespace DataExtraction
{

class DataExtractor
{
public:
  explicit DataExtractor();
  virtual ~DataExtractor() = default;

protected:
  virtual DataAccess::Interface::ProjectHandle& getProjectHandle() const = 0;

  std::string getMapsFileName( const std::string& propertyName ) const;

  std::string getPropertyFormationDataGroupName( const DataAccess::Interface::Formation* formation,
                                                 const DataAccess::Interface::Surface* surface,
                                                 const DataAccess::Interface::Property* property,
                                                 const DataAccess::Interface::Snapshot* snapshot) const;
};

} // namespace DataExtraction
