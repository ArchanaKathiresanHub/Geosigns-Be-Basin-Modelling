//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FDCLithoProperties.h"

#include "FDCProjectManager.h"

namespace fastDepthConversion
{

FDCLithoProperties::FDCLithoProperties(FDCProjectManager& fdcProjectManager) :
  m_fdcProjectManager{fdcProjectManager},
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
  mbapi::StratigraphyManager& stratManager = m_fdcProjectManager.getStratManager();
  if (ErrorHandler::ReturnCode::NoError != stratManager.layerLithologiesList(m_currentLayer, m_lithoList, m_lithoPercent, m_lithoPercMap))
  {
    throw ErrorHandler::Exception(stratManager.errorCode()) << "Cannot read the lithologies for the current layer: " << stratManager.errorMessage();
  }
}

void FDCLithoProperties::setLithoSurfaces(const mbapi::StratigraphyManager::SurfaceID currentSurface)
{
  m_nextSurface = currentSurface + 1;
  m_currentLayer = currentSurface;
  m_nextLayer = m_nextSurface;

  mbapi::StratigraphyManager& stratManager = m_fdcProjectManager.getStratManager();
  m_currentTopName = stratManager.surfaceName(m_currentLayer);
  m_nextTopName = stratManager.surfaceName(m_nextLayer);
  m_currentLayerName = stratManager.layerName(m_currentLayer);
}

double FDCLithoProperties::getMaxSeismicVelocityForCurrentLayer() const
{
  mbapi::LithologyManager& lithoManager = m_fdcProjectManager.getLithoManager();

  double maxSeisVel = 0.0;
  for (const std::string& lith : m_lithoList)
  {
    if ( lith.empty() ) { continue; }

    const mbapi::LithologyManager::LithologyID lithID = lithoManager.findID(lith);
    if ( IsValueUndefined( lithID ) )
    {
      throw ErrorHandler::Exception(lithoManager.errorCode()) << "Cannot find the id for the lithology " << lith
                                                                << ", " << lithoManager.errorMessage();
    }

    const double seisVel = lithoManager.seisVelocity(lithID);
    if ( IsValueUndefined( seisVel ) )
    {
      throw ErrorHandler::Exception(lithoManager.errorCode()) << "Cannot find the seismic velocity for the lithology "
                                                                << lith << ", " << lithoManager.errorMessage();
    }

    if (maxSeisVel < seisVel) { maxSeisVel = seisVel; }
  }

  return maxSeisVel;
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

} // namespace fastDepthConversion
