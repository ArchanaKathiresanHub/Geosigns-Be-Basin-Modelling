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

#include <RemoteViz/Rendering/RenderAreaListener.h>

#include <memory>

#include <Interface/Interface.h>

using namespace RemoteViz::Rendering;

class SceneExaminer;

class BpaRenderAreaListener : public RenderAreaListener
{
  std::shared_ptr<DataAccess::Interface::ObjectFactory> m_factory;
  std::shared_ptr<DataAccess::Interface::ProjectHandle> m_handle;

  std::shared_ptr<Project> m_project;
  Project::ProjectInfo m_projectInfo;

  SceneGraphManager m_sceneGraphManager;
  RenderArea*       m_renderArea;
  SceneExaminer*    m_examiner;

  bool m_drawFaces;
  bool m_drawEdges;

  void createSceneGraph(const std::string& id);

public:

  explicit BpaRenderAreaListener(RenderArea* renderArea);

  ~BpaRenderAreaListener();

  void sendProjectInfo() const;

  virtual void onOpenedConnection(RenderArea* renderArea, Connection* connection);

  virtual void onClosedConnection(RenderArea* renderArea, const std::string& connectionId);

  virtual void onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message);

  virtual bool onPreRender(RenderArea* renderArea, bool& clearWindow, bool& clearZBuffer);

  virtual void onPostRender(RenderArea* renderArea);

  virtual void onResize(RenderArea* renderArea, unsigned int width, unsigned int height);

  virtual void onRequestedSize(RenderArea* renderArea, Connection* sender, unsigned int width, unsigned int height);

  virtual bool onMouseUp(int x, int y, int button);

  virtual bool onMouseDown(int x, int y, int button);

  virtual bool onMouseMove(int x, int y);

  virtual bool onMouseEnter(int x, int y);

  virtual bool onMouseLeave(int x, int y);

  virtual bool onMouseWheel(int delta);

  virtual bool onKeyUp(const std::string& key);

  virtual bool onKeyDown(const std::string& key);

  virtual bool onTouchStart(unsigned int id, int x, int y);

  virtual bool onTouchEnd(unsigned int id, int x, int y);

  virtual bool onTouchMove(unsigned int id, int x, int y);

};

#endif
