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

#include <Visualization/Project.h>
#include <Visualization/SceneGraphManager.h>

//#include <RemoteViz/Rendering/RenderAreaListener.h>
#include <RenderAreaListener.h>

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

  bool m_drawFaces;
  bool m_drawEdges;

  void createSceneGraph(const std::string& id);

  void sendProjectInfo() const;

public:

  explicit BpaRenderAreaListener(RenderArea* renderArea);

  ~BpaRenderAreaListener();

  virtual void onOpenedConnection(RenderArea* renderArea, Connection* connection);

  virtual void onClosedConnection(RenderArea* renderArea, const std::string& connectionId);

  virtual void onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message);
};

#endif
