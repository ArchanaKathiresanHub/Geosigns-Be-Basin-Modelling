//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FDCVectorFieldProperties.h"

namespace fastDepthCalibration
{

FDCVectorFieldProperties::FDCVectorFieldProperties(std::shared_ptr<mbapi::Model> & mdl, const mbapi::StratigraphyManager::SurfaceID referenceSurface) :
  m_mdl{mdl},
  m_refDepths{},
  m_newDepths{},
  m_refTwts{},
  m_tarTwts{},
  m_preservedErosion{},
  m_twtconvFactor{1.e-3}
{
  m_refDepths = getSurfaceDepthFromGridMapValues(referenceSurface);
  m_refTwts.resize(m_refDepths.size(), 0.0);
  m_tarTwts = m_refTwts;
}

void FDCVectorFieldProperties::setTopSurfaceProperties(const mbapi::StratigraphyManager::SurfaceID surfaceID,
                                                       const mbapi::StratigraphyManager::SurfaceID referenceSurface,
                                                       const std::string & twtMapNames)
{
  if (surfaceID != referenceSurface)
  {
    m_refDepths = m_newDepths;
    m_refTwts = m_tarTwts;
    return;
  }

  retrievePropertiesWhenAtReferenceSurface(surfaceID, twtMapNames);
}

std::vector<double> FDCVectorFieldProperties::setMeasuredTwtAtSpecifiedSurface(const mbapi::StratigraphyManager::SurfaceID surface, const std::string & twtMapNames) const
{
  std::vector<double> tarTwts;
  mbapi::MapsManager & mapsMgrLocal = m_mdl->mapsManager();

  mbapi::MapsManager::MapID twtMapID = mapsMgrLocal.findID(twtMapNames);
  if (ErrorHandler::ReturnCode::NoError != mapsMgrLocal.mapGetValues(twtMapID, tarTwts))
  {
    throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << " Cannot get the measured twt map for the surface " << surface
                                                            << ", message: " << mapsMgrLocal.errorMessage();
  }

  return tarTwts;
}

std::vector<double> FDCVectorFieldProperties::calculateIncreasedDepthsIncludingPreservedErosion(const mbapi::StratigraphyManager::SurfaceID currentSurface, const double maxSeisVel) const
{
  const int nRefDepths = m_refDepths.size();
  std::vector<double> increasedDepths(nRefDepths);
  if (currentSurface == 0)
  {
    calculateTopMostSurfaceIncreasedDepth(increasedDepths, nRefDepths, maxSeisVel);
  }
  else
  {
    calculateSurfaceIncreasedDepth(increasedDepths, nRefDepths, maxSeisVel);
  }
  return increasedDepths;
}

std::vector<double> FDCVectorFieldProperties::getSurfaceDepthFromGridMapValues(const mbapi::StratigraphyManager::SurfaceID surfID) const
{
  mbapi::MapsManager & mapsMgrLocal = m_mdl->mapsManager();

  std::vector<double> depths;
  if (!m_mdl->getGridMapDepthValues(surfID, depths))
  {
    throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << " Cannot get the depth map for the current surface " << surfID
                                                            << ", message: " << mapsMgrLocal.errorMessage();
  }

  return depths;
}

void FDCVectorFieldProperties::calculateTopMostSurfaceIncreasedDepth(std::vector<double> & increasedDepths, const int nRefDepths, const double maxSeisVel) const
{
  for (size_t i = 0; i != nRefDepths; ++i)
  {
    if (m_refDepths[i] == DataAccess::Interface::DefaultUndefinedMapValue || m_tarTwts[i] == DataAccess::Interface::DefaultUndefinedMapValue)
    {
      increasedDepths[i] = DataAccess::Interface::DefaultUndefinedMapValue;
    }
    else
    {
      increasedDepths[i] = m_refDepths[i] + m_tarTwts[i] * twtconvFactor() * maxSeisVel * 0.5;
    }
  }
}

void FDCVectorFieldProperties::calculateSurfaceIncreasedDepth(std::vector<double> & increasedDepths, const int nRefDepths, const double maxSeisVel) const
{
  for (size_t i = 0; i != nRefDepths; ++i)
  {
    if (m_refDepths[i] == DataAccess::Interface::DefaultUndefinedMapValue ||
        m_tarTwts[i] == DataAccess::Interface::DefaultUndefinedMapValue ||
        m_refTwts[i] == DataAccess::Interface::DefaultUndefinedMapValue )
    {
      increasedDepths[i] = DataAccess::Interface::DefaultUndefinedMapValue;
    }
    else
    {
      // Note: m_preservedErosion is the one from previous step!
      increasedDepths[i] = m_refDepths[i] - m_preservedErosion[i] + (m_tarTwts[i] - m_refTwts[i]) * twtconvFactor() * maxSeisVel * 0.5;
    }
  }
}

void FDCVectorFieldProperties::retrievePropertiesWhenAtReferenceSurface(const mbapi::StratigraphyManager::SurfaceID currentSurface, const std::string & twtMapNames)
{
  m_refDepths = getSurfaceDepthFromGridMapValues(currentSurface);
  m_newDepths.resize(m_refDepths.size());
  m_preservedErosion.resize(m_refDepths.size(),0.0);

  m_refTwts = setMeasuredTwtAtSpecifiedSurface(currentSurface, twtMapNames);
}

double FDCVectorFieldProperties::twtconvFactor() const
{
  return m_twtconvFactor;
}

std::vector<double> FDCVectorFieldProperties::refDepths() const
{
  return m_refDepths;
}

std::vector<double> FDCVectorFieldProperties::newDepths() const
{
  return m_newDepths;
}

void FDCVectorFieldProperties::setNewDepths(const std::vector<double> & newDepths)
{
  m_newDepths = newDepths;
}

std::vector<double> FDCVectorFieldProperties::refTwts() const
{
  return m_refTwts;
}

std::vector<double> FDCVectorFieldProperties::tarTwts() const
{
  return m_tarTwts;
}

void FDCVectorFieldProperties::setTarTwts(const std::vector<double> & tarTwts)
{
  m_tarTwts = tarTwts;
}

std::vector<double> FDCVectorFieldProperties::preservedErosion() const
{
  return m_preservedErosion;
}

void FDCVectorFieldProperties::setPreservedErosion(const std::vector<double> & values)
{
  m_preservedErosion = values;
}

void FDCVectorFieldProperties::setModel(std::shared_ptr<mbapi::Model> & mdl)
{
  m_mdl.reset();
  m_mdl = mdl;
}


} // namespace fastDepthCalibration
