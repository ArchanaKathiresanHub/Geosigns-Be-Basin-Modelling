//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FDCFastCauldronManager class manages the initialization and run of fastCauldron application
#pragma once

#include <memory>
#include <vector>
#include <string>

class FastcauldronSimulator;
class FastcauldronStartup;

namespace DataAccess
{
   namespace Interface
   {
      class GridMap;
      class PropertyValue;
   }
}

namespace fastDepthConversion
{

class FDCFastCauldronManager
{
public:
  FDCFastCauldronManager(int argc,
                         char** argv,
                         const long scX,
                         const long scY,
                         const bool noExtrapolationFlag);
  ~FDCFastCauldronManager();

  void prepareAndRunFastCauldron();
  DataAccess::Interface::GridMap * getPropertyGridMap(const std::string & propertyName, const std::string & layerName);
  void finalizeFastCauldronStartup();

private:
  const std::vector<const DataAccess::Interface::PropertyValue *> * getPropertyValues(const std::string & propertyName, const std::string & layerName) const;

  std::unique_ptr<FastcauldronStartup> m_fastcauldronStartup;

  const long m_XScalingFactor;
  const long m_YScalingFactor;
  const bool m_noExtrapolationFlag;
};

} // namespace fastDepthConversion
