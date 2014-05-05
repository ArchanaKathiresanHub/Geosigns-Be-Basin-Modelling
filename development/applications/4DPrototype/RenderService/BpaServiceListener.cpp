#include "BpaServiceListener.h"
#include "BpaRenderAreaListener.h"
#include "SceneExaminer.h"
#include "RenderService.h"

#include <Visualization/SceneGraph.h>

#include <RemoteViz/Rendering/RenderArea.h>

#include <Inventor/gestures/recognizers/SoScaleGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoRotateGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoLongTapGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoScaleGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoDoubleTapGestureRecognizer.h>

#include <Interface/ProjectHandle.h>

#include <iostream>

namespace di = DataAccess::Interface;

BpaServiceListener::BpaServiceListener(RenderService* renderService)
  : m_renderService(renderService)
{
  if(m_renderService != 0)
    m_renderService->logMessage("BpaServiceListener instantiated");
}

BpaServiceListener::~BpaServiceListener()
{
}

bool BpaServiceListener::onPendingCreateRenderArea(
  const std::string& renderAreaId, 
  unsigned int& width, 
  unsigned int& height, 
  Device* device, 
  ConnectionParameters* parameters)
{
  if(m_renderService != 0)
    m_renderService->logMessage("Accepting connection");

  std::cout << "Accepting connection, renderAreaId = '" << renderAreaId << "' (" << width << "x" << height << ")" << std::endl;
  return true; // accept connection
}

bool BpaServiceListener::onPendingShareRenderArea(RenderArea* renderArea, Device* device, ConnectionParameters* parameters)
{
  if(m_renderService != 0)
    m_renderService->logMessage("RenderArea shared");

  std::cout << "Accepting connection, shared renderAreaId = '" << renderArea->getId() << std::endl;
  return true; // accept connection
}
  
void BpaServiceListener::onInstantiatedRenderArea(RenderArea *renderArea)
{
  if(m_renderService != 0)
    m_renderService->logMessage("RenderArea instantiated");

  std::cout << "onInstantiatedRenderArea(renderArea = " << renderArea->getId() << std::endl;

	// Add recognizers for gesture events
  SoScaleGestureRecognizer *zoomRecognizer = new SoScaleGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(zoomRecognizer);
  SoRotateGestureRecognizer* rotateRecognizer = new SoRotateGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(rotateRecognizer);
  SoDoubleTapGestureRecognizer* doubleTapRecognizer = new SoDoubleTapGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(doubleTapRecognizer);
  SoLongTapGestureRecognizer* longTapRecognizer = new SoLongTapGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(longTapRecognizer);
	
	// Add the renderAreaListener instance as renderArea listener
  BpaRenderAreaListener* renderAreaListener = new BpaRenderAreaListener(renderArea);
	renderArea->addListener(renderAreaListener);
}

void BpaServiceListener::onDisposedRenderArea(const std::string& renderAreaId)
{
  std::cout << "Render area '" << renderAreaId << "' disposed" << std::endl;
}

void BpaServiceListener::onConnectedDevice(const std::string& deviceId)
{
  if(m_renderService != 0)
    m_renderService->logMessage("New device connected");

  std::cout << "Device '" << deviceId << "' connected" << std::endl;
}

void BpaServiceListener::onDisconnectedDevice(const std::string& deviceId)
{
  if(m_renderService != 0)
    m_renderService->logMessage("Device disconnected");

  std::cout << "Device '" << deviceId << "' disconnected" << std::endl;
}

void BpaServiceListener::onMissingLicense(const std::string& renderAreaId, ConnectionParameters* parameters)
{
  if(m_renderService != 0)
    m_renderService->logMessage("Missing license");

  std::cout << "License missing (renderAreaId = '" << renderAreaId << "')" << std::endl;
}

