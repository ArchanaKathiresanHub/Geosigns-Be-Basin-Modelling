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

#include "CommandHandler.h"

#include <Project.h>
#include <SceneGraphManager.h>

#ifdef USE_H264
#include <RenderAreaListener.h>
#else
#include <RemoteViz/Rendering/RenderAreaListener.h>
#endif

#include <memory>

using namespace RemoteViz::Rendering;

class SceneExaminer;

class BpaRenderAreaListener : public RenderAreaListener
{
  std::shared_ptr<Project> m_project;
  Project::ProjectInfo m_projectInfo;

  SceneGraphManager m_sceneGraphManager;
  RenderArea*       m_renderArea;
  SceneExaminer*    m_examiner;
  CommandHandler    m_commandHandler;

  bool m_drawFaces;
  bool m_drawEdges;
  bool m_logEvents;

  void createSceneGraph(const std::string& id);

public:

  explicit BpaRenderAreaListener(RenderArea* renderArea);

  ~BpaRenderAreaListener();

  virtual void onOpenedConnection(RenderArea* renderArea, Connection* connection);

  virtual void onClosedConnection(RenderArea* renderArea, const std::string& connectionId);

  virtual void onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message);

  virtual void onResize(RenderArea* renderArea, unsigned int width, unsigned int height);
};

#endif
