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
#include "Scheduler.h"
#include "JSONUtil.h"

#include <SceneGraphManager.h>
#include <Seismic.h>
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

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

struct LoadProjectTask : public Task
{
  //BpaRenderAreaListener* listener;

  std::string projectFile;
  std::shared_ptr<Project> project;

  void run() override
  {
    try
    {
      BOOST_LOG_TRIVIAL(trace) << "Loading project " << projectFile;
      project = Project::load(projectFile);
    }
    catch(std::runtime_error&)
    {
      BOOST_LOG_TRIVIAL(trace) << "Error loading project";
      error = true;
    }
  }
};

void BpaRenderAreaListener::loadSeismic()
{
  // Try to find seismic data
  boost::filesystem::path ldmFile;
  boost::filesystem::directory_iterator iter(m_projectdir);
  boost::filesystem::directory_iterator end;
  while (iter != end && iter->path().extension() != ".ldm")
    ++iter;

  if (iter != end)
    ldmFile = *iter;

  if (!ldmFile.empty())
  {
    BOOST_LOG_TRIVIAL(trace) << "found LDM file " << ldmFile;

    std::string ldmFileStr = ldmFile.string();
    auto dim = m_project->getProjectInfo().dimensions;
    m_seismicScene = std::make_shared<SeismicScene>(ldmFileStr.c_str(), dim);
    m_sceneGraphManager->addSeismicScene(m_seismicScene);
  }
}

void BpaRenderAreaListener::createSceneGraph()
{
  assert(m_project);

  m_sceneGraphManager = std::make_shared<SceneGraphManager>(m_scheduler);
  m_sceneGraphManager->setup(m_project);

  m_examiner->setSceneGraphManager(m_sceneGraphManager);
  m_examiner->setFenceAddedCallback(std::bind(&BpaRenderAreaListener::onFenceAdded, this, std::placeholders::_1));

  loadSeismic();

  m_commandHandler.setup(
    m_sceneGraphManager.get(),
    m_seismicScene.get(),
    m_examiner.ptr());

  //m_renderArea->getSceneManager()->setSceneGraph(m_examiner.ptr());
  m_examiner->viewAll(m_renderArea->getSceneManager()->getViewportRegion());
}

jsonxx::Object BpaRenderAreaListener::createProjectLoadedEvent() const
{
  jsonxx::Object params;
  params << "projectInfo" << toJSON(m_projectInfo);

  auto mode = m_examiner->getCameraMode();
  std::string projection = (mode == SceneInteractor::PERSPECTIVE)
    ? "perspective"
    : "orthographic";

  params << "projection" << projection;

  if(m_seismicScene)
  {
    SeismicScene::Info info;
    info.extent = m_seismicScene->getExtent();
    info.size = m_seismicScene->getDimensions();

    params
      << "seismicInfo" << toJSON(info)
      << "seismicState" << toJSON(m_seismicScene->getViewState());
  }

  params << "areaState" << toJSON(*m_renderArea->getSettings());
  params << "viewState" << toJSON(m_sceneGraphManager->getViewState());

  jsonxx::Object event;
  event << "type" << "projectLoaded";
  event << "params" << params;

  return event;
}

jsonxx::Object BpaRenderAreaListener::createConnectionCountEvent() const
{
  jsonxx::Object params;
  params << "count" << m_renderArea->getNumConnections();

  jsonxx::Object event;
  event << "type" << "connectionCountChanged";
  event << "params" << params;

  return event;
}

jsonxx::Object BpaRenderAreaListener::createFenceAddedEvent(int fenceId) const
{
  jsonxx::Object params;
  params << "fenceId" << fenceId;

  jsonxx::Object event;
  event << "type" << "fenceAdded";
  event << "params" << params;

  return event;
}

void BpaRenderAreaListener::setupProject(const std::string& id)
{
  boost::filesystem::path projectFile(m_rootdir);
  projectFile.append(id);

  m_projectdir = projectFile.parent_path().string();

  m_loadTask = std::make_shared<LoadProjectTask>();
  m_loadTask->projectFile = projectFile.string();
  m_scheduler.put(m_loadTask);
}

void BpaRenderAreaListener::onFenceAdded(int fenceId)
{
  jsonxx::Object msg;
  msg << "event" << createFenceAddedEvent(fenceId);

  m_renderArea->sendMessage(msg.write(jsonxx::JSON));
}

void BpaRenderAreaListener::onConnectionCountChanged()
{
  jsonxx::Object msg;
  msg << "event" << createConnectionCountEvent();

  m_renderArea->sendMessage(msg.write(jsonxx::JSON));
}

void BpaRenderAreaListener::onProjectLoaded(std::shared_ptr<Project> project)
{
  BOOST_LOG_TRIVIAL(trace) << "Project loaded";

  m_project = project;
  m_loadTask.reset();

  if(m_project)
  {
    m_projectInfo = m_project->getProjectInfo();

    createSceneGraph();

    jsonxx::Object msg;
    msg << "event" << createProjectLoadedEvent();
    m_renderArea->sendMessage(msg.write(jsonxx::JSON));
  }
}

BpaRenderAreaListener::BpaRenderAreaListener(RenderArea* renderArea, Scheduler& scheduler)
  : m_scheduler(scheduler)
  , m_renderArea(renderArea)
  , m_commandHandler(renderArea)
  , m_examiner(0)
  , m_logEvents(true)
{
}

BpaRenderAreaListener::~BpaRenderAreaListener()
{
}

void BpaRenderAreaListener::setDataDir(const std::string& dir)
{
  m_rootdir = dir;
}

void BpaRenderAreaListener::onOpenedConnection(RenderArea* renderArea, Connection* connection)
{
  if (m_logEvents)
    BOOST_LOG_TRIVIAL(trace) << "new connection opened on render area " << renderArea->getId() << ", id = " << connection->getId();

  onConnectionCountChanged();

  if(m_project)
  {
    jsonxx::Object msg;
    msg << "event" << createProjectLoadedEvent();
    connection->sendMessage(msg.write(jsonxx::JSON));
  }
  else
  {
    if(!m_loadTask)
    {
      m_examiner = new SceneExaminer;
      m_renderArea->getSceneManager()->setSceneGraph(m_examiner.ptr());
      m_examiner->viewAll(m_renderArea->getSceneManager()->getViewportRegion());

      setupProject(renderArea->getId());
    }
  }

  RenderAreaListener::onOpenedConnection(renderArea, connection);
}

void BpaRenderAreaListener::onClosedConnection(RenderArea* renderArea, const std::string& connectionId)
{
  if (m_logEvents)
    BOOST_LOG_TRIVIAL(trace) << "connection " << connectionId << " closed";

  onConnectionCountChanged();

  if(renderArea->getNumConnections() == 0 && m_loadTask)
  {
    BOOST_LOG_TRIVIAL(trace) << "Canceling project loading task";
    m_loadTask->canceled = true;
    m_loadTask.reset();
  }
    
  RenderAreaListener::onClosedConnection(renderArea, connectionId);
}

void BpaRenderAreaListener::onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message)
{
  m_commandHandler.onReceivedMessage(sender, message);

  RenderAreaListener::onReceivedMessage(renderArea, sender, message);
}

void BpaRenderAreaListener::onRequestedSize(RenderArea* renderArea, Connection* sender, unsigned int width, unsigned int height)
{
  if(m_logEvents)
    BOOST_LOG_TRIVIAL(trace) << "requested resize render area " << renderArea->getId() << " to " << width << " x " << height << " (DENIED)";
}

void BpaRenderAreaListener::onTaskCompleted(Task& task)
{
  BOOST_LOG_TRIVIAL(trace) << "task completed";
  onProjectLoaded(static_cast<LoadProjectTask&>(task).project);
}
