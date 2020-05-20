//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "cmbAPI.h"

#include "ModelPseudo1dProjectManager.h"
#include "ModelPseudo1dTableProperty.h"

#include "MapsManager.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mbapi
{
class Model;
} // namespace mbapi

namespace modelPseudo1d
{


class ModelPseudo1d
{
  typedef std::unordered_set<std::string> UnorderedStringSet;
  typedef std::pair<int, ModelPseudo1dTableProperty> PairModelPseudo1dTableProperty;
  typedef std::unordered_map<std::string, std::vector<PairModelPseudo1dTableProperty>> TablePropertyMapList;
  typedef std::unordered_map<std::string, mbapi::MapsManager::MapID> MapIDsHashTable;
public:  

  explicit ModelPseudo1d( mbapi::Model& model, const double xCoord , const double yCoord );

  void initialize();
  void extractScalarsFromInputMaps();
  void setScalarsInModel();
  void setSingleCellWindowXY();

  unsigned int indI() const;
  unsigned int indJ() const;

private:
  double extractScalarFromInputMaps(const ModelPseudo1dTableProperty & tableProperty, const std::string & tableName, const int row, const MapIDsHashTable & mapNameIDs);

  void checkSubSampling();
  void getXYIndices();
  void setReferredTablesSet();

  mbapi::Model& m_mdl;
  const double m_xCoord;
  const double m_yCoord;
  unsigned int m_indI;
  unsigned int m_indJ;
  std::vector<TablePropertyMapList> m_tablePropertyMapsLists;

  UnorderedStringSet m_referredTablesSet;
};

} // namespace modelPseudo1d
