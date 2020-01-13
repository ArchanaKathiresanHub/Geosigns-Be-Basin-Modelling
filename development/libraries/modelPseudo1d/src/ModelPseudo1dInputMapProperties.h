//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "ModelPseudo1dTableProperty.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace modelPseudo1d
{

class ModelPseudo1dInputMapProperties
{
public:
  ModelPseudo1dInputMapProperties(const ModelPseudo1dInputMapProperties &) = delete;
  ModelPseudo1dInputMapProperties & operator = (const ModelPseudo1dInputMapProperties &) = delete;
  ModelPseudo1dInputMapProperties(const ModelPseudo1dInputMapProperties &&) = delete;
  ModelPseudo1dInputMapProperties & operator = (const ModelPseudo1dInputMapProperties &&) = delete;

  static const ModelPseudo1dInputMapProperties & getInstance();
  void setProperties();
  std::vector<ModelPseudo1dTableProperty> tableMapPropertyNames(const std::string & tableName) const;

private:
  typedef std::unordered_map<std::string, std::vector<ModelPseudo1dTableProperty>> TypeTablePropertyMap;

  ModelPseudo1dInputMapProperties();
  ~ModelPseudo1dInputMapProperties();

  void setStratIoTbl();
  void setMobLayThicknIoTbl();
  void setAllochthonLithoDistribIoTbl();
  void setCrustIoTbl();
  void setContCrustalThicknessIoTbl();
  void setBasaltThicknessIoTbl();
  void setSourceRockLithoIoTbl();
  void setCTCIoTbl();
  void setBoundaryValuesIoTbl();
  void setSurfaceTempIoTbl();
  void setMntlHeatFlowIoTbl();
  void setBasementIoTbl();
  void setReservoirIoTbl();
  void setPalinspasticIoTbl();
  void setTwoWayTimeIoTbl();
  void setOceaCrustalThicknessIoTbl();

  TypeTablePropertyMap m_tableMapProperty;
};

} // namespace modelPseudo1d
