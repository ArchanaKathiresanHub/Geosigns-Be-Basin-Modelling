//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FDCLithoProperties class stores the lithology and stratigraphy data and handles any related functionality.
#pragma once

#include "cmbAPI.h"

namespace fastDepthConversion
{

class FDCLithoProperties
{
public:
  explicit FDCLithoProperties(mbapi::StratigraphyManager & stMgrLocal, mbapi::LithologyManager & litMgrLocal);

  void setlayerLithologiesListForCurrentLayer();
  void setLithoSurfaces(const mbapi::StratigraphyManager::SurfaceID currentSurface);
  double getMaxSeismicVelocityForCurrentLayer() const;
  std::vector<mbapi::StratigraphyManager::SurfaceID> surfacesIDs() const;

  mbapi::StratigraphyManager::SurfaceID nextSurface() const;
  mbapi::StratigraphyManager::LayerID currentLayer() const;
  std::string currentTopName() const;
  std::string nextTopName() const;
  std::string currentLayerName() const;
  std::string surfaceNameCurrentLayer() const;
  std::string surfaceNameNextLayer() const;
  void setManagers(mbapi::StratigraphyManager & stMgrLocal, mbapi::LithologyManager & litMgrLocal);

private:
  mbapi::StratigraphyManager * m_stMgrLocal;
  mbapi::LithologyManager *    m_litMgrLocal;

  mbapi::StratigraphyManager::SurfaceID m_nextSurface;
  mbapi::StratigraphyManager::LayerID   m_currentLayer;
  mbapi::StratigraphyManager::LayerID   m_nextLayer;

  std::string m_currentTopName;
  std::string m_nextTopName;
  std::string m_currentLayerName;

  std::vector<std::string> m_lithoList;
  std::vector<double>      m_lithoPercent;
  std::vector<std::string> m_lithoPercMap;
};

} // namespace fastDepthConversion
