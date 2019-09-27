//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FDCFastCauldronManager class manages the initialization and run of fastCauldron application
#pragma once

#include "FastcauldronStartup.h"

#include "Interface.h"

#include <memory>
#include <string>

class FastcauldronSimulator;

namespace fastDepthCalibration
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
  Interface::GridMap * getPropertyGridMap(const std::string & propertyName, const std::string & layerName);
  void finalizeFastCauldronSturtup();

private:
  const DataAccess::Interface::PropertyValueList * getPropertyValues(const std::string & propertyName, const string & layerName) const;

  FastcauldronStartup m_fastcauldronStartup;

  const long m_XScalingFactor;
  const long m_YScalingFactor;
  const bool m_noExtrapolationFlag;
};

} // namespace fastDepthCalibration
