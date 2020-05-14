//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

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

struct ModelPseudo1dInputData;
class ModelPseudo1dProjectManager;

class ModelPseudo1d
{
  typedef std::pair<int, ModelPseudo1dTableProperty> PairModelPseudo1dTableProperty;
  typedef std::unordered_map<std::string, std::vector<PairModelPseudo1dTableProperty>> TablePropertyMapList;
  typedef std::unordered_map<std::string, mbapi::MapsManager::MapID> MapIDsHashTable;
public:
  explicit ModelPseudo1d( modelPseudo1d::ModelPseudo1dInputData& inputData );

  void initialize();
  void extractScalarsFromInputMaps();
  void setScalarsInModel();
  void finalize();

private:
  double extractScalarFromInputMaps(const ModelPseudo1dTableProperty & tableProperty, const std::string & tableName, const int row, const MapIDsHashTable & mapNameIDs);

  ModelPseudo1dProjectManager m_projectMgr;
  mbapi::Model& m_mdl;
  std::vector<TablePropertyMapList> m_tablePropertyMapsLists;

  std::string m_outProjectFileName;
};

} // namespace modelPseudo1d
