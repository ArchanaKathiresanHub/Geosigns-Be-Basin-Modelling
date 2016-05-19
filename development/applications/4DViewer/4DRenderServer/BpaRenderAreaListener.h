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

#include "CommandHandler.h"
#include "SceneExaminer.h"

#include <Project.h>

#ifdef USE_H264
#include <RenderAreaListener.h>
#else
#include <RemoteViz/Rendering/RenderAreaListener.h>
#endif

#include <memory>

using namespace RemoteViz::Rendering;

class BpaRenderAreaListener : public RenderAreaListener
{
  std::string m_rootdir;
  std::shared_ptr<Project> m_project;
  Project::ProjectInfo m_projectInfo;

  std::shared_ptr<SceneGraphManager> m_sceneGraphManager;
  std::shared_ptr<SeismicScene> m_seismicScene;
  SoRef<SceneExaminer> m_examiner;

  RenderArea*       m_renderArea;
  CommandHandler    m_commandHandler;

  bool m_drawFaces;
  bool m_drawEdges;
  bool m_logEvents;

  void createSceneGraph(const std::string& id);

  void onFenceAdded(int fenceId);

public:

  explicit BpaRenderAreaListener(RenderArea* renderArea);

  ~BpaRenderAreaListener();

  void onOpenedConnection(RenderArea* renderArea, Connection* connection) override;

  void onClosedConnection(RenderArea* renderArea, const std::string& connectionId) override;

  void onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message) override;

  void onResize(RenderArea* renderArea, unsigned int width, unsigned int height) override;
};

#endif
