//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FDCMapFieldProperties.h"
#include "CommonDefinitions.h"

#include "ErrorHandler.h"

#include "LogHandler.h"

namespace fastDepthConversion
{

namespace
{

static const double s_thickness_threshold = 1.e-3;
static const double s_twt_threshold = 1.e-1;

bool checkMissingTwtRefAndNextSurfacePointValuesAreValid(const double refDepthI, const double refTwtI, const double nextDepthI, const double nextTwtI, const double currentDepthI)
{
  return (refDepthI == DataAccess::Interface::DefaultUndefinedMapValue ||
          refTwtI == DataAccess::Interface::DefaultUndefinedMapValue ||
          nextDepthI == DataAccess::Interface::DefaultUndefinedMapValue ||
          nextTwtI == DataAccess::Interface::DefaultUndefinedMapValue ||
          currentDepthI == DataAccess::Interface::DefaultUndefinedMapValue);
}

double checkThresholdsAndAssignTwtValue(const double nextDepthI, const double refDepthI, const double nextTwtI,
                                        const double refTwtI, const double currentDepthI)
{
  const double FM_thickness = nextDepthI - refDepthI;
  const double FM_twt = nextTwtI - refTwtI;
  double TwtI = refTwtI;
  if (FM_thickness > s_thickness_threshold &&
      currentDepthI > (refDepthI + s_thickness_threshold) &&
      FM_twt > s_twt_threshold)
  { TwtI += FM_twt * ((currentDepthI - refDepthI )/FM_thickness ); }

  return TwtI;
}

void calculateMissingTwt(std::vector<double> & Twt, const std::vector<double> & refDepth, const std::vector<double> & refTwt, const std::vector<double> & nextDepth,
                         const std::vector<double> & nextTwt, const std::vector<double> & currentDepth)
{
  for (size_t i = 0; i != Twt.size(); ++i)
  {
    if(!checkMissingTwtRefAndNextSurfacePointValuesAreValid(refDepth[i], refTwt[i], nextDepth[i], nextTwt[i], currentDepth[i]))
    {
      Twt[i] = DataAccess::Interface::DefaultUndefinedMapValue;
      continue;
    }

    Twt[i] = checkThresholdsAndAssignTwtValue(nextDepth[i], refDepth[i], nextTwt[i], refTwt[i], currentDepth[i]);
  }
}

void throwMapsManagerError(const std::string & message, const mbapi::StratigraphyManager::SurfaceID surfID, const mbapi::MapsManager & mapsMgrLocal)
{
  throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << message << surfID << ", message: " << mapsMgrLocal.errorMessage();
}

} // namespace

FDCMapFieldProperties::FDCMapFieldProperties(std::shared_ptr<mbapi::Model> & mdl, const mbapi::StratigraphyManager::SurfaceID referenceSurface, const mbapi::StratigraphyManager::SurfaceID endSurface) :
  m_mdl{mdl},
  m_referenceSurface{referenceSurface},
  m_endSurface{endSurface},
  m_correctedMapsNames{},
  m_twtMaps{},
  m_isoPacks{},
  m_missingTwtMaps{},
  m_correctedMapsSequenceNbr{},
  m_hiatus{},
  m_refDepthsHistory{},
  m_refTwtHistory{},
  m_addedTwtmapsequenceNbr{}
{
  initializeAllMaps();
}

void FDCMapFieldProperties::initializeAllMaps()
{
  for (mbapi::StratigraphyManager::SurfaceID s = m_referenceSurface; s <= m_endSurface; ++s)
  {
    m_correctedMapsNames[s] = "";
    m_twtMaps[s] = "";
    m_correctedMapsSequenceNbr[s] = -1;
    m_hiatus[s] = -1;
    m_missingTwtMaps[s] = {0.0};
  }
}

void FDCMapFieldProperties::calculateInitialMaps(const string & masterResultsFilePathName, const bool preserveErosionFlag)
{
  setTwtMapsForAllSurfaces();
  detectHiatusForAllSurfaces();
  calculateIsoPackThicknessForSurfacesBelowEndSurface(preserveErosionFlag);
  initializeMapWriterOnMasterResultsFile(masterResultsFilePathName);
}

void FDCMapFieldProperties::setTwtMapsForAllSurfaces()
{
  mbapi::StratigraphyManager & stMgr = m_mdl->stratigraphyManager();
  for (mbapi::StratigraphyManager::SurfaceID surface = m_referenceSurface; surface <= m_endSurface; ++surface)
  {
    const std::string twtGridName = stMgr.twtGridName(surface);
    if (twtGridName.empty())
    {
      throw T2Zexception() << " No twt map was found for the " << (surface == m_referenceSurface ? "reference" : "end") << " surface  " << surface;
    }

    setTwtMaps(surface, twtGridName);
  }
}

void FDCMapFieldProperties::detectHiatusForAllSurfaces()
{
  mbapi::StratigraphyManager & stMgr = m_mdl->stratigraphyManager();
  for (mbapi::StratigraphyManager::SurfaceID currentSurface = m_referenceSurface; currentSurface <= m_endSurface; ++currentSurface)
  {
    const std::string depthMapS = m_mdl->tableValueAsString( "StratIoTbl", currentSurface, "DepthGrid" );
    if ( IsValueUndefined(depthMapS) ) { continue; }

    for (mbapi::StratigraphyManager::SurfaceID it = m_referenceSurface; it < currentSurface; ++it)
    {
      const std::string depthMap = m_mdl->tableValueAsString( "StratIoTbl", it, "DepthGrid" );
      if (twtMaps(currentSurface) ==  twtMaps(it) && !IsValueUndefined(depthMap) && depthMap == depthMapS)
      {
        LogHandler(LogHandler::WARNING_SEVERITY) << "Hiatus detected: "<< stMgr.surfaceName(currentSurface) << "on Surface" << stMgr.surfaceName(it);
        setHiatus(currentSurface-1, it);
      }
    }
  }
}

std::string FDCMapFieldProperties::getTwtGridName(const mbapi::StratigraphyManager::SurfaceID surface, mbapi::StratigraphyManager & stMgr)
{
  std::string twtGridName = stMgr.twtGridName(surface);
  if (twtGridName.empty())
  { throw T2Zexception() << " No twt map was found for the " << (surface == m_referenceSurface ? "reference" : "end") << " surface  " << surface; }

  return twtGridName;
}

void FDCMapFieldProperties::initializeMapWriterOnMasterResultsFile(const string & masterResultsFilePathName) const
{
  mbapi::MapsManager & mapMgr = m_mdl->mapsManager();
  if (ErrorHandler::NoError != mapMgr.initializeMapWriter(masterResultsFilePathName, false))
  { throw T2Zexception() << "Cannot initialize the map writer"; }

  if (ErrorHandler::NoError != mapMgr.finalizeMapWriter())
  { throw T2Zexception() << "Cannot finalize the map writer"; }
}

std::vector<double> FDCMapFieldProperties::getGridMapDepthValuesForCurrentSurface(const mbapi::StratigraphyManager::SurfaceID & currentSurface) const
{
  std::vector<double> bottomDepth;
  if (!m_mdl->getGridMapDepthValues(currentSurface, bottomDepth))
  { throw T2Zexception() << " Cannot get the depth map for the surface " << currentSurface << ", " << m_mdl->errorMessage(); }
  return bottomDepth;
}

void FDCMapFieldProperties::calculateIsoPackThicknessForSurfacesBelowEndSurface(const bool preserveErosionFlag)
{
  mbapi::StratigraphyManager & stMgr = m_mdl->stratigraphyManager();
  const std::vector<double> depthEndSurface = getGridMapDepthValuesForCurrentSurface(m_endSurface);
  for (mbapi::StratigraphyManager::SurfaceID s = m_endSurface + 1; s < stMgr.surfacesIDs().size(); ++s)
  {
    std::vector<double> bottomDepth = getGridMapDepthValuesForCurrentSurface(s);

    for (size_t i = 0; i != bottomDepth.size(); ++i)
    {
      bottomDepth[i] -= depthEndSurface[i];
      if (!preserveErosionFlag && bottomDepth[i] < 0.0) { bottomDepth[i] = 0.0; }
    }
    m_isoPacks[s] = bottomDepth;
  }
}

std::vector<double> FDCMapFieldProperties::getCurrentDepth(const mbapi::StratigraphyManager::SurfaceID surfID) const
{
  const mbapi::MapsManager& mapsMgrLocal = m_mdl->mapsManager();
  std::vector<double> currentDepth;
  if (!m_mdl->getGridMapDepthValues(surfID, currentDepth))
  { throwMapsManagerError(" Cannot get the current depth for the surface ", surfID, mapsMgrLocal); }
  return currentDepth;
}

mbapi::StratigraphyManager::SurfaceID FDCMapFieldProperties::getIDofFirstNonMissingTwtSurface(const mbapi::StratigraphyManager::SurfaceID surfID) const
{
  mbapi::StratigraphyManager::SurfaceID s = surfID-1;
  while (s)
  {
    if (!twtMaps(s).empty()) { break; }
    --s;
  }

  if (s == 0) { throw T2Zexception() << "Failed to get the reference TWT map!" << "\n"; }
  return s;
}

std::vector<double> FDCMapFieldProperties::getReferenceDepth(const mbapi::StratigraphyManager::SurfaceID s) const
{
  const mbapi::MapsManager& mapsMgrLocal = m_mdl->mapsManager();
  std::vector<double> refDepth;
  if (!m_mdl->getGridMapDepthValues(s, refDepth))
  { throwMapsManagerError(" Cannot get the depth map for the current surface ", s, mapsMgrLocal); }
  return refDepth;
}

std::vector<double> FDCMapFieldProperties::getReferenceTwt(const mbapi::StratigraphyManager::SurfaceID s) const
{
  mbapi::MapsManager& mapsMgrLocal = m_mdl->mapsManager();
  std::vector<double> refTwt;
  mbapi::MapsManager::MapID refTwtID = mapsMgrLocal.findID(twtMaps(s));
  if (ErrorHandler::NoError != mapsMgrLocal.mapGetValues(refTwtID, refTwt))
  { throwMapsManagerError(" Cannot get the measured twt map for the current surface ", s, mapsMgrLocal); }
  return refTwt;
}

bool FDCMapFieldProperties::getIDofLastExistingTwtSurface(const mbapi::StratigraphyManager::SurfaceID surfID, mbapi::StratigraphyManager::SurfaceID & surface) const
{
  for (mbapi::StratigraphyManager::SurfaceID s = surfID+1; s <= m_endSurface; ++s)
  {
    if(!twtMaps(s).empty())
    {
      surface = s;
      return true;
    }
  }
  return false;
}

std::vector<double> FDCMapFieldProperties::getNextTwt(const mbapi::StratigraphyManager::SurfaceID s) const
{
  mbapi::MapsManager& mapsMgrLocal = m_mdl->mapsManager();
  std::vector<double> nextTwt;
  mbapi::MapsManager::MapID nextTwtID = mapsMgrLocal.findID(twtMaps(s));
  if (ErrorHandler::NoError != mapsMgrLocal.mapGetValues(nextTwtID, nextTwt))
  { throwMapsManagerError(" Cannot get the measured twt map for the current surface ", s, mapsMgrLocal); }
  return nextTwt;
}

std::vector<double> FDCMapFieldProperties::getNextDepth(const mbapi::StratigraphyManager::SurfaceID s) const
{
  const mbapi::MapsManager& mapsMgrLocal = m_mdl->mapsManager();
  std::vector<double> nextDepth;
  if (!m_mdl->getGridMapDepthValues(s, nextDepth))
  { throwMapsManagerError(" Cannot get the depth map for the current surface ", s, mapsMgrLocal); }
  return nextDepth;
}

void FDCMapFieldProperties::setMissingTwtForSurface(const mbapi::StratigraphyManager::SurfaceID surfID)
{
  const std::vector<double> currentDepth = getCurrentDepth(surfID);
  const mbapi::StratigraphyManager::SurfaceID sRef = getIDofFirstNonMissingTwtSurface(surfID);
  const std::vector<double> refTwt = getReferenceTwt(sRef);
  const std::vector<double> refDepth = getReferenceDepth(sRef);

  mbapi::StratigraphyManager::SurfaceID sNext;
  if (!getIDofLastExistingTwtSurface(surfID, sNext))
  { throw T2Zexception() << "Cannot find an existing surface with valid TwoWayTime values." << "\n"; }
  const std::vector<double> nextTwt = getNextTwt(sNext);
  const std::vector<double> nextDepth = getNextDepth(sNext);

  std::vector<double> Twt(refTwt.size());
  calculateMissingTwt(Twt, refDepth, refTwt, nextDepth, nextTwt, currentDepth);
  setMissingTwtMaps(surfID, Twt);
}

void FDCMapFieldProperties::setPropertyHistoryMaps(const mbapi::StratigraphyManager::LayerID currentLayer,
                                                   const std::vector<double> & refDepths,
                                                   const std::vector<double> & refTwts)
{
  m_refDepthsHistory[currentLayer] = refDepths;
  m_refTwtHistory[currentLayer] = refTwts;
}

std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double> > FDCMapFieldProperties::refDepthsHistory() const
{
  return m_refDepthsHistory;
}

std::map<const mbapi::StratigraphyManager::LayerID, std::vector<double> > FDCMapFieldProperties::refTwtHistory() const
{
  return m_refTwtHistory;
}

std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> FDCMapFieldProperties::correctedMapsNames() const
{
  return m_correctedMapsNames;
}

std::string FDCMapFieldProperties::correctedMapsName(const mbapi::StratigraphyManager::SurfaceID ID) const
{
  return m_correctedMapsNames.count(ID) ? m_correctedMapsNames.at(ID) : "";
}

void FDCMapFieldProperties::setCorrectedMapsNames(const mbapi::StratigraphyManager::SurfaceID ID, const std::string& value)
{
  m_correctedMapsNames[ID] = value;
}

std::map<const mbapi::StratigraphyManager::SurfaceID, int> FDCMapFieldProperties::correctedMapsSequenceNbr() const
{
  return m_correctedMapsSequenceNbr;
}

void FDCMapFieldProperties::setCorrectedMapsSequenceNbr(const mbapi::StratigraphyManager::SurfaceID ID, const int value)
{
  m_correctedMapsSequenceNbr[ID] = value;
}

std::vector<int> FDCMapFieldProperties::addedTwtmapsequenceNbr() const
{
  return m_addedTwtmapsequenceNbr;
}

void FDCMapFieldProperties::setAddedTwtmapsequenceNbr(const int number)
{
  m_addedTwtmapsequenceNbr.push_back(number);
}

std::map<const mbapi::StratigraphyManager::SurfaceID, std::vector<double> > FDCMapFieldProperties::isoPacks() const
{
  return m_isoPacks;
}

void FDCMapFieldProperties::setIsoPacks(const mbapi::StratigraphyManager::SurfaceID ID, const std::vector<double> & value)
{
  m_isoPacks[ID] = value;
}

std::string FDCMapFieldProperties::twtMaps(const mbapi::StratigraphyManager::SurfaceID ID) const
{
  return m_twtMaps.count(ID) ? m_twtMaps.at(ID) : "";
}

void FDCMapFieldProperties::setTwtMaps(const mbapi::StratigraphyManager::SurfaceID ID, const string & value)
{
  m_twtMaps[ID] = value;
}

std::vector<double> FDCMapFieldProperties::missingTwtMaps(const mbapi::StratigraphyManager::SurfaceID ID) const
{
  return m_missingTwtMaps.count(ID) ? m_missingTwtMaps.at(ID) : std::vector<double>{};
}

void FDCMapFieldProperties::setMissingTwtMaps(const mbapi::StratigraphyManager::SurfaceID ID, const std::vector<double> & value)
{
  m_missingTwtMaps[ID] = value;
}

std::map<const mbapi::StratigraphyManager::SurfaceID, int> FDCMapFieldProperties::hiatusAll() const
{
  return m_hiatus;
}

int FDCMapFieldProperties::hiatusID(const mbapi::StratigraphyManager::SurfaceID ID) const
{
  return m_hiatus.count(ID) ? m_hiatus.at(ID) : DataAccess::Interface::DefaultUndefinedScalarIntValue;
}

void FDCMapFieldProperties::setHiatus(const mbapi::StratigraphyManager::SurfaceID ID, const int value)
{
  m_hiatus[ID] = value;
}

void FDCMapFieldProperties::setModel(std::shared_ptr<mbapi::Model> & mdl)
{
  m_mdl.reset();
  m_mdl = mdl;
}

} // namespace fastDepthConversion
