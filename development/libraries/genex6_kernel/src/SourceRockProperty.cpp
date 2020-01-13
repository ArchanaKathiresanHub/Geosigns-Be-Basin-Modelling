//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SourceRockProperty.h"

namespace Genex6
{

SourceRockProperty::SourceRockProperty() :
  m_Vr{0.5},
  m_TocIni{0.0},
  m_SCVRe05{0.0},
  m_activationEnergy{0.0},
  m_HCVRe05{0.0},
  m_AsphalteneDiffusionEnergy{0.0},
  m_ResinDiffusionEnergy{0.0},
  m_C15AroDiffusionEnergy{0.0},
  m_C15SatDiffusionEnergy{0.0},
  m_typeNameID{""}
{
}

double SourceRockProperty::TocIni() const
{
  return m_TocIni;
}

void SourceRockProperty::setTocIni(const double TocIni)
{
  m_TocIni = TocIni;
}

const double & SourceRockProperty::SCVRe05() const
{
  return m_SCVRe05;
}

void SourceRockProperty::setSCVRe05(const double SCVRe05)
{
  m_SCVRe05 = SCVRe05;
}

const double & SourceRockProperty::activationEnergy() const
{
  return m_activationEnergy;
}

void SourceRockProperty::setActivationEnergy(const double activationEnergy)
{
  m_activationEnergy = activationEnergy;
}

const double & SourceRockProperty::HCVRe05() const
{
  return m_HCVRe05;
}

void SourceRockProperty::setHCVRe05(const double HCVRe05)
{
  m_HCVRe05 = HCVRe05;
}

const double & SourceRockProperty::Vr() const
{
  return m_Vr;
}

const double & SourceRockProperty::AsphalteneDiffusionEnergy() const
{
  return m_AsphalteneDiffusionEnergy;
}

void SourceRockProperty::setAsphalteneDiffusionEnergy(const double AsphalteneDiffusionEnergy)
{
  m_AsphalteneDiffusionEnergy = AsphalteneDiffusionEnergy;
}

const double & SourceRockProperty::ResinDiffusionEnergy() const
{
  return m_ResinDiffusionEnergy;
}

void SourceRockProperty::setResinDiffusionEnergy(const double ResinDiffusionEnergy)
{
  m_ResinDiffusionEnergy = ResinDiffusionEnergy;
}

const double & SourceRockProperty::C15AroDiffusionEnergy() const
{
  return m_C15AroDiffusionEnergy;
}

void SourceRockProperty::setC15AroDiffusionEnergy(const double C15AroDiffusionEnergy)
{
  m_C15AroDiffusionEnergy = C15AroDiffusionEnergy;
}

const double & SourceRockProperty::C15SatDiffusionEnergy() const
{
  return m_C15SatDiffusionEnergy;
}

void SourceRockProperty::setC15SatDiffusionEnergy(const double C15SatDiffusionEnergy)
{
  m_C15SatDiffusionEnergy = C15SatDiffusionEnergy;
}

const std::string & SourceRockProperty::typeNameID() const
{
  return m_typeNameID;
}

void SourceRockProperty::setTypeNameID(const std::string & typeNameID)
{
  m_typeNameID = typeNameID;
}

} // namespace Genex6
