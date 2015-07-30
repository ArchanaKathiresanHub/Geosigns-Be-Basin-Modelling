#include "BpaRenderAreaListener.h"
#include "SceneExaminer.h"

#include <Visualization/SceneGraph.h>
#include <Visualization/SnapshotNode.h>

#include <RemoteViz/Rendering/RenderArea.h>
#include <RemoteViz/Rendering/Connection.h>
#include <RemoteViz/Rendering/RenderAreaSettings.h>

#include <Inventor/SoSceneManager.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoGradientBackground.h>

#include <Interface/ProjectHandle.h>
#include <Interface/ObjectFactory.h>
#include <Interface/Property.h>

#include <string>
#include <list>
#include <sstream>

using namespace std;

namespace di = DataAccess::Interface;

namespace
{
  std::list<std::string> &split(const std::string &s, char delim, std::list<std::string> &elems) 
  {
    stringstream ss(s);
    string item;

    while (getline(ss, item, delim)) 
      elems.push_back(item);

    return elems;
  }
}

void BpaRenderAreaListener::createSceneGraph(const std::string& id)
{
  std::cout << "Loading scenegraph..."<< std::endl;

  //const std::string rootdir = "E:/Data/";
  const std::string rootdir = "/home/ree/CauldronSmall";
  const std::string filename = "/Project.project3d";
  std::string path = rootdir + filename;

  m_handle.reset(di::OpenCauldronProject(path, "r", m_factory.get()));

  std::cout << "Project loaded!" << std::endl;

  SceneGraph* sceneGraph = new SceneGraph;
  sceneGraph->setup(m_handle.get());
  sceneGraph->RenderMode = SnapshotNode::RenderMode_Skin;
  m_sceneGraph = sceneGraph;

  SoGradientBackground* background = new SoGradientBackground;
  background->color0 = SbColor(.1f, .1f, .2f);
  background->color1 = SbColor(.2f, .2f, .5f);

  m_examiner = new SceneExaminer();
  m_examiner->addChild(background);
  m_examiner->addChild(m_sceneGraph);

  // Apply the sceneExaminer node as renderArea scene graph
  m_renderArea->getSceneManager()->setSceneGraph(m_examiner);

  // viewall
  m_examiner->viewAll(m_renderArea->getSceneManager()->getViewportRegion());

  std::cout << "...done" << std::endl;
}

BpaRenderAreaListener::BpaRenderAreaListener(RenderArea* renderArea)
  : m_renderArea(renderArea)
  , m_sceneGraph(0)
  , m_examiner(0)
  , m_factory(new di::ObjectFactory)
  , m_handle(0)
{
}

BpaRenderAreaListener::~BpaRenderAreaListener()
{
}

void BpaRenderAreaListener::sendProjectInfo() const
{
  std::string msg = "{ \"projectInfo\": { ";

  msg += "\"snapshotCount\": " + std::to_string((long long)m_sceneGraph->snapshotCount());
  msg += ", ";
  msg += "\"numI\": " + std::to_string((long long)m_sceneGraph->numI());
  msg += ", ";
  msg += "\"numJ\": " + std::to_string((long long)m_sceneGraph->numJ());
  msg += ", ";
  msg += "\"numIHiRes\": " + std::to_string((long long)m_sceneGraph->numIHiRes());
  msg += ", ";
  msg += "\"numJHiRes\": " + std::to_string((long long)m_sceneGraph->numJHiRes());
  msg += ", ";
  msg += "\"properties\": [";

  // Add properties to parent node
  int flags = di::FORMATION;
  int type = di::VOLUME;

  std::shared_ptr<di::PropertyList> properties(m_handle->getProperties(true, flags));
  if(!properties->empty())
    msg += "\"" + (*properties)[0]->getName() + "\"";
  for(size_t i=1; i < properties->size(); ++i)
    msg += ", \"" + (*properties)[i]->getName() + "\"";

  msg += "] } }";

  m_renderArea->sendMessage(msg);
}

void BpaRenderAreaListener::onOpenedConnection(RenderArea* renderArea, Connection* connection)
{
  std::cout << "[BpaRenderAreaListener] onOpenedConnection(renderArea = " << renderArea->getId() << ", connection = " << connection->getId() << ")" << std::endl;

  if(m_sceneGraph == 0)
    createSceneGraph(renderArea->getId());
  sendProjectInfo();
}

void BpaRenderAreaListener::onClosedConnection(RenderArea* renderArea, const std::string& connectionId)
{
  std::cout << "[BpaRenderAreaListener] onClosedConnection(renderArea = " << renderArea->getId() << ", connection = " << connectionId << ")" << std::endl;
  if(renderArea->getNumConnections() == 0)
  {
    //renderArea->dispose();
  }
}

void BpaRenderAreaListener::onReceivedMessage(RenderArea* renderArea, Connection* sender, const string& message)
{
  std::cout << "[BpaRenderAreaListener] onReceivedMessage(renderArea = " << renderArea->getId() << ", message = " << message << ")" << std::endl;
  SoSeparator* root = (SoSeparator*) renderArea->getSceneManager()->getSceneGraph();
  
  std::list<std::string> elems;
  split(message, ' ', elems); // split the received message, pattern : "COMMAND ARGUMENT"

  string command = elems.front();
  elems.pop_front();
  string argument;
  if(!elems.empty())
    argument = elems.front();

  // parse the commands
  if (command == "FPS")
  {
    //renderArea->getSettings()->setMaxSendingFPS(atoi(argument.c_str()));
  }
  else if (command == "STILLQUALITY")
  {
    renderArea->getSettings()->setStillCompressionQuality((float)atof(argument.c_str())); 
  }
  else if (command == "SCALEFACTOR")
  {
    renderArea->getSettings()->setInteractiveScaleFactor((float)atof(argument.c_str()));
  }
  else if (command == "INTERACTIVEQUALITY")
  {
    renderArea->getSettings()->setInteractiveCompressionQuality((float)atof(argument.c_str())); 
  }
  else if (command == "WIDTH")
  {
    renderArea->resize(atoi(argument.c_str()),renderArea->getSceneManager()->getSize()[1]);
  }
  else if (command == "HEIGHT")
  {
    renderArea->resize(renderArea->getSceneManager()->getSize()[0], atoi(argument.c_str()));
  }
  else if(command == "SNAPSHOT")
  {
    int index = atoi(argument.c_str());
    m_sceneGraph->setCurrentSnapshot(index);
  }
  else if(command == "RENDERMODE")
  {
    if(argument == "skin")
      m_sceneGraph->RenderMode = SnapshotNode::RenderMode_Skin;
    else
      m_sceneGraph->RenderMode = SnapshotNode::RenderMode_Slices;
  }
  else if(command == "MESHMODE")
  {
    if(argument == "formations")
      m_sceneGraph->setMeshMode(SceneGraph::MeshMode_All);
    else
      m_sceneGraph->setMeshMode(SceneGraph::MeshMode_Reservoirs);
  }
  else if(command == "VSCALE")
  {
    int scale = atoi(argument.c_str());
    m_sceneGraph->setVerticalScale((float)scale);
  }
  else if(command == "SLICEI")
  {
    int index = atoi(argument.c_str());
    m_sceneGraph->SliceI = index;
  }
  else if(command == "SLICEJ")
  {
    int index = atoi(argument.c_str());
    m_sceneGraph->SliceJ = index;
  }
  else if(command == "SETPROPERTY")
  {
    const di::Property* prop = m_handle->findProperty(argument);
    if(prop != 0)
      m_sceneGraph->setProperty(prop);
  }
  else if(command == "DRAWFACES")
  {
    bool drawFaces, drawEdges;
    m_sceneGraph->getRenderStyle(drawFaces, drawEdges);
    drawFaces = (argument == "TRUE");

    m_sceneGraph->setRenderStyle(drawFaces, drawEdges);
  }
  else if(command == "DRAWEDGES")
  {
    bool drawFaces, drawEdges;
    m_sceneGraph->getRenderStyle(drawFaces, drawEdges);
    drawEdges = (argument == "TRUE");

    m_sceneGraph->setRenderStyle(drawFaces, drawEdges);
  }
  else if(command == "VIEWALL")
  {
    SbViewportRegion vpregion = m_renderArea->getSceneManager()->getViewportRegion();
    m_examiner->viewAll(vpregion);
  }
}

void BpaRenderAreaListener::onRender(RenderArea* renderArea)
{
  //std::cout << "[BpaRenderAreaListener] onRender(renderArea = " << renderArea->getId() << ")" << std::endl;
}

void BpaRenderAreaListener::onResize(RenderArea* renderArea, unsigned int width, unsigned int height)
{
  std::cout << "[BpaRenderAreaListener] onResize(renderArea = " << renderArea->getId() << ", width = " << width << ", height = " << height << ")" << std::endl;
}

void BpaRenderAreaListener::onRequestedSize(RenderArea* renderArea, Connection* sender, unsigned int width, unsigned int height)
{
  std::cout << "[BpaRenderAreaListener] onRequestedResize(renderArea = " 
	    << renderArea->getId() 
	    << ", connection = " << sender->getId() 
	    << ", width = " << width 
	    << ", height = " << height << ")" << std::endl;

  renderArea->resize(width, height);
}

bool BpaRenderAreaListener::onMouseUp(int x, int y, int button)
{
  return true;
}

bool BpaRenderAreaListener::onMouseDown(int x, int y, int button)
{
  return true;
}

bool BpaRenderAreaListener::onMouseMove(int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onMouseEnter(int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onMouseLeave(int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onMouseWheel(int delta)
{
  return true;
}

bool BpaRenderAreaListener::onKeyUp(const std::string& key)
{
  return true;
}

bool BpaRenderAreaListener::onKeyDown(const std::string& key)
{
  return true;
}

bool BpaRenderAreaListener::onTouchStart(unsigned int id, int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onTouchEnd(unsigned int id, int x, int y)
{
  return true;
}

bool BpaRenderAreaListener::onTouchMove(unsigned int id, int x, int y)
{
  return true;
}

