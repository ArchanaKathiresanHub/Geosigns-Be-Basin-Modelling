//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dSimulator class:

#pragma once

// GeoPhysics
#include "GeoPhysicsProjectHandle.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace Genex6
{
class GenexSourceRock;
}

namespace Genex0d
{

struct Genex0dInputData;
class Genex0dSourceRock;

class Genex0dSimulator : public GeoPhysics::ProjectHandle
{
public:
  Genex0dSimulator(database::ProjectFileHandlerPtr database,
                   const std::string & name,
                   const DataAccess::Interface::ObjectFactory* objectFactory);
  ~Genex0dSimulator();
  
  static Genex0dSimulator * CreateFrom(const std::string& fileName, DataAccess::Interface::ObjectFactory* objectFactory);
  bool run(const DataAccess::Interface::Formation* formation, const Genex0dInputData& inData, unsigned int indI, unsigned int indJ,
           double thickness, double inorganicDensity, const std::vector<double>& time, const std::vector<double>& temperature,
           const std::vector<double>& pressure, const std::vector<double>& VRE, const std::vector<double>& porePressure, const std::vector<double>& permeability,
           const std::vector<double>& porosity);
  bool saveTo(const std::string & outputFileName);
  void setLangmuirData(const std::string& adsorptionFunctionTPVData, const std::string& langmuirName);
  void setIrreducibleWaterSaturationData(const std::string& irreducibleWaterSaturationData);

private:
  void setRequestedOutputProperties();
  void setRequestedSpeciesOutputProperties();
  void registerProperties();
  bool isPropertyRegistered(const std::string & propertyName);
  bool computeSourceRock(const DataAccess::Interface::Formation * aFormation);

  std::unique_ptr<Genex0dSourceRock> m_gnx0dSourceRock;
  std::unordered_set<std::string> m_registeredProperties;
  std::vector<std::string> m_shaleProperties;
  std::vector<std::string> m_requestedProperties;
  std::vector<std::string> m_expelledToCarrierBedProperties;
  std::vector<std::string> m_expelledToSourceRockProperties;
  std::vector<std::string> m_expelledToCarrierBedPropertiesS;
  std::vector<std::string> m_expelledToSourceRockPropertiesS;
};

} // namespace genex0d
