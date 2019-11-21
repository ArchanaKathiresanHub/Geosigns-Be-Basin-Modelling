//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSourceRock.h"

#include "CommonDefinitions.h"
#include "Genex0dSourceRockDefaultProperties.h"

#include "ConstantsGenex.h"
#include "Simulator.h"
#include "SourceRockNode.h"

#include "Interface.h"

namespace genex0d
{

Genex0dSourceRock::Genex0dSourceRock(Interface::ProjectHandle* projectHandle,
                                     const Genex0dInputData & inData) :
  Genex6::GenexSourceRock{projectHandle, nullptr},
  m_sourceRockType{inData.sourceRockType},
  m_srProperties{},
  m_formationName{inData.formationName},
  m_vreThreshold{0.5}, // TODO: check if this value can be obtained from input
  m_vesMax{50}, // TODO: check if this value can be obtained from input
  m_adsorptionCapacityFunctionName{""}, // TODO: check if this value can be obtained from input
  m_adsorptionSimulatorName{""} // TODO: check if this value can be obtained from input

{
  m_srProperties = Genex0dSourceRockDefaultProperties::getInstance().getProperties(m_sourceRockType);
  setPropertiesFromInput(inData.ToCIni, inData.SCVRe05, inData.HCVRe05);
}

Genex0dSourceRock::~Genex0dSourceRock()
{
}

void Genex0dSourceRock::setPropertiesFromInput(const double ToCIni, const double SCVRe05, const double HCVRe05)
{
  m_srProperties.setTocIni(ToCIni);
  m_srProperties.setSCVRe05(SCVRe05);
  m_srProperties.setHCVRe05(HCVRe05);
}

const std::string & Genex0dSourceRock::getLayerName (void) const
{
  return m_formationName;
}

const std::string & Genex0dSourceRock::getType (void) const
{
  return m_sourceRockType;
}

const double & Genex0dSourceRock::getHcVRe05(void) const
{
  return m_srProperties.HCVRe05();
}

const double & Genex0dSourceRock::getScVRe05(void) const
{
  return m_srProperties.SCVRe05();
}

const double & Genex0dSourceRock::getPreAsphaltStartAct(void) const
{
  return m_srProperties.activationEnergy();
}

const double & Genex0dSourceRock::getAsphalteneDiffusionEnergy(void) const
{
  return m_srProperties.AsphalteneDiffusionEnergy();
}

const double & Genex0dSourceRock::getResinDiffusionEnergy(void) const
{
  return m_srProperties.ResinDiffusionEnergy();
}

const double & Genex0dSourceRock::getC15AroDiffusionEnergy(void) const
{
  return m_srProperties.C15AroDiffusionEnergy();
}

const double & Genex0dSourceRock::getC15SatDiffusionEnergy(void) const
{
  return m_srProperties.C15SatDiffusionEnergy();
}

bool Genex0dSourceRock::isVREoptimEnabled(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

const double & Genex0dSourceRock::getVREthreshold(void) const
{
  return m_vreThreshold;
}

const std::string & Genex0dSourceRock::getBaseSourceRockType (void) const
{
  return m_sourceRockType; // TODO: Check if it is different from SR type. If so can it be obtained from input?
}

bool Genex0dSourceRock::isVESMaxEnabled(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

const double & Genex0dSourceRock::getVESMax(void) const
{
  return m_vesMax; // TODO: See if it's necessary to be parsed from input
}

bool Genex0dSourceRock::doApplyAdsorption(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

bool Genex0dSourceRock::adsorptionIsTOCDependent(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

bool Genex0dSourceRock::doComputeOTGC(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

const string & Genex0dSourceRock::getAdsorptionCapacityFunctionName(void) const
{
  return m_adsorptionCapacityFunctionName; // TODO: See if it's necessary to be parsed from input
}

const string & Genex0dSourceRock::getAdsorptionSimulatorName(void) const
{
  return m_adsorptionSimulatorName; // TODO: See if it's necessary to be parsed from input
}

const DataAccess::Interface::GridMap * Genex0dSourceRock::getMap(DataAccess::Interface::SourceRockMapAttributeId attributeId) const
{
  switch (attributeId)
  {
    case DataAccess::Interface::SourceRockMapAttributeId::TocIni :
      {
        return loadMap(attributeId, m_srProperties.TocIni());
      }
    case DataAccess::Interface::SourceRockMapAttributeId::HcIni :
      {
        return loadMap(attributeId, m_srProperties.HCVRe05());
      }
  }

  return nullptr;
}

const DataAccess::Interface::GridMap * Genex0dSourceRock::loadMap(DataAccess::Interface::SourceRockMapAttributeId attributeId, const double mapScalarValue) const
{
  const  DataAccess::Interface::Grid * grid = m_projectHandle->getActivityOutputGrid();
  if (!grid)
  {
    grid = m_projectHandle->getInputGrid();
  }
  return m_projectHandle->getFactory()->produceGridMap (this, attributeId, grid, mapScalarValue);
}

} // namespace genex0d
