//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FDCProjectManager.h"
#include "CommonDefinitions.h"

#include "LogHandler.h"

#include "MapsManager.h"

#include "propinterface.h"

namespace fastDepthCalibration
{

namespace
{

std::string t2zNameInBPANameMapping(const std::string & bpaNameMapping, const std::string & currentDate)
{
  return "T2Z_" + currentDate + "[" + bpaNameMapping + "]";
}

std::string t2zConversionNameInBPANameMapping(const std::string & bpaNameMapping, const std::string & currentDate, const std::string & BPAName = "")
{
  return "T2Z conversion " + currentDate + " with Base\\n" + (BPAName.empty() ? "" : BPAName + "\\n") + bpaNameMapping;
}

} // namespace

FDCProjectManager::FDCProjectManager(const std::string & projectFileName ) :
  m_mdl(nullptr),
  m_XScalingFactor(0),
  m_YScalingFactor(0),
  m_addedTwtSurfaceNames(),
  m_addedTwtMapNames()
{
  reloadModel(projectFileName);
  m_mdl->subsampling(m_XScalingFactor, m_YScalingFactor);
}

void FDCProjectManager::setSubSamplingWindow()
{
  if (m_XScalingFactor > 1 || m_YScalingFactor > 1)
  { LogHandler(LogHandler::WARNING_SEVERITY) << "The project is subsampled. For the depth calibration the full resolution is recommended, "
                                             << "as an initial test subsampling might be OK. "; }

  long dimI, dimJ, winMinI, winMinJ, winMaxI, winMaxJ;

  m_mdl->hiresGridArealSize(dimI, dimJ);
  m_mdl->window(winMinI, winMaxI, winMinJ, winMaxJ);

  if (winMinI > 1 || winMinJ > 1 || winMaxI < dimI - 1 || winMaxJ < dimJ - 1)
  {
    LogHandler(LogHandler::WARNING_SEVERITY) << "The project is windowed. For the depth calibration the entire domain is required."
                                             << " Changing WindowX and WindowY in to run the simulation over the entire domain";
    if (ErrorHandler::NoError != m_mdl->setWindow(1, dimI - 1, 1, dimJ - 1)) { throw T2Zexception() << "Cannot set window"; }
  }
}

void FDCProjectManager::clearTables()
{
  if (ErrorHandler::NoError != m_mdl->clearTable("TimeIoTbl")) { throw T2Zexception() << "Cannot clear the table TimeIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->clearTable("3DTimeIoTbl")) { throw T2Zexception() << "Cannot clear the table 3DTimeIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->clearTable("1DTimeIoTbl")) { throw T2Zexception() << "Cannot clear the table 1DTimeIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->clearTable("ReservoirIoTbl")) { throw T2Zexception() << "Cannot clear the table ReservoirIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->clearTable("MobLayThicknIoTbl")) { throw T2Zexception() << "Cannot clear the table MobLayThicknIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->clearTable("FilterTimeIoTbl")) { throw T2Zexception() << "Cannot clear the table FilterTimeIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->clearTable("SnapshotIoTbl")) { throw T2Zexception() << "Cannot clear the table SnapshotIoTbl "; }
}

void FDCProjectManager::setDepthAndTwtPropertiesInFilterTimeIoTbl()
{
  if (ErrorHandler::NoError != m_mdl->propertyManager().requestPropertyInSnapshots("Depth", "SedimentsOnly")) { throw T2Zexception() << "Cannot set the table property Depth in the FilterTimeIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->propertyManager().requestPropertyInSnapshots("TwoWayTime", "SedimentsOnly")) { throw T2Zexception() << "Cannot set the table property TwoWayTime in the FilterTimeIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->propertyManager().requestPropertyInSnapshots("TwoWayTimeResidual", "SedimentsOnly")) { throw T2Zexception() << "Cannot set the table property TwoWayTimeResidual in the FilterTimeIoTbl "; }
}

void FDCProjectManager::prepareProject()
{
  setSubSamplingWindow();
  clearTables();
  setDepthAndTwtPropertiesInFilterTimeIoTbl();
}

void FDCProjectManager::reverseDepoSequenceInStratIoTblFromTopSurfaceToBeforeCurrentSurface(const mbapi::StratigraphyManager::SurfaceID surface)
{
  for (mbapi::StratigraphyManager::SurfaceID s = 0; s < surface; ++s)
  {
    long deposequence = surface - s;
    if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", s, "MobileLayer", (long)0)) { throw T2Zexception() << "Cannot set MobileLayer "; }
    if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", s, "DepoSequence", deposequence)) { throw T2Zexception() << "Cannot set DepoSequence"; }
  }
}

void FDCProjectManager::setDepoSequenceOfCurrentSurfaceToUndefined(const mbapi::StratigraphyManager::SurfaceID surface)
{
  if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", surface, "MobileLayer", (long)0)) { throw T2Zexception() << "Cannot set MobileLayer "; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", surface, "DepoSequence", (long)DataAccess::Interface::DefaultUndefinedScalarIntValue)) { throw T2Zexception() << "Cannot set DepoSequence "; }
}

void FDCProjectManager::removeFromStratIoTblSurfaceRecordsBelowCurrentSurface(const mbapi::StratigraphyManager::SurfaceID surface, const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs)
{
  for (mbapi::StratigraphyManager::SurfaceID s = surface + 1; s < surfacesIDs.size(); ++s)
  { if (ErrorHandler::NoError != m_mdl->removeRecordFromTable("StratIoTbl", surface + 1)) { throw T2Zexception() << "Cannot remove records from the StratIoTbl "; } }
}

void FDCProjectManager::appendCorrectedMapNamesInStratIoTbl(const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames)
{
  for (auto it = correctedMapsNames.begin(); it != correctedMapsNames.end(); ++it)
  {
    const string correctedMapName = it->second;
    if (correctedMapName.empty()) { continue; }
    if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", it->first, "DepthGrid", it->second)) { throw T2Zexception() << "Cannot set DepthGrid "; }
  }
}

void FDCProjectManager::setCurrentMapDataInGridMapIoTbl(const std::string & correctedMapsName, const std::string & resultsMapFileName, const long correctedMapSequenceNbr)
{
  size_t row = m_mdl->tableSize("GridMapIoTbl") - 1;

  if (ErrorHandler::NoError != m_mdl->setTableValue("GridMapIoTbl", row, "ReferredBy", "StratIoTbl")) { throw T2Zexception() << "Cannot set ReferredBy in GridMapIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("GridMapIoTbl", row, "MapName", correctedMapsName)) { throw T2Zexception() << "Cannot set MapName in GridMapIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("GridMapIoTbl", row, "MapType", "HDF5")) { throw T2Zexception() << "Cannot set MapType in GridMapIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("GridMapIoTbl", row, "MapFileName", resultsMapFileName)) { throw T2Zexception() << "Cannot set MapFileName in GridMapIoTbl "; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("GridMapIoTbl", row, "MapSeqNbr", correctedMapSequenceNbr)) { throw T2Zexception() << "Cannot set MapSeqNbr in GridMapIoTbl "; }
}

void FDCProjectManager::appendCorrectedMapNamesInGridMapIoTbl(const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames,
                                                              const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & correctedMapsSequenceNbr,
                                                              const std::string & resultsMapFileName)
{
  for (auto it = correctedMapsNames.begin(); it != correctedMapsNames.end(); ++it)
  {
    const string correctedMapName = it->second;
    if (correctedMapName.empty()) { continue; }
    if (ErrorHandler::NoError != m_mdl->addRowToTable("GridMapIoTbl")) { throw T2Zexception() << "Cannot add a new row in GridMapIoTbl"; }
    setCurrentMapDataInGridMapIoTbl(correctedMapName, resultsMapFileName, (long)correctedMapsSequenceNbr.at(it->first));
  }
}

void FDCProjectManager::appendCorrectedMapNames(const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames,
                                                const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & correctedMapsSequenceNbr,
                                                const std::string & resultsMapFileName)
{
  appendCorrectedMapNamesInStratIoTbl(correctedMapsNames);
  appendCorrectedMapNamesInGridMapIoTbl(correctedMapsNames, correctedMapsSequenceNbr, resultsMapFileName);
}

void FDCProjectManager::appendAddedTwtMapNamesInGridMapIoTbl(const std::vector<int> & addedTwtmapsequenceNbr, const std::string & resultsMapFileName)
{
  int index = 0;
  for (auto it : addedTwtmapsequenceNbr)
  {
    if (ErrorHandler::NoError != m_mdl->addRowToTable("GridMapIoTbl")) { throw T2Zexception() << "Cannot add a new row in GridMapIoTbl"; }
    setCurrentMapDataInGridMapIoTbl(m_addedTwtMapNames[index], resultsMapFileName, (long)it);
    ++index;
  }
}

void FDCProjectManager::appendAddedTwtMapNamesInTwoWayTimeIoTbl(const int twtMapsSize)
{
  for (int index = 0; index < twtMapsSize; ++index)
  {
    if (ErrorHandler::NoError != m_mdl->addRowToTable("TwoWayTimeIoTbl")) { throw T2Zexception() << "Cannot add a new row in TwoWayTimeIoTbl"; }
    size_t row = m_mdl->tableSize("TwoWayTimeIoTbl")-1;
    if (ErrorHandler::NoError != m_mdl->setTableValue("TwoWayTimeIoTbl", row, "SurfaceName", m_addedTwtSurfaceNames[index])) { throw T2Zexception() << "Cannot set SurfaceName in TwoWayTimeIoTbl ";}
    if (ErrorHandler::NoError != m_mdl->setTableValue("TwoWayTimeIoTbl", row, "TwoWayTimeGrid", m_addedTwtMapNames[index])) { throw T2Zexception() << "Cannot set TwoWayTimeGrid in TwoWayTimeIoTbl ";}
    if (ErrorHandler::NoError != m_mdl->setTableValue("TwoWayTimeIoTbl", row, "TwoWayTime", Utilities::Numerical::IbsNoDataValue)) { throw T2Zexception() << "Cannot set TwoWayTime in TwoWayTimeIoTbl ";}
  }
}

void FDCProjectManager::appendAddedTwtMapNamesToTablesIfNoCalculatedTWToutput(const bool noCalculatedTWToutput,
                                                                              const std::vector<int> & addedTwtmapsequenceNbr,
                                                                              const std::string & resultsMapFileName)
{
  if (noCalculatedTWToutput) { return; }
  appendAddedTwtMapNamesInGridMapIoTbl(addedTwtmapsequenceNbr, resultsMapFileName);
  appendAddedTwtMapNamesInTwoWayTimeIoTbl(addedTwtmapsequenceNbr.size());
}

void FDCProjectManager::replaceValueInStratIoTblIfIsHiatusAndPreviousErosion(const mbapi::StratigraphyManager::SurfaceID surfaceID, const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & hiatus)
{
  for (mbapi::StratigraphyManager::SurfaceID s = 1; s < surfaceID; ++s)
  {
    if (!(hiatus.count(s-1) > 0 && hiatus.at(s-1) > -1)) { continue; }
    std::string previousMapName = m_mdl->tableValueAsString("StratIoTbl", hiatus.at(s-1) , "DepthGrid");
    if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", s, "DepthGrid", previousMapName) ){ throw T2Zexception() << "Cannot set hiatus map name for surface id"<<s; }
  }
}

void FDCProjectManager::modifyTables(const mbapi::StratigraphyManager::SurfaceID surfaceID,
                                     const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & hiatus,
                                     const std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> & correctedMapsNames,
                                     const std::map<const mbapi::StratigraphyManager::SurfaceID, int> & correctedMapsSequenceNbr,
                                     const std::vector<int> & addedTwtmapsequenceNbr,
                                     const std::string & resultsMapFileName,
                                     const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfaceIDs,
                                     const bool noCalculatedTWToutput)
{
  reverseDepoSequenceInStratIoTblFromTopSurfaceToBeforeCurrentSurface(surfaceID);
  setDepoSequenceOfCurrentSurfaceToUndefined(surfaceID);
  removeFromStratIoTblSurfaceRecordsBelowCurrentSurface(surfaceID, surfaceIDs);
  appendCorrectedMapNames(correctedMapsNames, correctedMapsSequenceNbr, resultsMapFileName);
  appendAddedTwtMapNamesToTablesIfNoCalculatedTWToutput(noCalculatedTWToutput, addedTwtmapsequenceNbr, resultsMapFileName);
  replaceValueInStratIoTblIfIsHiatusAndPreviousErosion(surfaceID, hiatus);
}

void FDCProjectManager::setMapNameInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID surfaceID, const std::string & newMapName)
{
  if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", surfaceID, "DepthGrid", newMapName))
  { throw T2Zexception() << "Cannot set the map " << newMapName << " as the new depth map of surface " << surfaceID << " in the StratIoTbl"; }
}

std::shared_ptr<mbapi::Model> FDCProjectManager::getModel() const
{
  return m_mdl;
}

void FDCProjectManager::reloadModel(const std::string & caseProjectFilePath)
{
  MPI_Barrier(PETSC_COMM_WORLD);
  m_mdl.reset(new mbapi::Model());
  if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(caseProjectFilePath.c_str()))
  { throw T2Zexception() << "Cannot load model from " << caseProjectFilePath << ", " << m_mdl->errorMessage(); }
  MPI_Barrier(PETSC_COMM_WORLD);
}

std::string FDCProjectManager::bpaNameMapping(const size_t row) const
{
  return m_mdl->tableValueAsString("BPANameMapping", row, "BPAColumnValue");
}

void FDCProjectManager::setAlternativeTableNames(const std::string & currentDate)
{
  const std::string BPAName = bpaNameMapping(0);
  if (ErrorHandler::NoError != m_mdl->setTableValue("BPANameMapping", 0, "BPAColumnValue", t2zNameInBPANameMapping(bpaNameMapping(0), currentDate))) { throw T2Zexception() << "Cannot set CAULDRON_ALTERNATIVE NAME in BPANameMapping"; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("BPANameMapping", 1, "BPAColumnValue", t2zConversionNameInBPANameMapping(bpaNameMapping(1), currentDate, BPAName))) { throw T2Zexception() << "Cannot set CAULDRON_ALTERNATIVE DESCRIPTION in BPANameMapping"; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("BPANameMapping", 2, "BPAColumnValue", t2zNameInBPANameMapping(bpaNameMapping(2), currentDate))) { throw T2Zexception() << "Cannot set CAULDRON NAME in BPANameMapping"; }
  if (ErrorHandler::NoError != m_mdl->setTableValue("BPANameMapping", 3, "BPAColumnValue", t2zConversionNameInBPANameMapping(bpaNameMapping(3), currentDate))) { throw T2Zexception() << "Cannot set CAULDRON  DESCRIPTION in BPANameMapping"; }
}

void FDCProjectManager::setCurrentSurfaceMapNameInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID s, const std::string & mapName)
{
  if (ErrorHandler::NoError != m_mdl->setTableValue("StratIoTbl", s, "DepthGrid", mapName))
  {
    throw T2Zexception() << "Cannot set the map " << mapName << " as new depth iso surface in the StratIoTbl";
  }
}

void FDCProjectManager::generateMapInGridMapIoTbl(const std::string & mapName,
                                                  const std::vector<double> & increasedDepths,
                                                  size_t & mapsSequenceNbr,
                                                  const std::string & resultsMapFileName,
                                                  const mbapi::StratigraphyManager::SurfaceID surfaceID)
{
  mbapi::MapsManager & m_mapsMgrLocal = m_mdl->mapsManager();

  if (increasedDepths.empty())
  {
    return;
  }
  const bool tmp = m_mapsMgrLocal.generateMap("StratIoTbl", mapName, increasedDepths, mapsSequenceNbr, resultsMapFileName) > m_mapsMgrLocal.mapsIDs().size();
  if (tmp) { throw T2Zexception() << "Wrong ID for maps with corrected depths for the surface " << surfaceID; }
}

mbapi::MapsManager::MapID FDCProjectManager::generateMapInTwoWayTimeIoTbl(const std::vector<double> & tarTwts,
                                                                          size_t & mapsSequenceNbr,
                                                                          const std::string & resultsMapFileName)
{
  mbapi::MapsManager & m_mapsMgrLocal = m_mdl->mapsManager();
  return m_mapsMgrLocal.generateMap("TwoWayTimeIoTbl", m_addedTwtMapNames.back(), tarTwts, mapsSequenceNbr, resultsMapFileName);
}

std::vector<std::string> FDCProjectManager::addedTwtSurfaceNames() const
{
  return m_addedTwtSurfaceNames;
}

void FDCProjectManager::appendToAddedTwtSurfaceNames(const std::string & addedTwtSurfaceName)
{
  m_addedTwtSurfaceNames.push_back(addedTwtSurfaceName);
}

long FDCProjectManager::yScalingFactor() const
{
  return m_YScalingFactor;
}

long FDCProjectManager::xScalingFactor() const
{
  return m_XScalingFactor;
}

std::vector<std::string> FDCProjectManager::addedTwtMapNames() const
{
  return m_addedTwtMapNames;
}

void FDCProjectManager::appendToAddedTwtMapNames(const std::string & surfaceName)
{
  m_addedTwtMapNames.push_back("Calculated_TWT_for_" + surfaceName);
}

std::string FDCProjectManager::t2ZTemporaryMapName(const std::string & topName, const std::string & date) const
{
  return "T2Z[" + date + "][" + topName + "]";
}

std::string FDCProjectManager::t2ZIsoPackMapName(const std::string & surfaceName, const std::string & date) const
{
  return "T2ZIsoSurf[" + date + "][" + surfaceName + "]";
}

} // namespace fastDepthCalibration
