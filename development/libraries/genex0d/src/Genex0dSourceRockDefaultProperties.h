//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dSourceRockDefaultProperties class: contains maps to default values of Source Rock type properties.
// Note: The default values are obtained from BPA. Later on it must be checked whether or not the values are consistent with BPA and BPA2 values!
#pragma once

#include "Genex0dSourceRockProperty.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace genex0d
{

class Genex0dSourceRockDefaultProperties
{
public:
  Genex0dSourceRockDefaultProperties(const Genex0dSourceRockDefaultProperties &) = delete;
  Genex0dSourceRockDefaultProperties & operator = (const Genex0dSourceRockDefaultProperties &) = delete;
  Genex0dSourceRockDefaultProperties(const Genex0dSourceRockDefaultProperties &&) = delete;
  Genex0dSourceRockDefaultProperties & operator = (const Genex0dSourceRockDefaultProperties &&) = delete;

  static const Genex0dSourceRockDefaultProperties & getInstance();
  Genex0dSourceRockProperty getProperties(const std::string & typeName) const;

private:
  Genex0dSourceRockDefaultProperties();
  ~Genex0dSourceRockDefaultProperties();

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

  static Genex0dSourceRockDefaultProperties m_instance;
  std::unordered_map<std::string, Genex0dSourceRockProperty> m_properties;
};

} // namespace genex0d
