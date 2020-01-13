//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// SourceRockDefaultProperties class: contains maps to default values of Source Rock type properties.
// Note: The default values are obtained from BPA. Later on it must be checked whether or not the values are consistent with BPA and BPA2 values!
#pragma once

#include "SourceRockProperty.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Genex6
{

class SourceRockDefaultProperties
{
public:
  SourceRockDefaultProperties(const SourceRockDefaultProperties &) = delete;
  SourceRockDefaultProperties & operator = (const SourceRockDefaultProperties &) = delete;
  SourceRockDefaultProperties(const SourceRockDefaultProperties &&) = delete;
  SourceRockDefaultProperties & operator = (const SourceRockDefaultProperties &&) = delete;

  static const SourceRockDefaultProperties & getInstance();
  DataAccess::Interface::SourceRockProperty getProperties(const std::string & typeName) const;

  const std::unordered_map<std::string, std::string> & CfgFileNameBySRType() const;
private:
  SourceRockDefaultProperties();
  ~SourceRockDefaultProperties();

  void setPropertyMap(const std::string& sourceRockType,
                      const std::vector<std::string> & nameListNoSulphur,
                      const std::string & nameWithSulphur,
                      const double TocIni,
                      const double HCVRe05,
                      const double SCVRe05,
                      const double ActivationEnergy,
                      const double AsphalteneDiffusionEnergy,
                      const double ResinDiffusionEnergy,
                      const double C15AroDiffusionEnergy,
                      const double C15SatDiffusionEnergy,
                      const double HCVRe05WithSulphur,
                      const double SCVRe05WithSulphur);
  void setPropertyMaps();

  static SourceRockDefaultProperties m_instance;
  std::unordered_map<std::string, DataAccess::Interface::SourceRockProperty> m_properties;
  std::unordered_map<std::string, std::string> m_CfgFileNameBySRType;
};

} // namespace Genex6
