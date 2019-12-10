//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSourceRockProperty.h"

namespace genex0d
{

Genex0dSourceRockProperty::Genex0dSourceRockProperty() :
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

double Genex0dSourceRockProperty::TocIni() const
{
  return m_TocIni;
}

void Genex0dSourceRockProperty::setTocIni(const double TocIni)
{
  m_TocIni = TocIni;
}

const double & Genex0dSourceRockProperty::SCVRe05() const
{
  return m_SCVRe05;
}

void Genex0dSourceRockProperty::setSCVRe05(const double SCVRe05)
{
  m_SCVRe05 = SCVRe05;
}

const double & Genex0dSourceRockProperty::activationEnergy() const
{
  return m_activationEnergy;
}

void Genex0dSourceRockProperty::setActivationEnergy(const double activationEnergy)
{
  m_activationEnergy = activationEnergy;
}

const double & Genex0dSourceRockProperty::HCVRe05() const
{
  return m_HCVRe05;
}

void Genex0dSourceRockProperty::setHCVRe05(const double HCVRe05)
{
  m_HCVRe05 = HCVRe05;
}

const double & Genex0dSourceRockProperty::Vr() const
{
  return m_Vr;
}

const double & Genex0dSourceRockProperty::AsphalteneDiffusionEnergy() const
{
  return m_AsphalteneDiffusionEnergy;
}

void Genex0dSourceRockProperty::setAsphalteneDiffusionEnergy(const double AsphalteneDiffusionEnergy)
{
  m_AsphalteneDiffusionEnergy = AsphalteneDiffusionEnergy;
}

const double & Genex0dSourceRockProperty::ResinDiffusionEnergy() const
{
  return m_ResinDiffusionEnergy;
}

void Genex0dSourceRockProperty::setResinDiffusionEnergy(const double ResinDiffusionEnergy)
{
  m_ResinDiffusionEnergy = ResinDiffusionEnergy;
}

const double & Genex0dSourceRockProperty::C15AroDiffusionEnergy() const
{
  return m_C15AroDiffusionEnergy;
}

void Genex0dSourceRockProperty::setC15AroDiffusionEnergy(const double C15AroDiffusionEnergy)
{
  m_C15AroDiffusionEnergy = C15AroDiffusionEnergy;
}

const double & Genex0dSourceRockProperty::C15SatDiffusionEnergy() const
{
  return m_C15SatDiffusionEnergy;
}

void Genex0dSourceRockProperty::setC15SatDiffusionEnergy(const double C15SatDiffusionEnergy)
{
  m_C15SatDiffusionEnergy = C15SatDiffusionEnergy;
}

const std::string & Genex0dSourceRockProperty::typeNameID() const
{
  return m_typeNameID;
}

void Genex0dSourceRockProperty::setTypeNameID(const std::string & typeNameID)
{
  m_typeNameID = typeNameID;
}

} // namespace genex0d
