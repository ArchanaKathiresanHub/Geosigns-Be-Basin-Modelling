//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef BPARENDERAREALISTENER_H_INCLUDED
#define BPARENDERAREALISTENER_H_INCLUDED

class SceneGraphManager;
class SeismicScene;

struct LoadProjectTask;

namespace jsonxx { class Object; }

#include "CommandHandler.h"
#include "SceneExaminer.h"

#include <Project.h>
#include <Scheduler.h>

#ifdef USE_H264
#include <RenderAreaListener.h>
#else
#include <RemoteViz/Rendering/RenderAreaListener.h>
#endif

#include <memory>

using namespace RemoteViz::Rendering;

class BpaRenderAreaListener 
  : public RenderAreaListener
  , public TaskSource
{
  Scheduler& m_scheduler;

  std::string m_rootdir;
  std::string m_projectdir;

  std::shared_ptr<LoadProjectTask> m_loadTask;
  std::shared_ptr<Project> m_project;
  Project::ProjectInfo m_projectInfo;

  std::shared_ptr<SceneGraphManager> m_sceneGraphManager;
  std::shared_ptr<SeismicScene> m_seismicScene;
  SoRef<SceneExaminer> m_examiner;

  RenderArea*    m_renderArea;
  CommandHandler m_commandHandler;

  bool m_logEvents;

  void loadSeismic();
  void createSceneGraph();

  void onFenceAdded(int fenceId);
  void onConnectionCountChanged();

  jsonxx::Object createProjectLoadedEvent() const;
  jsonxx::Object createConnectionCountEvent() const;
  jsonxx::Object createFenceAddedEvent(int fenceId) const;

  void setupProject(const std::string& id);

public:

  BpaRenderAreaListener(RenderArea* renderArea, Scheduler& scheduler);

  ~BpaRenderAreaListener();

  void onProjectLoaded(std::shared_ptr<Project> project);

  void setDataDir(const std::string& dir);

  void onOpenedConnection(RenderArea* renderArea, Connection* connection) override;

  void onClosedConnection(RenderArea* renderArea, const std::string& connectionId) override;

  void onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message) override;

  void onRequestedSize(RenderArea* renderArea, Connection* sender, unsigned int width, unsigned int height) override;

  void onTaskCompleted(Task& task) override;
};

#endif
