//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SourceRockTypeNameMappings.h"

namespace Genex6
{

SourceRockTypeNameMappings::SourceRockTypeNameMappings() :
  m_typeNamesSR{},
  m_typeNamesSRWithSulphur{},
  m_CfgFileNameBySRType{}
{
  setPropertyMaps();
  setCfgFileNameBySRType();
}

SourceRockTypeNameMappings::~SourceRockTypeNameMappings()
{
}

const SourceRockTypeNameMappings & SourceRockTypeNameMappings::getInstance()
{
  static SourceRockTypeNameMappings instance;
  return instance;
}

void SourceRockTypeNameMappings::setPropertyMaps()
{
  std::vector<std::string> typeNamesSR;
  std::vector<std::string> typeNamesSRWithSulphur;

  // TypeI
  typeNamesSR = {"LacustrineAlgal",
                 "Type_I_CenoMesozoic_Lacustrine_kin",
                 "Type_I_CenoMesozoic_Lacustrine_Algal_Shale_kin"};
  typeNamesSRWithSulphur = {"Type_I_CenoMesozoic_Lacustrine_kin_s",
                           "Type_I_CenoMesozoic_Lacustrine_Algal_Shale_kin_s"};
  m_typeNamesSR["TypeI"] = typeNamesSR;
  m_typeNamesSRWithSulphur["TypeI"] = typeNamesSRWithSulphur;

  // TypeII
  typeNamesSR = {"MesozoicMarineShale",
                 "Type_II_Mesozoic_MarineShale_kin",
                 "Type_II_Mesozoic_Marine_Shale_kin"};
  typeNamesSRWithSulphur = {"Type_II_Mesozoic_MarineShale_kin_s",
                           "Type_II_Mesozoic_Marine_Shale_kin_s"};
  m_typeNamesSR["TypeII"] = typeNamesSR;
  m_typeNamesSRWithSulphur["TypeII"] = typeNamesSRWithSulphur;

  // TypeI_II
  typeNamesSR = {"Type_I_II_Mesozoic_MarineShale_lit",
                 "Type_I_II_Mesozoic_Marine_Shale_lit"};
  typeNamesSRWithSulphur = {"Type_I_II_Mesozoic_MarineShale_lit_s",
                           "Type_I_II_Mesozoic_Marine_Shale_lit_s"};
  m_typeNamesSR["TypeI_II"] = typeNamesSR;
  m_typeNamesSRWithSulphur["TypeI_II"] = typeNamesSRWithSulphur;

  // TypeIIS
  typeNamesSR = {"MesozoicCalcareousShale",
                 "Type_II_Mesozoic_Marl_kin",
                 "Type_II_Mesozoic_Marine_Marl_kin"};
  typeNamesSRWithSulphur = {"Type_II_Mesozoic_Marl_kin_s",
                           "Type_II_Mesozoic_Marine_Marl_kin_s"};
  m_typeNamesSR["TypeIIS"] = typeNamesSR;
  m_typeNamesSRWithSulphur["TypeIIS"] = typeNamesSRWithSulphur;

  // TypeIIHS
  typeNamesSR = {"PaleozoicMarineShale",
                 "Type_II_Paleozoic_MarineShale_kin",
                 "Type_II_Paleozoic_Marine_Shale_kin"};
  typeNamesSRWithSulphur = {"Type_II_Paleozoic_MarineShale_kin_s",
                           "Type_II_Paleozoic_Marine_Shale_kin_s"};
  m_typeNamesSR["TypeIIHS"] = typeNamesSR;
  m_typeNamesSRWithSulphur["TypeIIHS"] = typeNamesSRWithSulphur;

  // TypeII_III
  typeNamesSR = {"Type_III_II_Mesozoic_HumicCoal_lit",
                 "Type_III_II_Mesozoic_Terrestrial_Humic_Coal_lit"};
  typeNamesSRWithSulphur = {"Type_III_II_Mesozoic_HumicCoal_lit_s",
                           "Type_II_III_Mesozoic_Terrestrial_Humic_Coal_lit_s"};
  m_typeNamesSR["TypeII_III"] = typeNamesSR;
  m_typeNamesSRWithSulphur["TypeII_III"] = typeNamesSRWithSulphur;

  // TypeIII
  typeNamesSR = {"VitriniticCoals",
                 "Type_III_MesoPaleozoic_VitriniticCoal_kin",
                 "Type_III_Paleozoic_Terrestrial_Vitrinitic_Coals_kin"
                /*"Type_III_MesoPaleozoic_Terrestrial_Vitrinitic_Coals_kin"*/};
  typeNamesSRWithSulphur = {};
  m_typeNamesSR["TypeIII"] = typeNamesSR;
  m_typeNamesSRWithSulphur["TypeIII"] = typeNamesSRWithSulphur;
}

void SourceRockTypeNameMappings::setCfgFileNameBySRType()
{
  for (auto itSRMapping : m_typeNamesSR)
  {
    for (auto itSRname : itSRMapping.second)
    {
      m_CfgFileNameBySRType[itSRname] = itSRMapping.first;
    }
  }

  for (auto itSRMapping : m_typeNamesSRWithSulphur)
  {
    for (auto itSRname : itSRMapping.second)
    {
      m_CfgFileNameBySRType[itSRname] = itSRMapping.first;
    }
  }
}

void SourceRockTypeNameMappings::typeNamesSRWithSulphur(std::vector<std::string> & typeNameList, const std::string & srCfgFileTypeName) const
{
  if (m_typeNamesSRWithSulphur.count(srCfgFileTypeName) == 0)
  {
    return;
  }

  typeNameList = m_typeNamesSRWithSulphur.at(srCfgFileTypeName);
}

void SourceRockTypeNameMappings::typeNamesSR(std::vector<std::string> & typeNameList, const std::string & srCfgFileTypeName) const
{
  if (m_typeNamesSR.count(srCfgFileTypeName) == 0)
  {
    return;
  }

  typeNameList = m_typeNamesSR.at(srCfgFileTypeName);
}

const std::unordered_map<std::string, std::string> & SourceRockTypeNameMappings::CfgFileNameBySRType() const
{
  return m_CfgFileNameBySRType;
}

} // namespace Genex6
