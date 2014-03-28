#pragma once
#include <RemoteViz/Rendering/RenderAreaListener.h>

using namespace RemoteViz::Rendering;

namespace DataAccess
{
  namespace Interface
  {
    class ProjectHandle;
  }
}

class SceneGraph;

class BpaRenderAreaListener : public RenderAreaListener
{
  RenderArea* m_renderArea;
  SceneGraph* m_sceneGraph;

  DataAccess::Interface::ProjectHandle* m_handle;

  void createSceneGraph();

public:

  explicit BpaRenderAreaListener(RenderArea* renderArea);

  void sendProjectInfo() const;

  virtual void onOpenedConnection(RenderArea* renderArea, Connection* connection);

  virtual void onClosedConnection(RenderArea* renderArea, const std::string& connectionId);

  virtual void onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message);

  virtual void onRender(RenderArea* renderArea);

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

