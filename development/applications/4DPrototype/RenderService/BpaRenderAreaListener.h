#pragma once
#include <RemoteViz/Rendering/RenderAreaListener.h>

#include <memory>

#include <Interface/Interface.h>

using namespace RemoteViz::Rendering;


class SceneGraph;
class SceneExaminer;

class BpaRenderAreaListener : public RenderAreaListener
{
  RenderArea*    m_renderArea;
  SceneGraph*    m_sceneGraph;
  SceneExaminer* m_examiner;

  std::unique_ptr<DataAccess::Interface::ObjectFactory> m_factory;
  std::unique_ptr<DataAccess::Interface::ProjectHandle> m_handle;

  void createSceneGraph(const std::string& id);

public:

  explicit BpaRenderAreaListener(RenderArea* renderArea);

  ~BpaRenderAreaListener();

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

