//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <string>
#include <map>

namespace casaWizard
{

namespace functions
{
  std::string getUnit(const std::string& cauldronPropertyName)
  {
    std::map<std::string, std::string> propertyToUnitMapping;
    propertyToUnitMapping.insert({"TwoWayTime", "ms"});
    propertyToUnitMapping.insert({"Velocity", "m/s"});
    propertyToUnitMapping.insert({"SonicSlowness", "us/m"});
    propertyToUnitMapping.insert({"Temperature", "C"});
    propertyToUnitMapping.insert({"BulkDensity", "kg/m3"});
    propertyToUnitMapping.insert({"VRe", "%"});
    propertyToUnitMapping.insert({"Pressure", "MPa"});
    propertyToUnitMapping.insert({"GammaRay", "API"});

    return propertyToUnitMapping[cauldronPropertyName];
  }
}  // namespace functions

}  // namespace casaWizard
