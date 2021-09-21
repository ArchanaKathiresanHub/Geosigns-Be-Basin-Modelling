//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QString>

#include <map>
#include <string>
#include <vector>

namespace casaWizard
{
class CasaScenario;

typedef std::vector<double> DepthVector;
typedef std::vector<double> PropertyVector;
typedef std::map<QString, std::pair<DepthVector, PropertyVector>> ModelDataPropertyMap;

class OneDModelDataExtractor
{
public:
  explicit OneDModelDataExtractor(const CasaScenario& scenario);

  ModelDataPropertyMap extract(const std::string& property) const;
private:
  const CasaScenario& scenario_;
};

} // namespace casaWizard
