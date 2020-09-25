//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SourceRockDefaultProperties.h"

#include "SourceRockTypeNameMappings.h"

#include <memory>

namespace Genex6
{

SourceRockDefaultProperties::SourceRockDefaultProperties() :
  m_properties{}
{
  setPropertyMaps();
}

SourceRockDefaultProperties::~SourceRockDefaultProperties()
{
}

const SourceRockDefaultProperties & SourceRockDefaultProperties::getInstance()
{
  static SourceRockDefaultProperties instance;
  return instance;
}

void SourceRockDefaultProperties::setPropertyMap(const std::string & srCfgFileTypeName,
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
  std::unique_ptr<DataAccess::Interface::SourceRockProperty> property(new DataAccess::Interface::SourceRockProperty());

  // without Sulphur
  property->setTocIni(TocIni);
  property->setHCVRe05(HCVRe05);
  property->setSCVRe05(SCVRe05);
  property->setActivationEnergy(ActivationEnergy);
  property->setAsphalteneDiffusionEnergy(AsphalteneDiffusionEnergy);
  property->setResinDiffusionEnergy(ResinDiffusionEnergy);
  property->setC15AroDiffusionEnergy(C15AroDiffusionEnergy);
  property->setC15SatDiffusionEnergy(C15SatDiffusionEnergy);
  property->setTypeNameID(srCfgFileTypeName);

  typedef const SourceRockTypeNameMappings & (*(SRTypeNameMappingsFunc))();
  SRTypeNameMappingsFunc srTypeNameMappingFunc = SourceRockTypeNameMappings::getInstance;

  std::vector<std::string> typeNameList;
  (*srTypeNameMappingFunc)().typeNamesSR(typeNameList, srCfgFileTypeName);
  for (const std::string & nameNoSulphur : typeNameList)
  {
    m_properties[nameNoSulphur] = *property;
  }

  // with Sulphur
  typeNameList.clear();
  (*srTypeNameMappingFunc)().typeNamesSRWithSulphur(typeNameList, srCfgFileTypeName);
  if (typeNameList.empty())
  {
    return;
  }
  property->setHCVRe05(HCVRe05WithSulphur);
  property->setSCVRe05(SCVRe05WithSulphur);

  for (const std::string & nameWithSulphur : typeNameList)
  {
    m_properties[nameWithSulphur] = *property;
  }
}

void SourceRockDefaultProperties::setPropertyMaps()
{
  // TypeI
  setPropertyMap("TypeI", 10.0, 1.45, 0.0, 214000.0, 90000.0, 83000.0, 81000.0, 72000.0, 1.56, 0.02);
  
  // TypeII
  setPropertyMap("TypeII", 10.0, 1.25, 0.0, 210000.0, 88000.0, 85000.0, 80000.0, 75000.0, 1.24, 0.035);

  // TypeI_II
  setPropertyMap("TypeI_II", 4.0, 1.35, 0.0, 211000.0, 90000.0, 87000.0, 80000.0, 74000.0, 1.35, 0.02);
  
  // TypeIIS
  setPropertyMap("TypeIIS", 10.0, 1.25, 0.0, 211000.0, 88000.0, 85000.0, 74000.0, 69000.0, 1.24, 0.05);

  // TypeIIHS
  setPropertyMap("TypeIIHS", 10.0, 1.2, 0.0, 212000.0, 87000.0, 80000.0, 77000.0, 71000.0, 1.13, 0.045);

  // TypeII_III
  setPropertyMap("TypeII_III", 20.0, 1.03, 0.0, 208000.0, 90000.0, 87000.0, 81000.0, 76000.0, 1.03, 0.02);
  
  // TypeIII
  setPropertyMap("TypeIII", 10.0, 0.801, 0.0, 206000.0, 90000.0, 87000.0, 84000.0, 81000.0, 0.0, 0.0);
}

DataAccess::Interface::SourceRockProperty SourceRockDefaultProperties::getProperties(const std::string & typeName) const
{
  if (m_properties.count(typeName) == 0)
  {
    return DataAccess::Interface::SourceRockProperty();
  }
  
  return m_properties.at(typeName);
}

} // namespace Genex6
