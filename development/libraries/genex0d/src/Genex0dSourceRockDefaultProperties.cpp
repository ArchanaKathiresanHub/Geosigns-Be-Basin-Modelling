//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSourceRockDefaultProperties.h"

#include <memory>

namespace genex0d
{

Genex0dSourceRockDefaultProperties::Genex0dSourceRockDefaultProperties() :
  m_properties{}
{
  setPropertyMaps();
}

Genex0dSourceRockDefaultProperties::~Genex0dSourceRockDefaultProperties()
{
}

///Changing the Singleton implementation to thread safe type.
const Genex0dSourceRockDefaultProperties & Genex0dSourceRockDefaultProperties::getInstance()
{
	static Genex0dSourceRockDefaultProperties	m_instance;
	return m_instance;
}

void Genex0dSourceRockDefaultProperties::setPropertyMap(const std::string & sourceRockType,
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
                                                        const double SCVRe05WithSulphur)
{
  std::unique_ptr<Genex0dSourceRockProperty> property(new Genex0dSourceRockProperty());

  // without Sulphur
  property->setTocIni(TocIni);
  property->setHCVRe05(HCVRe05);
  property->setSCVRe05(SCVRe05);
  property->setActivationEnergy(ActivationEnergy);
  property->setAsphalteneDiffusionEnergy(AsphalteneDiffusionEnergy);
  property->setResinDiffusionEnergy(ResinDiffusionEnergy);
  property->setC15AroDiffusionEnergy(C15AroDiffusionEnergy);
  property->setC15SatDiffusionEnergy(C15SatDiffusionEnergy);
  property->setTypeNameID(sourceRockType);
  for (const std::string & nameNoSulphur : nameListNoSulphur)
  {
    m_properties[nameNoSulphur] = *property;
  }

  // with Sulphur
  if (nameWithSulphur.empty())
  {
    return;
  }
  property->setHCVRe05(HCVRe05WithSulphur);
  property->setSCVRe05(SCVRe05WithSulphur);
  m_properties[nameWithSulphur] = *property;
}

void Genex0dSourceRockDefaultProperties::setPropertyMaps()
{
  setPropertyMap("TypeI", {"Type_I_CenoMesozoic_Lacustrine_kin", "LacustrineAlgal", "Type I - Lacustrine"}, "Type_I_CenoMesozoic_Lacustrine_kin_s",
                 10.0, 1.45, 0.0, 214000.0, 90000.0, 83000.0, 81000.0, 72000.0,
                 1.56, 0.02);
  
  setPropertyMap("TypeII", {"Type_II_Mesozoic_MarineShale_kin", "Type II - Mesozoic Marine", "MesozoicMarineShale"}, "Type_II_Mesozoic_MarineShale_kin_s",
                 10.0, 1.25, 0.0, 210000.0, 88000.0, 85000.0, 80000.0, 75000.0,
                 1.24, 0.035);

  setPropertyMap("TypeI_II", {"Type_I_II_Mesozoic_MarineShale_lit", "Type I/II - Marine Shale"}, "Type_I_II_Mesozoic_MarineShale_lit_s",
                 4.0, 1.35, 0.0, 211000.0, 90000.0, 87000.0, 80000.0, 74000.0,
                 1.35, 0.02);
  
  setPropertyMap("TypeIIS", {"Type_II_Mesozoic_Marl_kin", "Type II - Marine Marl", "MesozoicCalcareousShale"}, "Type_II_Mesozoic_Marl_kin_s",
                 10.0, 1.25, 0.0, 211000.0, 88000.0, 85000.0, 74000.0, 69000.0,
                 1.24, 0.05);
  
  setPropertyMap("TypeIIHS", {"Type_II_Paleozoic_MarineShale_kin", "Type II - Paleozoic Marine", "PaleozoicMarineShale"}, "Type_II_Paleozoic_MarineShale_kin_s",
                 10.0, 1.2, 0.0, 212000.0, 87000.0, 80000.0, 77000.0, 71000.0,
                 1.13, 0.045);
  
  setPropertyMap("TypeII_III", {"Type_III_II_Mesozoic_HumicCoal_lit", "Type II/III - Humic Coal"}, "Type_III_II_Mesozoic_HumicCoal_lit_s",
                 20.0, 1.03, 0.0, 208000.0, 90000.0, 87000.0, 81000.0, 76000.0,
                 1.03, 0.02);
  
  setPropertyMap("TypeIII", {"Type_III_MesoPaleozoic_VitriniticCoal_kin", "Type III - Terrestrial Coal", "VitriniticCoals"}, "",
                 10.0, 0.801, 0.0, 206000.0, 90000.0, 87000.0, 84000.0, 81000.0,
                 0.0, 0.0);
}

Genex0dSourceRockProperty Genex0dSourceRockDefaultProperties::getProperties(const std::string & typeName) const
{
  if (m_properties.count(typeName) == 0)
  {
    return Genex0dSourceRockProperty();
  }
  
  return m_properties.at(typeName);
}

} // namespace genex0d
