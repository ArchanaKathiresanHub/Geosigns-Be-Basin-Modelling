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
#include "jsonxx.h"

#include <Visualization/SceneGraphManager.h>

#include <RemoteViz/Rendering/RenderArea.h>
#include <RemoteViz/Rendering/Connection.h>
#include <RemoteViz/Rendering/RenderAreaSettings.h>

#include <Inventor/SoSceneManager.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoGradientBackground.h>

#include <Interface/ProjectHandle.h>
#include <Interface/ObjectFactory.h>
#include <Interface/Property.h>
#include <Interface/Formation.h>
#include <Interface/Surface.h>
#include <Interface/FaultCollection.h>
#include <Interface/Faulting.h>

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

void BpaRenderAreaListener::createSceneGraph(const std::string& /*id*/)
{
  std::cout << "Loading scenegraph..."<< std::endl;

  const std::string rootdir = "V:/data/";
  //const std::string rootdir = "/home/ree/";
  const std::string project = "CauldronSmall";
  const std::string filename = "/Project.project3d";

  std::string path = rootdir + project + filename;

  m_handle.reset(di::OpenCauldronProject(path, "r", m_factory.get()));

  if (!m_handle)
  {
    std::cout << "Failed to load project!" << std::endl;
    return;
  }

  std::cout << "Project loaded!" << std::endl;

  m_sceneGraphManager.setup(m_handle.get());

  SoGradientBackground* background = new SoGradientBackground;
  background->color0 = SbColor(.1f, .1f, .2f);
  background->color1 = SbColor(.2f, .2f, .5f);

  m_examiner = new SceneExaminer();
  m_examiner->addChild(background);
  m_examiner->addChild(m_sceneGraphManager.getRoot());

  // Apply the sceneExaminer node as renderArea scene graph
  m_renderArea->getSceneManager()->setSceneGraph(m_examiner);

  // viewall
  m_examiner->viewAll(m_renderArea->getSceneManager()->getViewportRegion());

  std::cout << "...done" << std::endl;
}

BpaRenderAreaListener::BpaRenderAreaListener(RenderArea* renderArea)
  : m_renderArea(renderArea)
  , m_examiner(0)
  , m_factory(new di::ObjectFactory)
  , m_handle((di::ProjectHandle*)0)
  , m_drawFaces(true)
  , m_drawEdges(true)
{
}

BpaRenderAreaListener::~BpaRenderAreaListener()
{
}

void BpaRenderAreaListener::sendProjectInfo() const
{
  std::string msg = "{ \"projectInfo\": { ";

  msg += "\"snapshotCount\": " + std::to_string((long long)m_sceneGraphManager.getSnapshotCount());
  msg += ", ";
  msg += "\"numI\": " + std::to_string((long long)m_sceneGraphManager.numI());
  msg += ", ";
  msg += "\"numJ\": " + std::to_string((long long)m_sceneGraphManager.numJ());
  msg += ", ";
  msg += "\"numIHiRes\": " + std::to_string((long long)m_sceneGraphManager.numIHiRes());
  msg += ", ";
  msg += "\"numJHiRes\": " + std::to_string((long long)m_sceneGraphManager.numJHiRes());
  msg += ", ";

  // formations
  msg += "\"formations\": [";
  std::unique_ptr<di::FormationList> formations(m_handle->getFormations(0, false));
  for (size_t i = 0; i < formations->size(); ++i)
  {
    msg += "\"" + (*formations)[i]->getName() + "\"";
    if (i < formations->size() - 1)
      msg += ", ";
  }
  msg += "], ";
  
  // surfaces
  msg += "\"surfaces\": [";
  std::unique_ptr<di::SurfaceList> surfaces(m_handle->getSurfaces(0, false));
  for (size_t i = 0; i < surfaces->size(); ++i)
  {
    msg += "\"" + (*surfaces)[i]->getName() + "\"";
    if (i < surfaces->size() - 1)
      msg += ", ";
  }
  msg += "], ";

  // faults
  msg += "\"faultCollections\": [";
  std::unique_ptr<di::FaultCollectionList> faultCollections(m_handle->getFaultCollections(0));
  if (faultCollections && !faultCollections->empty())
  {
    for (size_t i = 0; i < faultCollections->size(); ++i)
    {
      msg += "{ \"name\": \"" + (*faultCollections)[i]->getName() + "\", \"faults\": [";

      std::unique_ptr<di::FaultList> faults((*faultCollections)[i]->getFaults());
      for (size_t j = 0; j < faults->size(); ++j)
      {
        msg += "\"" + (*faults)[j]->getName() + "\"";
        if (j < faults->size() - 1)
          msg += ", ";
      }
      msg += "]}";

      if (i < faultCollections->size() - 1)
        msg += ", ";
    }
  }
  msg += "], ";

  // properties
  msg += "\"properties\": [";

  const int allFlags = di::FORMATION | di::SURFACE | di::RESERVOIR | di::FORMATIONSURFACE;
  const int allTypes = di::MAP | di::VOLUME;

  std::unique_ptr<di::PropertyList> properties(m_handle->getProperties(true));

  // Get the list of property names for which values are available
  std::vector<std::string> propertyNames;
  for (size_t i = 1; i < properties->size(); ++i)
  {
    const di::Property* prop = (*properties)[i];
    if (prop->hasPropertyValues(allFlags, 0, 0, 0, 0, allTypes))
      propertyNames.push_back(prop->getName());
  }

  for (size_t i = 0; i < propertyNames.size(); ++i)
  {
    msg += "\"" + propertyNames[i] + "\"";
    if (i < propertyNames.size() - 1)
      msg += ", ";
  }
  msg += "] } }";

  m_renderArea->sendMessage(msg);
}

void BpaRenderAreaListener::onOpenedConnection(RenderArea* renderArea, Connection* connection)
{
  std::cout << "[BpaRenderAreaListener] onOpenedConnection(renderArea = " << renderArea->getId() << ", connection = " << connection->getId() << ")" << std::endl;

  if(m_sceneGraphManager.getRoot() == 0)
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

void BpaRenderAreaListener::onReceivedMessage(RenderArea* renderArea, Connection* /*sender*/, const string& message)
{
  std::cout << "[BpaRenderAreaListener] onReceivedMessage(renderArea = " << renderArea->getId() << ", message = " << message << ")" << std::endl;
  
  jsonxx::Object jsonObj;
  if (!jsonObj.parse(message))
    return;

  auto cmd = jsonObj.get<std::string>("cmd");
  auto params = jsonObj.get<jsonxx::Object>("params");

  if (cmd == "EnableFormation")
  {
    auto name = params.get<std::string>("name");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableFormation(name, enabled);
  }
  else if (cmd == "EnableSurface")
  {
    auto name = params.get<std::string>("name");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableSurface(name, enabled);
  }
  else if (cmd == "EnableSlice")
  {
    auto slice = params.get<jsonxx::Number>("slice");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableSlice((int)slice, enabled);
  }
  else if (cmd == "SetSlicePosition")
  {
    auto slice = params.get<jsonxx::Number>("slice");
    auto position = params.get<jsonxx::Number>("position");

    m_sceneGraphManager.setSlicePosition((int)slice, (int)position);
  }
  else if (cmd == "EnableFault")
  {
    auto collection = params.get<std::string>("collection");
    auto name = params.get<std::string>("name");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableFault(collection, name, enabled);
  }
  else if (cmd == "SetProperty")
  {
    auto name = params.get<std::string>("name");

    m_sceneGraphManager.setProperty(name);
  }
  else if (cmd == "SetVerticalScale")
  {
    auto scale = params.get<jsonxx::Number>("scale");

    m_sceneGraphManager.setVerticalScale((float)scale);
  }
  else if (cmd == "SetRenderStyle")
  {
    auto drawFaces = params.get<bool>("drawFaces");
    auto drawEdges = params.get<bool>("drawEdges");

    m_sceneGraphManager.setRenderStyle(drawFaces, drawEdges);
  }
  else if (cmd == "SetCurrentSnapshot")
  {
    auto index = params.get<jsonxx::Number>("index");

    m_sceneGraphManager.setCurrentSnapshot((int)index);
  }
  else if (cmd == "ViewAll")
  {
    SbViewportRegion vpregion = m_renderArea->getSceneManager()->getViewportRegion();
    m_examiner->viewAll(vpregion);
  }
  else if (cmd == "SetStillQuality")
  {
    auto quality = params.get<jsonxx::Number>("quality");

    std::cout << "Setting still quality to " << quality << std::endl;
    renderArea->getSettings()->setStillCompressionQuality((float)quality);
  }
  else if (cmd == "SetInteractiveQuality")
  {
    auto quality = params.get<jsonxx::Number>("quality");

    std::cout << "Setting interactive quality to " << quality << std::endl;
    renderArea->getSettings()->setInteractiveCompressionQuality((float)quality);
  }
  else if (cmd == "SetWidth")
  {
    auto width = params.get<jsonxx::Number>("width");
    auto height = renderArea->getSceneManager()->getSize()[1];

    renderArea->resize((int)width, (int)height);
  }
  else if (cmd == "SetHeight")
  {
    auto width = renderArea->getSceneManager()->getSize()[0];
    auto height = params.get<jsonxx::Number>("height"); 

    renderArea->resize((int)width, (int)height);
  }
}

bool BpaRenderAreaListener::onPreRender(RenderArea* /*renderArea*/, bool& clearWindow, bool& clearZBuffer)
{
  std::cout << "," << std::flush;

  clearWindow = true;
  clearZBuffer = true;

  return true;
}

void BpaRenderAreaListener::onPostRender(RenderArea* /*renderArea*/)
{
  std::cout << "." << std::flush;
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

bool BpaRenderAreaListener::onMouseUp(int /*x*/, int /*y*/, int /*button*/)
{
  return true;
}

bool BpaRenderAreaListener::onMouseDown(int /*x*/, int /*y*/, int /*button*/)
{
  return true;
}

bool BpaRenderAreaListener::onMouseMove(int /*x*/, int /*y*/)
{
  return true;
}

bool BpaRenderAreaListener::onMouseEnter(int /*x*/, int /*y*/)
{
  return true;
}

bool BpaRenderAreaListener::onMouseLeave(int /*x*/, int /*y*/)
{
  return true;
}

bool BpaRenderAreaListener::onMouseWheel(int /*delta*/)
{
  return true;
}

bool BpaRenderAreaListener::onKeyUp(const std::string& /*key*/)
{
  return true;
}

bool BpaRenderAreaListener::onKeyDown(const std::string& /*key*/)
{
  return true;
}

bool BpaRenderAreaListener::onTouchStart(unsigned int /*id*/, int /*x*/, int /*y*/)
{
  return true;
}

bool BpaRenderAreaListener::onTouchEnd(unsigned int /*id*/, int /*x*/, int /*y*/)
{
  return true;
}

bool BpaRenderAreaListener::onTouchMove(unsigned int /*id*/, int /*x*/, int /*y*/)
{
  return true;
}

