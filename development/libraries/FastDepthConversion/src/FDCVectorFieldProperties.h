//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FDCVectorFieldProperties class caches the variables that are stored as vector fields,
// and handles any functionality related to these variables.
#pragma once

#include "cmbAPI.h"

namespace fastDepthConversion
{

class FDCProjectManager;

class FDCVectorFieldProperties
{
public:
  explicit FDCVectorFieldProperties(FDCProjectManager& fdcProjectManager, const mbapi::StratigraphyManager::SurfaceID referenceSurface);

  void setTopSurfaceProperties(const mbapi::StratigraphyManager::SurfaceID surfaceID, const mbapi::StratigraphyManager::SurfaceID referenceSurface, const std::string & twtMapNames);
  std::vector<double> getMeasuredTwtAtSpecifiedSurface(const mbapi::StratigraphyManager::SurfaceID surface, const std::string & twtMapNames) const;
  std::vector<double> calculateIncreasedDepthsIncludingPreservedErosion(const mbapi::StratigraphyManager::SurfaceID currentSurface, const double maxSeisVel) const;  

  std::vector<double> refDepths() const;
  std::vector<double> newDepths() const;
  void setNewDepths(const std::vector<double> & newDepths);
  std::vector<double> refTwts() const;
  std::vector<double> tarTwts() const;
  void setTarTwts(const std::vector<double> & tarTwts);
  std::vector<double> preservedErosion() const;
  void setPreservedErosion(const std::vector<double> & values);

  double twtconvFactor() const;  

private:
  void calculateTopMostSurfaceIncreasedDepth(std::vector<double> & increasedDepths, const int nRefDepths, const double maxSeisVel) const;
  void calculateSurfaceIncreasedDepth(std::vector<double> & increasedDepths, const int nRefDepths, const double maxSeisVel) const;
  void retrievePropertiesWhenAtReferenceSurface(const mbapi::StratigraphyManager::SurfaceID currentSurface, const std::string & twtMapNames);

  FDCProjectManager&    m_fdcProjectManager;
  std::vector<double>   m_refDepths;
  std::vector<double>   m_newDepths;
  std::vector<double>   m_refTwts;
  std::vector<double>   m_tarTwts;
  std::vector<double>   m_preservedErosion;
  const double          m_twtconvFactor;
};

} // namespace fastDepthConversion
