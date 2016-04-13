//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BpaRenderAreaListener.h"
#include "SceneExaminer.h"

#include <SceneGraphManager.h>
#include <CameraUtil.h>

#ifdef USE_H264
#include <RenderArea.h>
#include <Connection.h>
#include <ConnectionSettings.h>
#include <RenderAreaSettings.h>
#else
#include <RemoteViz/Rendering/RenderArea.h>
#include <RemoteViz/Rendering/Connection.h>
#include <RemoteViz/Rendering/RenderAreaSettings.h>
#endif

#include <Inventor/SoSceneManager.h>
#include <Inventor/nodes/SoGradientBackground.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>

#include <string>


void BpaRenderAreaListener::createSceneGraph(const std::string& id)
{
  std::cout << "Loading project, id = " << id << std::endl;
#ifdef WIN64
  const std::string rootdir = "V:/data/";
#else
  const std::string rootdir = "/home/ree/data/";
#endif
  const std::string path = rootdir + id;

  m_project = Project::load(path);
  m_projectInfo = m_project->getProjectInfo();

  std::cout << "Project loaded, building scenegraph" << std::endl;

  m_sceneGraphManager = std::make_shared<SceneGraphManager>();
  m_sceneGraphManager->setup(m_project);

  m_examiner = new SceneExaminer(m_sceneGraphManager);
  m_examiner->setFenceAddedCallback(std::bind(&BpaRenderAreaListener::onFenceAdded, this, std::placeholders::_1));

  m_commandHandler.setup(m_sceneGraphManager.get(), m_examiner.ptr());

  m_renderArea->getSceneManager()->setSceneGraph(m_examiner.ptr());
  m_examiner->viewAll(m_renderArea->getSceneManager()->getViewportRegion());

  std::cout << "...done" << std::endl;
}

void BpaRenderAreaListener::onFenceAdded(int fenceId)
{
  m_commandHandler.sendFenceAddedEvent(m_renderArea, fenceId);
}

BpaRenderAreaListener::BpaRenderAreaListener(RenderArea* renderArea)
: m_renderArea(renderArea)
, m_examiner(0)
, m_drawFaces(true)
, m_drawEdges(true)
, m_logEvents(true)
{
}

BpaRenderAreaListener::~BpaRenderAreaListener()
{
}

void BpaRenderAreaListener::onOpenedConnection(RenderArea* renderArea, Connection* connection)
{
  if (m_logEvents)
  {
    std::cout << "[BpaRenderAreaListener] onOpenedConnection("
      << "renderArea = " << renderArea->getId()
      << ", connection = " << connection->getId()
      << ")" << std::endl;
  }

  if(!m_sceneGraphManager)
    createSceneGraph(renderArea->getId());

  m_commandHandler.sendProjectInfo(renderArea, m_projectInfo);

  RemoteViz::Rendering::RenderAreaListener::onOpenedConnection(renderArea, connection);
}

void BpaRenderAreaListener::onClosedConnection(RenderArea* renderArea, const std::string& connectionId)
{
  if (m_logEvents)
  {
    std::cout << "[BpaRenderAreaListener] onClosedConnection("
      << "renderArea = " << renderArea->getId()
      << ", connection = " << connectionId
      << ")" << std::endl;
  }

  RenderAreaListener::onClosedConnection(renderArea, connectionId);
}

void BpaRenderAreaListener::onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message)
{
  if (m_logEvents)
  {
    std::cout << "[BpaRenderAreaListener] onReceivedMessage("
      << "renderArea = " << renderArea->getId()
      << ", message = " << message
      << ")" << std::endl;
  }

  m_commandHandler.onReceivedMessage(renderArea, sender, message);

  RemoteViz::Rendering::RenderAreaListener::onReceivedMessage(renderArea, sender, message);
}

void BpaRenderAreaListener::onResize(RenderArea* renderArea, unsigned int width, unsigned int height)
{
  if (m_logEvents)
  {
    std::cout << "[BpaRenderAreaListener] onResize("
      << "renderArea = " << renderArea->getId()
      << ", width = " << width
      << ", height = " << height
      << ")" << std::endl;
  }

  RemoteViz::Rendering::RenderAreaListener::onResize(renderArea, width, height);
}