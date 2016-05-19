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

BpaServiceListener::BpaServiceListener(RenderService* renderService)
  : m_renderService(renderService)
{
}

BpaServiceListener::~BpaServiceListener()
{
}

bool BpaServiceListener::onPendingCreateRenderArea(
  const std::string& renderAreaId,
  unsigned int& width,
  unsigned int& height,
  Client* client,
  ConnectionParameters* parameters)
{
  std::cout << "onPendingCreateRenderArea(renderArea = " << renderAreaId << ")" << std::endl;
  return true;
}

bool BpaServiceListener::onPendingShareRenderArea(RenderArea* renderArea, Client* client, ConnectionParameters* parameters)
{
  std::cout << "onPendingShareRenderArea(renderArea = " << renderArea->getId() << ")" << std::endl;
  return true;
}

void BpaServiceListener::onInstantiatedRenderArea(RenderArea *renderArea)
{
  std::cout << "onInstantiatedRenderArea(renderArea = " << renderArea->getId() << std::endl;

  renderArea->addListener(std::make_shared<BpaRenderAreaListener>(renderArea));
  renderArea->getTouchManager()->addDefaultRecognizers();
}

void BpaServiceListener::onDisposedRenderArea(const std::string& renderAreaId)
{
  std::cout << "onDisposedRenderArea(renderArea = " << renderAreaId << ")" << std::endl;
}

void BpaServiceListener::onConnectedClient(const std::string& clientId)
{
  std::cout << "onConnectedClient(clientId = " << clientId << ")" << std::endl;
}

void BpaServiceListener::onDisconnectedClient(const std::string& clientId)
{
  std::cout << "onDisconnectedClient(clientId= " << clientId << ")" << std::endl;
}

void BpaServiceListener::onMissingLicense(const std::string& renderAreaId, ConnectionParameters* parameters)
{
}
