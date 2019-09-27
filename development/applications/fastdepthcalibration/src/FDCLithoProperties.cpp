//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FDCLithoProperties.h"

namespace fastDepthCalibration
{

FDCLithoProperties::FDCLithoProperties(mbapi::StratigraphyManager & stMgrLocal, mbapi::LithologyManager & litMgrLocal) :
  m_stMgrLocal{&stMgrLocal},
  m_litMgrLocal{&litMgrLocal},
  m_nextSurface{0},
  m_currentLayer{0},
  m_nextLayer{0},
  m_currentTopName{""},
  m_nextTopName{""},
  m_currentLayerName{""},
  m_lithoList{},
  m_lithoPercent{},
  m_lithoPercMap{}
{
}

void FDCLithoProperties::setlayerLithologiesListForCurrentLayer()
{
  if (ErrorHandler::ReturnCode::NoError != m_stMgrLocal->layerLithologiesList(m_currentLayer, m_lithoList, m_lithoPercent, m_lithoPercMap))
  {
    throw ErrorHandler::Exception(m_stMgrLocal->errorCode()) << "Cannot read the lithologies for the current layer: " << m_stMgrLocal->errorMessage();
  }
}

void FDCLithoProperties::setLithoSurfaces(const mbapi::StratigraphyManager::SurfaceID currentSurface)
{
  m_nextSurface = currentSurface + 1;
  m_currentLayer = currentSurface;
  m_nextLayer = m_nextSurface;

  m_currentTopName = m_stMgrLocal->surfaceName(m_currentLayer);
  m_nextTopName = m_stMgrLocal->surfaceName(m_nextLayer);
  m_currentLayerName = m_stMgrLocal->layerName(m_currentLayer);
}

double FDCLithoProperties::getMaxSeismicVelocityForCurrentLayer() const
{
  double maxSeisVel = 0.0;
  for (const std::string& lith : m_lithoList)
  {
    if ( lith.empty() ) { continue; }

    const mbapi::LithologyManager::LithologyID lithID = m_litMgrLocal->findID(lith);
    if ( IsValueUndefined( lithID ) )
    {
      throw ErrorHandler::Exception(m_litMgrLocal->errorCode()) << "Cannot find the id for the lithology " << lith
                                                                << ", " << m_litMgrLocal->errorMessage();
    }

    const double seisVel = m_litMgrLocal->seisVelocity(lithID);
    if ( IsValueUndefined( seisVel ) )
    {
      throw ErrorHandler::Exception(m_litMgrLocal->errorCode()) << "Cannot find the seismic velocity for the lithology "
                                                                << lith << ", " << m_litMgrLocal->errorMessage();
    }

    if (maxSeisVel < seisVel) { maxSeisVel = seisVel; }
  }

  return maxSeisVel;
}


std::vector<mbapi::StratigraphyManager::SurfaceID> FDCLithoProperties::surfacesIDs() const
{
  return m_stMgrLocal->surfacesIDs();
}

mbapi::StratigraphyManager::SurfaceID FDCLithoProperties::nextSurface() const
{
  return m_nextSurface;
}

mbapi::StratigraphyManager::LayerID FDCLithoProperties::currentLayer() const
{
  return m_currentLayer;
}

std::string FDCLithoProperties::currentTopName() const
{
  return m_currentTopName;
}

std::string FDCLithoProperties::nextTopName() const
{
  return m_nextTopName;
}

std::string FDCLithoProperties::currentLayerName() const
{
  return m_currentLayerName;
}

void FDCLithoProperties::setManagers(mbapi::StratigraphyManager & stMgrLocal, mbapi::LithologyManager & litMgrLocal)
{
  m_stMgrLocal = &stMgrLocal;
  m_litMgrLocal = &litMgrLocal;
}

} // namespace fastDepthCalibration
