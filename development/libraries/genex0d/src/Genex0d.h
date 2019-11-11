//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0d class: is the main API class for genex0d

#pragma once

#include "DerivedPropertyManager.h"

#include <memory>
#include <string>
#include <vector>

namespace Genex6
{
class GenexSourceRock;
}

namespace genex0d
{

class Genex0dFormationManager;
class Genex0dGenexSourceRock;
class Genex0dInputData;
class Genex0dProjectManager;
class Genex0dSourceRock;

class Genex0d
{
public:
  Genex0d(const Genex0dInputData & inputData);
  ~Genex0d();

  void run();
  void printResults(const std::string & outputFileName) const;

private:
  void initialize();
  void setSourceRockInput(const double inorganicDensity);
  void setRequestedOutputProperties();

  //--------------------
  void setRequestedOutputProperties();
  void setRequestedSpeciesOutputProperties();
  void registerProperties();
  bool isPropertyRegistered(const string & propertyName);
  DerivedProperties::DerivedPropertyManager * m_propertyManager;
  bool computeSourceRock ( Genex6::GenexSourceRock * aSourceRock, const DataAccess::Interface::Formation * aFormation );
  //---------------------

  const Genex0dInputData & m_inData;
  std::unique_ptr<Genex0dFormationManager> m_formationMgr;
  std::unique_ptr<Genex0dProjectManager> m_projectMgr;
  std::unique_ptr<Genex0dSourceRock> m_sourceRock;
  Genex0dGenexSourceRock * m_genexSourceRock;

  //---------------------
  std::vector<std::string> m_registeredProperties;
  std::vector<std::string> m_shaleProperties;
  std::vector<std::string> m_requestedProperties;
  std::vector<std::string> m_expelledToCarrierBedProperties;
  std::vector<std::string> m_expelledToSourceRockProperties;
  std::vector<std::string> m_expelledToCarrierBedPropertiesS;
  std::vector<std::string> m_expelledToSourceRockPropertiesS;
};

} // namespace genex0d
