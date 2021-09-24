//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <map>
#include <string>
#include <vector>

namespace casaWizard
{
class CalibrationTargetManager;

typedef std::vector<double> DepthVector;
typedef std::vector<double> PropertyVector;
typedef std::pair<DepthVector, PropertyVector> DepthPropertyPair;
typedef std::map<std::string, DepthPropertyPair> ModelDataPropertyMap;

class OneDModelDataExtractor
{
public:
  explicit OneDModelDataExtractor(const casaWizard::CalibrationTargetManager& ctManager, const std::string& iterationFolder, const std::string& project3dFilename);

  ModelDataPropertyMap extract(const std::string& property) const;
private:
  const CalibrationTargetManager& ctManager_;
  const std::string& iterationFolder_;
  const std::string& project3dFilename_;
};

} // namespace casaWizard
