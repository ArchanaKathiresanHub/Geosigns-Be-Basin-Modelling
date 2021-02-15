//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dSourceRock class gets source rock (SR) input data, and computes SR properties.

#pragma once

#include "Genex0dInputData.h"
#include "GenexBaseSourceRock.h"
#include "SourceRock.h"

namespace DataAccess
{
namespace Interface
{
class GridMap;
}
}

namespace Genex0d
{

class Genex0dPointAdsorptionHistory;

class Genex0dSourceRock : public Genex6::GenexBaseSourceRock
{
public:
  Genex0dSourceRock(DataAccess::Interface::ProjectHandle& projectHandle, const Genex0dInputData& inData, const unsigned int indI, const unsigned int indJ);
  virtual ~Genex0dSourceRock();

  const std::string & getLayerName (void) const final;
  const std::string & getType (void) const final;
  const double & getHcVRe05(void) const final;
  const double & getScVRe05(void) const final;
  const double & getPreAsphaltStartAct(void) const final;
  const double & getAsphalteneDiffusionEnergy(void) const final;
  const double & getResinDiffusionEnergy(void) const final;
  const double & getC15AroDiffusionEnergy(void) const final;
  const double & getC15SatDiffusionEnergy(void) const final;
  bool isVREoptimEnabled(void) const final;
  const double & getVREthreshold(void) const final;
  const std::string & getBaseSourceRockType(void) const final;
  bool isVESMaxEnabled(void) const final;
  const double & getVESMax(void) const final;
  bool doApplyAdsorption(void) const final;
  bool adsorptionIsTOCDependent(void) const final;
  bool doComputeOTGC(void) const final;
  const string & getAdsorptionCapacityFunctionName(void) const final;
  const string & getAdsorptionSimulatorName(void) const final;
  const DataAccess::Interface::GridMap * getMap(DataAccess::Interface::SourceRockMapAttributeId attributeId) const final;
  const std::string& getTypeID() const;


  void initializeInputs(const double thickness, const double inorganicDensity, const std::vector<double>& time,
                              const std::vector<double>& temperature, const std::vector<double>& Ves, const std::vector<double>& VRE, const std::vector<double>& porePressure, const std::vector<double>& permeability,
                              const std::vector<double>& porosity);

  const Genex6::SourceRockNode& getSourceRockNode() const;
  const Genex6::Simulator& simulator() const;

  bool initialize(const bool printInitialisationDetails = true) final;
  bool addHistoryToNodes() final;
  bool setFormationData (const DataAccess::Interface::Formation * aFormation) final;
  bool preprocess() final;
  bool process() final;

private:
  Genex0dSourceRock(DataAccess::Interface::ProjectHandle& projectHandle, const DataAccess::Interface::SourceRockProperty& sourceRockProperties, const Genex0dInputData& inData);
  std::string extractTypeID(const string& sourceRockType) const;

  void setSourceRockPropertiesFromInput(const Genex0dInputData& inData);
  const GridMap * loadMap (DataAccess::Interface::SourceRockMapAttributeId attributeId, const double mapScalarValue) const;
  bool computePTSnapShot(const double timePrevious, const double time, double inPressure, const double inTemperaturePrevious, const double inTemperature, const double inVre, const double inPorePressurePrevious, const double inPorePressure, const double inPermeability, const double inPorosity, const bool isInterpolatedTime);

  const double m_vreThreshold;
  const double m_vesMax;
  const bool m_vesMaxEnabled;
  const bool m_applyAdsorption;
  const bool m_doOTCG;
  const std::string m_adsorptionCapacityFunctionName;
  const std::string m_adsorptionSimulatorName;

  std::unique_ptr<Genex6::SourceRockNode> m_sourceRockNode;
  double m_thickness;
  const unsigned int m_indI;
  const unsigned int m_indJ;

  std::vector<double> m_inTimes;
  std::vector<double> m_inTemperatures;
  std::vector<double> m_inVesAll;
  std::vector<double> m_inVRE;
  std::vector<double> m_inPorePressure;
  std::vector<double> m_inPorosity;
  std::vector<double> m_inPermeability;
  double m_inorganicDensity;

  const std::string m_formationName;
  const std::string m_sourceRockType;
  std::unique_ptr<Genex0dPointAdsorptionHistory> m_pointAdsorptionHistory;
  std::unique_ptr<Genex0dSourceRock> m_sourceRock2;
  double m_mixingHI;
};

} // namespace genex0d
