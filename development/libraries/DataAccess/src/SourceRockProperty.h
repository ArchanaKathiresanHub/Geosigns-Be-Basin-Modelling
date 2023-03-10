//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// SourceRockProperty class, which contains source rock property variables.
// Note: all the variables contain values that should be obtained for Vr = 0.5!

#pragma once

#include <string>

namespace DataAccess
{
namespace Interface
{

class SourceRockProperty
{
public:
  SourceRockProperty();

  double TocIni() const;
  void setTocIni(const double TocIni);

  const double & SCVRe05() const;
  void setSCVRe05(const double SCVRe05);

  const double & activationEnergy() const;
  void setActivationEnergy(const double activationEnergy);

  const double & HCVRe05() const;
  void setHCVRe05(const double HCVRe05);

  const double & Vr() const;

  const double & AsphalteneDiffusionEnergy() const;
  void setAsphalteneDiffusionEnergy(const double AsphalteneDiffusionEnergy);

  const double & ResinDiffusionEnergy() const;
  void setResinDiffusionEnergy(const double ResinDiffusionEnergy);

  const double & C15AroDiffusionEnergy() const;
  void setC15AroDiffusionEnergy(const double C15AroDiffusionEnergy);

  const double & C15SatDiffusionEnergy() const;
  void setC15SatDiffusionEnergy(const double C15SatDiffusionEnergy);

  const std::string & typeNameID() const;
  void setTypeNameID(const std::string & typeNameID);

private:
  double m_Vr; // Vr always has constant value of 0.5
  double m_TocIni;
  double m_SCVRe05;
  double m_activationEnergy; // Note: activationEnergy or Emean or Eact = 1000.0*preAsphaltStartAct
  double m_HCVRe05;
  double m_AsphalteneDiffusionEnergy;
  double m_ResinDiffusionEnergy;
  double m_C15AroDiffusionEnergy;
  double m_C15SatDiffusionEnergy;

  std::string m_typeNameID;
};

} // namespace DataAccess
} // namespace Interface
