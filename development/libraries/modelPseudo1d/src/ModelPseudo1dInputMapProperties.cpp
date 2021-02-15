//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ModelPseudo1dInputMapProperties.h"

namespace modelPseudo1d
{

ModelPseudo1dInputMapProperties::ModelPseudo1dInputMapProperties() :
  m_tableMapProperty{}
{
  setProperties();
}

ModelPseudo1dInputMapProperties::~ModelPseudo1dInputMapProperties()
{
}

///Changing the Singleton implementation to thread safe type.
const ModelPseudo1dInputMapProperties & ModelPseudo1dInputMapProperties::getInstance()
{
	static ModelPseudo1dInputMapProperties	m_instance;
	return m_instance;
}

void ModelPseudo1dInputMapProperties::setStratIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Depth", "DepthGrid"),
    ModelPseudo1dTableProperty("Thickness", "ThicknessGrid"),
    ModelPseudo1dTableProperty("Percent1", "Percent1Grid"),
    ModelPseudo1dTableProperty("Percent2", "Percent2Grid"),
    ModelPseudo1dTableProperty("SourceRockMixingHI", "SourceRockMixingHIGrid"),
    ModelPseudo1dTableProperty("SourceRockMixingHC", "SourceRockMixingHCGrid")
  };

  m_tableMapProperty["StratIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setMobLayThicknIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Thickness", "ThicknessGrid")
  };

  m_tableMapProperty["MobLayThicknIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setAllochthonLithoDistribIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("", "") // TODO: to obtain the fields
  };

  m_tableMapProperty["AllochthonLithoDistribIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setCrustIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Thickness", "ThicknessGrid")
  };

  m_tableMapProperty["CrustIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setContCrustalThicknessIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Thickness", "ThicknessGrid")
  };

  m_tableMapProperty["ContCrustalThicknessIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setBasaltThicknessIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("BasaltThickness", "BasaltThicknessGrid"),
    ModelPseudo1dTableProperty("CrustThicknessMeltOnset", "CrustThicknessMeltOnsetGrid")
  };

  m_tableMapProperty["BasaltThicknessIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setSourceRockLithoIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("TocIni", "TocIniGrid"),
  };

  m_tableMapProperty["SourceRockLithoIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setCTCIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("TRIni", "TRIniGrid"),
    ModelPseudo1dTableProperty("HCuIni", "HCuIniGrid"),
    ModelPseudo1dTableProperty("HLMuIni", "HLMuIniGrid"),
    ModelPseudo1dTableProperty("DeltaSL", "DeltaSLGrid"),
    ModelPseudo1dTableProperty("HBu", "HBuGrid"),
  };

  m_tableMapProperty["CTCIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setBoundaryValuesIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("PropertyValue", "PropertyValueGrid"),
  };

  m_tableMapProperty["BoundaryValuesIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setSurfaceDepthIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Depth", "DepthGrid"),
  };

  m_tableMapProperty["SurfaceDepthIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setSurfaceTempIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Temperature", "TemperatureGrid"),
  };

  m_tableMapProperty["SurfaceTempIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setMntlHeatFlowIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("HeatFlow", "HeatFlowGrid"),
  };

  m_tableMapProperty["MntlHeatFlowIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setBasementIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("TopCrustHeatProd", "TopCrustHeatProdGrid"),
  };

  m_tableMapProperty["BasementIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setReservoirIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("NetToGross", "NetToGrossGrid"),
  };

  m_tableMapProperty["ReservoirIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setPalinspasticIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Depth", "DepthGrid"), // TODO: to check if the fields are correct
  };

  m_tableMapProperty["PalinspasticIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setTwoWayTimeIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("TwoWayTime", "TwoWayTimeGrid"),
  };

  m_tableMapProperty["TwoWayTimeIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setOceaCrustalThicknessIoTbl()
{
  std::vector<ModelPseudo1dTableProperty> tablePropertyVector = {
    ModelPseudo1dTableProperty("Thickness", "ThicknessGrid"),
  };

  m_tableMapProperty["OceaCrustalThicknessIoTbl"] = tablePropertyVector;
}

void ModelPseudo1dInputMapProperties::setProperties()
{
  setStratIoTbl();
  setMobLayThicknIoTbl();
  setAllochthonLithoDistribIoTbl(); // TODO
  setCrustIoTbl();
  setContCrustalThicknessIoTbl();
  setBasaltThicknessIoTbl();
  setSourceRockLithoIoTbl();
  setCTCIoTbl();
  setBoundaryValuesIoTbl();
  setSurfaceDepthIoTbl();
  setSurfaceTempIoTbl();
  setMntlHeatFlowIoTbl();
  setBasementIoTbl();
  setReservoirIoTbl();
  setPalinspasticIoTbl();
  setTwoWayTimeIoTbl();
  setOceaCrustalThicknessIoTbl();
}

std::vector<ModelPseudo1dTableProperty> ModelPseudo1dInputMapProperties::tableMapPropertyNames(const std::string & tableName) const
{
  if (m_tableMapProperty.count(tableName) == 0)
  {
    return {};
  }

  return m_tableMapProperty.at(tableName);
}

} // namespace modelPseudo1d
