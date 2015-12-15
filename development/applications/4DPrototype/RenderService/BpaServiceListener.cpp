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
  //if(m_renderService != 0)
  //  m_renderService->logMessage("BpaServiceListener instantiated");
}

BpaServiceListener::~BpaServiceListener()
{
}

void BpaServiceListener::onInstantiatedRenderArea(RenderArea *renderArea)
{
  //if(m_renderService != 0)
  //  m_renderService->logMessage("RenderArea instantiated");
  std::cout << "onInstantiatedRenderArea(renderArea = " << renderArea->getId() << std::endl;

  renderArea->addListener(std::make_shared<BpaRenderAreaListener>(renderArea));
  renderArea->getTouchManager()->addDefaultRecognizers();
}

