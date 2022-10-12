//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FDCMapFieldProperties class caches the variables that are stored as map fields,
// and handles any functionality related to these variables.
#pragma once

#include "cmbAPI.h"

#include <string>

namespace fastDepthConversion
{

class FDCProjectManager;

class FDCMapFieldProperties
{
public:
  explicit FDCMapFieldProperties(FDCProjectManager& fdcProjectManager, const mbapi::StratigraphyManager::SurfaceID referenceSurface, const mbapi::StratigraphyManager::SurfaceID endSurface);

  void setEndSurface(const mbapi::StratigraphyManager::SurfaceID endSurface);

  void calculateInitialMaps(const std::string & masterResultsFilePathName, const bool preserveErosionFlag);

  /// @brief Create the missing TWT map from existing maps and depth maps.
  /// @param surfID the mbapi::StratigraphyManager::SurfaceID defining the surface
  void setMissingTwtForSurface(const mbapi::StratigraphyManager::SurfaceID surfID);

  std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> correctedMapsNames() const;
  std::string correctedMapsName(const mbapi::StratigraphyManager::SurfaceID ID) const;
  void setCorrectedMapsNames(const mbapi::StratigraphyManager::SurfaceID ID, const std::string& value);
  std::map<const mbapi::StratigraphyManager::SurfaceID, int> correctedMapsSequenceNbr() const;
  void setCorrectedMapsSequenceNbr(const mbapi::StratigraphyManager::SurfaceID ID, const int value);
  std::vector<int> addedTwtmapsequenceNbr() const;
  void setAddedTwtmapsequenceNbr(const int number);

  std::map<const mbapi::StratigraphyManager::SurfaceID, std::vector<double>> isoPacks() const;
  void setIsoPacks(const mbapi::StratigraphyManager::SurfaceID ID, const std::vector<double> & value);
  std::string twtMaps(const mbapi::StratigraphyManager::SurfaceID ID) const;
  std::vector<double> missingTwtMaps(const mbapi::StratigraphyManager::SurfaceID ID) const;
  void setMissingTwtMaps(const mbapi::StratigraphyManager::SurfaceID ID, const std::vector<double>& value);
  std::map<const mbapi::StratigraphyManager::SurfaceID, int> hiatusAll() const;
  int hiatusID(const mbapi::StratigraphyManager::SurfaceID ID) const;

  void setPropertyHistoryMaps(const mbapi::StratigraphyManager::LayerID currentLayer,
                              const std::vector<double> & refDepths,
                              const std::vector<double> & refTwts);

  std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double> > refDepthsHistory() const;
  std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double> > refTwtHistory() const;  

private:
  void initializeAllMaps();
  void setHiatus(const mbapi::StratigraphyManager::SurfaceID ID, const int value);  
  void setTwtMaps(const mbapi::StratigraphyManager::SurfaceID ID, const std::string& value);
  void setTwtMapsForAllSurfaces();
  void calculateIsoPackThicknessForSurfacesBelowEndSurface(const bool preserveErosionFlag);  
  void initializeMapWriterOnMasterResultsFile(const std::string & masterResultsFilePathName) const;
  std::string getTwtGridName(const mbapi::StratigraphyManager::SurfaceID surface, mbapi::StratigraphyManager& stMgr);
  void detectHiatusForAllSurfaces();  
  std::vector<double> getReferenceTwt(const mbapi::StratigraphyManager::SurfaceID s) const;
  std::vector<double> getNextTwt(const mbapi::StratigraphyManager::SurfaceID s) const;  
  mbapi::StratigraphyManager::SurfaceID getIDofFirstNonMissingTwtSurface(const mbapi::StratigraphyManager::SurfaceID surfID) const;
  mbapi::StratigraphyManager::SurfaceID getIDofLastExistingTwtSurface(const mbapi::StratigraphyManager::SurfaceID surfID) const;

  FDCProjectManager& m_fdcProjectManager;
  const mbapi::StratigraphyManager::SurfaceID m_referenceSurface;
  mbapi::StratigraphyManager::SurfaceID m_endSurface;

  // Data structures for the calibrated maps
  std::map<const mbapi::StratigraphyManager::SurfaceID, std::string>         m_correctedMapsNames;
  std::map<const mbapi::StratigraphyManager::SurfaceID, std::string>         m_twtMaps;
  std::map<const mbapi::StratigraphyManager::SurfaceID, std::vector<double>> m_isoPacks;
  std::map<const mbapi::StratigraphyManager::SurfaceID, std::vector<double>> m_missingTwtMaps;
  std::map<const mbapi::StratigraphyManager::SurfaceID, int>                 m_correctedMapsSequenceNbr;
  std::map<const mbapi::StratigraphyManager::SurfaceID, int>                 m_hiatus;
  std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double>>   m_refDepthsHistory;
  std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double>>   m_refTwtHistory;

  std::vector<int>                                                     m_addedTwtmapsequenceNbr;
};

} // namespace fastDepthConversion
