//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSourceRock.h"

#include "CommonDefinitions.h"
#include "SourceRockTypeNameMappings.h"

#include "ConstantsGenex.h"
#include "Simulator.h"
#include "SourceRockNode.h"

#include "Interface.h"

namespace genex0d
{

Genex0dSourceRock::Genex0dSourceRock(DataAccess::Interface::ProjectHandle & projectHandle,
                                     const Genex0dInputData& inData) :
  DataAccess::Interface::SourceRock{projectHandle, nullptr},
  m_formationName{inData.formationName},
  m_sourceRockType{inData.sourceRockType},
  m_vreThreshold{0.5},
  m_vesMax{inData.maxVes * Utilities::Maths::MegaPaToPa},
  m_vesMaxEnabled{inData.maxVesEnabled},
  m_adsorptionCapacityFunctionName{inData.whichAdsorptionFunction},
  m_applyAdsorption{inData.whichAdsorptionSimulator != ""},
  m_adsorptionSimulatorName{inData.whichAdsorptionSimulator},
  m_doOTCG{inData.doOTCG}
{
  setPropertiesFromInput(inData);
}

Genex0dSourceRock::~Genex0dSourceRock()
{
}

void Genex0dSourceRock::setPropertiesFromInput(const Genex0dInputData& inData)
{
  m_srProperties.setTypeNameID(extractTypeID());

  m_srProperties.setTocIni(inData.ToCIni);
  m_srProperties.setSCVRe05(inData.SCVRe05);
  m_srProperties.setHCVRe05(inData.HCVRe05);
  m_srProperties.setActivationEnergy(inData.activationEnergy);
  m_srProperties.setAsphalteneDiffusionEnergy(inData.asphalteneDiffusionEnergy);
  m_srProperties.setResinDiffusionEnergy(inData.resinDiffusionEnergy);
  m_srProperties.setC15AroDiffusionEnergy(inData.C15AroDiffusionEnergy);
  m_srProperties.setC15SatDiffusionEnergy(inData.C15SatDiffusionEnergy);
}

std::string Genex0dSourceRock::extractTypeID() const
{
  std::string typeId = "";
  std::unordered_map<std::string, std::string> typeToIDMap = Genex6::SourceRockTypeNameMappings::getInstance().CfgFileNameBySRType();
  std::unordered_map<std::string, std::string>::const_iterator it = typeToIDMap.find(m_sourceRockType);

  if(it != typeToIDMap.end())
  {
    typeId = it ->second;
  }

  return typeId;
}

const std::string & Genex0dSourceRock::getLayerName (void) const
{
  return m_formationName;
}

const std::string & Genex0dSourceRock::getType (void) const
{
  return m_sourceRockType;
}

const std::string & Genex0dSourceRock::getTypeID (void) const
{
  return m_srProperties.typeNameID();
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
  return m_sourceRockType;
}

bool Genex0dSourceRock::isVESMaxEnabled(void) const
{
  return m_vesMaxEnabled;
}

const double & Genex0dSourceRock::getVESMax(void) const
{
  return m_vesMax;
}

bool Genex0dSourceRock::doApplyAdsorption(void) const
{
  return m_applyAdsorption;
}

bool Genex0dSourceRock::adsorptionIsTOCDependent(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

bool Genex0dSourceRock::doComputeOTGC(void) const
{
  return m_doOTCG;
}

const string & Genex0dSourceRock::getAdsorptionCapacityFunctionName(void) const
{
  return m_adsorptionCapacityFunctionName;
}

const string & Genex0dSourceRock::getAdsorptionSimulatorName(void) const
{
  return m_adsorptionSimulatorName;
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
  const  DataAccess::Interface::Grid * grid = m_projectHandle.getActivityOutputGrid();
  if (!grid)
  {
    grid = m_projectHandle.getInputGrid();
  }
  return m_projectHandle.getFactory()->produceGridMap (this, attributeId, grid, mapScalarValue);
}

} // namespace genex0d
