//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dGenexSourceRock class gets source rock (SR) input data, and computes SR properties.

#pragma once

#include "GenexSourceRock.h"
#include "Genex0dSourceRockProperty.h"

namespace DataAccess
{
namespace Interface
{
class GridMap;
} // namespace Interface
} // namespace DataAccess

namespace genex0d
{

class Genex0dInputData;

class Genex0dGenexSourceRock : public Genex6::GenexSourceRock
{
public:
  explicit Genex0dGenexSourceRock(DataAccess::Interface::ProjectHandle * projectHandle, const Genex0dInputData & inData);
  virtual ~Genex0dGenexSourceRock();

  const std::string & getLayerName (void) const final;
  const std::string & getType (void) const final;
  const double & getHcVRe05(void) const final;
  const double & getScVRe05(void) const final;
  const double & getPreAsphaltStartAct(void) const final;
  const double & getAsphalteneDiffusionEnergy(void) const final;
  const double & getResinDiffusionEnergy(void) const final;
  const double & getC15AroDiffusionEnergy(void) const final;
  const double & getC15SatDiffusionEnergy(void) const final;
  bool isVREoptimEnabled(void) const final;
  const double & getVREthreshold(void) const final;
  const std::string & getBaseSourceRockType(void) const final;
  bool isVESMaxEnabled(void) const final;
  const double & getVESMax(void) const final;
  bool doApplyAdsorption(void) const final;
  bool adsorptionIsTOCDependent(void) const final;
  bool doComputeOTGC(void) const final;
  const string & getAdsorptionCapacityFunctionName(void) const final;
  const string & getAdsorptionSimulatorName(void) const final;
  const DataAccess::Interface::GridMap * getMap(DataAccess::Interface::SourceRockMapAttributeId attributeId) const final;

private:
  void setPropertiesFromInput();
  const GridMap * loadMap (DataAccess::Interface::SourceRockMapAttributeId attributeId, const double mapScalarValue) const;

  const Genex0dInputData & m_inData;
  Genex0dSourceRockProperty m_srProperties;
  const std::string & m_formationName;
  const std::string & m_sourceRockType;
  const double m_vreThreshold;
  const double m_vesMax;
  const std::string & m_adsorptionCapacityFunctionName;
  const std::string & m_adsorptionSimulatorName;
};

} // namespace genex0d
