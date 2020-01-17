//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// SourceRockTypeNameMappings class: maps names of different source rock types to their corresponding config file names.
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace Genex6
{

class SourceRockTypeNameMappings
{
public:
  SourceRockTypeNameMappings(const SourceRockTypeNameMappings &) = delete;
  SourceRockTypeNameMappings & operator = (const SourceRockTypeNameMappings &) = delete;
  SourceRockTypeNameMappings(const SourceRockTypeNameMappings &&) = delete;
  SourceRockTypeNameMappings & operator = (const SourceRockTypeNameMappings &&) = delete;

  static const SourceRockTypeNameMappings & getInstance();
  const std::unordered_map<std::string, std::string> & CfgFileNameBySRType() const;
  void typeNamesSR(std::vector<std::string> & typeNameList, const std::string & srCfgFileTypeName) const;
  void typeNamesSRWithSulphur(std::vector<std::string> & typeNameList, const std::string & srCfgFileTypeName) const;

private:
  SourceRockTypeNameMappings();
  ~SourceRockTypeNameMappings();

  void setPropertyMaps();
  void setCfgFileNameBySRType();

  std::unordered_map<std::string, std::vector<std::string>> m_typeNamesSR;
  std::unordered_map<std::string, std::vector<std::string>> m_typeNamesSRWithSulphur;
  std::unordered_map<std::string, std::string> m_CfgFileNameBySRType;
};

} // namespace Genex6
