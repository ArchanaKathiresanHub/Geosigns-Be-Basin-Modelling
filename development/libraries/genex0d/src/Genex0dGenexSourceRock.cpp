//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dGenexSourceRock.h"

#include "CommonDefinitions.h"
#include "Genex0dSourceRockProperty.h"

#include "ConstantsGenex.h"
#include "Simulator.h"
#include "SourceRockNode.h"

#include "Interface.h"

namespace genex0d
{

Genex0dGenexSourceRock::Genex0dGenexSourceRock (const Genex0dSourceRockProperty& srProperties,
                                                const std::string & formationName,
                                                const std::string & sourceRockType) :
  m_srProperties{srProperties},
  m_formationName{formationName},
  m_sourceRockType{sourceRockType},
  m_vreThreshold{0.5}, // TODO: check if this value can be obtained from input
  m_vesMax{20} // TODO: check if this value can be obtained from input
{
}

const std::string & Genex0dGenexSourceRock::getLayerName (void) const
{
  return m_formationName;
}

const std::string & Genex0dGenexSourceRock::getType (void) const final
{
  return m_sourceRockType;
}

const double & Genex0dGenexSourceRock::getHcVRe05(void) const
{
  return m_srProperties.HCVRe05;
}

const double & Genex0dGenexSourceRock::getScVRe05(void) const
{
  return m_srProperties.SCVRe05;
}

const double & Genex0dGenexSourceRock::getPreAsphaltStartAct(void) const
{
  return m_srProperties.activationEnergy();
}

const double & Genex0dGenexSourceRock::getAsphalteneDiffusionEnergy(void) const
{
  return m_srProperties.AsphalteneDiffusionEnergy();
}

const double & Genex0dGenexSourceRock::getResinDiffusionEnergy(void) const
{
  return m_srProperties.ResinDiffusionEnergy();
}

const double & Genex0dGenexSourceRock::getC15AroDiffusionEnergy(void) const
{
  return m_srProperties.C15AroDiffusionEnergy();
}

const double & Genex0dGenexSourceRock::getC15SatDiffusionEnergy(void) const
{
  return m_srProperties.C15SatDiffusionEnergy();
}

bool Genex0dGenexSourceRock::isVREoptimEnabled(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

const double & Genex0dGenexSourceRock::getVREthreshold(void) const
{
  return m_vreThreshold;
}

const std::string & Genex0dGenexSourceRock::getBaseSourceRockType (void) const
{
  return m_sourceRockType; // TODO: Check if it is different from SR type. If so can it be obtained from input?
}

bool Genex0dGenexSourceRock::isVESMaxEnabled(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

const double & getVESMax(void) const
{
  return m_vesMax;
}

} // namespace genex0d
