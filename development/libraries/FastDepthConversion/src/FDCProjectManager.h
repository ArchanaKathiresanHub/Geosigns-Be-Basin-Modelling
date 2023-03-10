//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FDCProjectManager class handles any functionality related to model, and read/write of project3d file.
#pragma once

#include "cmbAPI.h"

#include "ErrorHandler.h"

#include <map>
#include <vector>

namespace fastDepthConversion
{

class FDCProjectManager
{
public:
  explicit FDCProjectManager(const std::string & projectFileName);

  /// @brief Clean specific project tables before FastDepthConversion
  void prepareProject();

  /// @brief Modify the StratIoTbl and GridMapIoTbl tables for the next calibration loop
  void modifyTables(const mbapi::StratigraphyManager::SurfaceID surfaceID,
                    const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & hiatus,
                    const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames,
                    const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & correctedMapsSequenceNbr,
                    const std::vector<int> & addedTwtmapsequenceNbr,
                    const std::string & resultsMapFileName,                    
                    const bool noCalculatedTWToutput);

  void setMapNameInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID surfaceID, const std::string& newMapName);
  void generateMapInGridMapIoTbl(const std::string & mapName,
                                 const std::vector<double> & increasedDepths,
                                 size_t & mapsSequenceNbr,
                                 const std::string & resultsMapFileName,
                                 const mbapi::StratigraphyManager::SurfaceID surfaceID);
  mbapi::MapsManager::MapID generateMapInTwoWayTimeIoTbl(const std::vector<double> & tarTwts,
                                                         size_t& mapsSequenceNbr, const std::string & resultsMapFileName);

  void reloadModel(const std::string& caseProjectFilePath);

  long yScalingFactor() const;
  long xScalingFactor() const;

  std::vector<std::string> addedTwtSurfaceNames() const;
  void appendToAddedTwtSurfaceNames(const std::string & addedTwtSurfaceName);
  std::vector<std::string> addedTwtMapNames() const;
  void appendToAddedTwtMapNames(const std::string & surfaceName);
  void setCurrentSurfaceMapNameInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID s, const std::string & mapName);
  std::string t2ZTemporaryMapName(const std::string & topName) const;
  std::string t2ZIsoPackMapName(const std::string & surfaceName) const;

  mbapi::StratigraphyManager& getStratManager();
  mbapi::LithologyManager& getLithoManager();
  mbapi::MapsManager& getMapsManager();

  std::vector<double> getGridMapDepthValues(const mbapi::StratigraphyManager::SurfaceID s);

  mbapi::Model::ReturnCode saveModelToProjectFile(const std::string&  projectFileName, bool copyFiles);

  std::vector<mbapi::StratigraphyManager::SurfaceID> surfacesIDs() const;

private:
  void setSubSamplingWindow();
  void clearTables();
  void setDepthAndTwtPropertiesInFilterTimeIoTbl();
  void reverseDepoSequenceInStratIoTblFromTopSurfaceToBeforeCurrentSurface(const mbapi::StratigraphyManager::SurfaceID surface);
  void setDepoSequenceOfCurrentSurfaceToUndefined(const mbapi::StratigraphyManager::SurfaceID surface);
  void removeFromStratIoTblSurfaceRecordsBelowCurrentSurface(const mbapi::StratigraphyManager::SurfaceID surface);
  void setCurrentMapDataInGridMapIoTbl(const std::string& refferedBy, const std::string & correctedMapsName,
                                       const std::string & resultsMapFileName, const long correctedMapSequenceNbr);
  void appendCorrectedMapNamesInStratIoTbl(const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames);
  void appendCorrectedMapNamesInGridMapIoTbl(const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames,
                                             const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & correctedMapsSequenceNbr,
                                             const std::string & resultsMapFileName);
  void appendCorrectedMapNames(const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames,
                               const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & correctedMapsSequenceNbr,
                               const std::string & resultsMapFileName);
  void appendAddedTwtMapNamesInGridMapIoTbl(const std::vector<int> & addedTwtmapsequenceNbr, const std::string & resultsMapFileName);
  void appendAddedTwtMapNamesInTwoWayTimeIoTbl(const int twtMapsSize);
  void appendAddedTwtMapNamesToTablesIfCalculatedTWToutput(const bool noCalculatedTWToutput,
                                                             const std::vector<int> & addedTwtmapsequenceNbr,
                                                             const std::string & resultsMapFileName);
  void replaceValueInStratIoTblIfIsHiatusAndPreviousErosion(const mbapi::StratigraphyManager::SurfaceID surfaceID, const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & hiatus);

  std::unique_ptr<mbapi::Model> m_mdl;
  long                          m_XScalingFactor;
  long                          m_YScalingFactor;

  std::vector<std::string>      m_addedTwtSurfaceNames;
  std::vector<std::string>      m_addedTwtMapNames;
};

} // namespace fastDepthConversion
