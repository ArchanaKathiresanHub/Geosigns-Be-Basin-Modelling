//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BpaServiceListener.h"
#include "BpaRenderAreaListener.h"
#include "RenderService.h"

#ifdef USE_H264
#include <RenderArea.h>
#else
#include <RemoteViz/Rendering/RenderArea.h>
#endif

#include <Inventor/touch/SoTouchManager.h>

#include <iostream>

#include <boost/log/trivial.hpp>

BpaServiceListener::BpaServiceListener(RenderService* renderService)
  : m_renderService(renderService)
{
}

BpaServiceListener::~BpaServiceListener()
{
}

void BpaServiceListener::setDataDir(const std::string& dir)
{
  m_datadir = dir;
}

bool BpaServiceListener::onPendingCreateRenderArea(
  const std::string& renderAreaId,
  unsigned int& width,
  unsigned int& height,
  Client* client,
  ConnectionParameters* parameters)
{
  width = 1280;
  height = 720;
  BOOST_LOG_TRIVIAL(trace) << "about to create render area " << renderAreaId << "(" << width << " x " << height << ")";
  return true;
}

bool BpaServiceListener::onPendingShareRenderArea(RenderArea* renderArea, Client* client, ConnectionParameters* parameters)
{
  BOOST_LOG_TRIVIAL(trace) << "about to share render area " << renderArea->getId();
  return true;
}

void BpaServiceListener::onInstantiatedRenderArea(RenderArea *renderArea)
{
  BOOST_LOG_TRIVIAL(trace) << "instantiated render area " << renderArea->getId();

  auto listener = std::make_shared<BpaRenderAreaListener>(renderArea);
  listener->setDataDir(m_datadir);
  renderArea->addListener(listener);
  renderArea->getTouchManager()->addDefaultRecognizers();
}

void BpaServiceListener::onDisposedRenderArea(const std::string& renderAreaId)
{
  BOOST_LOG_TRIVIAL(trace) << "disposed render area " << renderAreaId;
}

void BpaServiceListener::onConnectedClient(const std::string& clientId)
{
  BOOST_LOG_TRIVIAL(trace) << "client " << clientId << " connected";
}

void BpaServiceListener::onDisconnectedClient(const std::string& clientId)
{
  BOOST_LOG_TRIVIAL(trace) << "client " << clientId << " disconnected";
}

void BpaServiceListener::onMissingLicense(const std::string& renderAreaId, ConnectionParameters* parameters)
{
}
