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
#include <Visualization/CameraUtil.h>

#include <RemoteViz/Rendering/RenderArea.h>
#include <RemoteViz/Rendering/Connection.h>
#include <RemoteViz/Rendering/RenderAreaSettings.h>

#include <Inventor/SoSceneManager.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoGradientBackground.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>

#include <Interface/ProjectHandle.h>
#include <Interface/ObjectFactory.h>
#include <Interface/Property.h>
#include <Interface/Formation.h>
#include <Interface/Surface.h>
#include <Interface/Reservoir.h>
#include <Interface/FaultCollection.h>
#include <Interface/Faulting.h>

#include <string>
#include <list>
#include <sstream>

using namespace std;

namespace di = DataAccess::Interface;

void BpaRenderAreaListener::createSceneGraph(const std::string& id)
{
  std::cout << "Loading scenegraph, id = " << id << std::endl;

  const std::string rootdir = "V:/data/";
  //const std::string rootdir = "/home/ree/data/";
  
  const std::string path = rootdir + id;

  m_project = Project::load(path);
  m_projectInfo = m_project->getProjectInfo();

  std::cout << "Project loaded!" << std::endl;

  m_sceneGraphManager.setup(m_project);

  SoGradientBackground* background = new SoGradientBackground;
  background->color0 = SbColor(.2f, .2f, .2f);
  background->color1 = SbColor(.2f, .2f, .3f);

  m_examiner = new SceneExaminer();
  m_examiner->addChild(background);
  m_examiner->addChild(m_sceneGraphManager.getRoot());

  m_renderArea->getSceneManager()->setSceneGraph(m_examiner);
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
  std::cout << "Sending project info" << std::endl;

  // Add formation names
  jsonxx::Array formations;
  std::unique_ptr<di::FormationList> formationList(m_handle->getFormations(0, false));
  for (auto formation : *formationList)
    formations << formation->getName();

  // Add surface names
  jsonxx::Array surfaces;
  std::unique_ptr<di::SurfaceList> surfaceList(m_handle->getSurfaces(0, false));
  for (auto surface : *surfaceList)
    surfaces << surface->getName();

  // Add reservoir names
  jsonxx::Array reservoirs;
  std::unique_ptr<di::ReservoirList> reservoirList(m_handle->getReservoirs());
  for (auto reservoir : *reservoirList)
    reservoirs << reservoir->getName();

  // Add fault collections
  jsonxx::Array faultCollections;
  std::unique_ptr<di::FaultCollectionList> faultCollectionList(m_handle->getFaultCollections(0));
  for (auto faultCollection : *faultCollectionList)
  {
    jsonxx::Array faults;
    std::unique_ptr<di::FaultList> faultList(faultCollection->getFaults());
    for (auto fault : *faultList)
      faults << fault->getName();

    jsonxx::Object collection;
    collection << "name" << faultCollection->getName();
    collection << "faults" << faults;

    faultCollections << collection;
  }

  // Add properties
  jsonxx::Array properties;
  const int allFlags = di::FORMATION | di::SURFACE | di::RESERVOIR | di::FORMATIONSURFACE;
  const int allTypes = di::MAP | di::VOLUME;
  std::unique_ptr<di::PropertyList> propertyList(m_handle->getProperties(true));
  for (auto property : *propertyList)
  {
    if (property->hasPropertyValues(allFlags, 0, 0, 0, 0, allTypes))
      properties << property->getName();
  }

  // Assemble complete projectInfo structure
  jsonxx::Object projectInfo;
  projectInfo
    << "snapshotCount" << (int)m_project->getSnapshotCount()
    << "numI" << m_project->numCellsI()
    << "numJ" << m_project->numCellsJ()
    << "numIHiRes" << m_project->numCellsIHiRes()
    << "numJHiRes" << m_project->numCellsJHiRes()
    << "formations" << formations
    << "surfaces" << surfaces
    << "reservoirs" << reservoirs
    << "faultCollections" << faultCollections
    << "properties" << properties;

  jsonxx::Object msg;
  msg << "projectInfo" << projectInfo;

  std::cout << msg.write(jsonxx::JSON) << std::endl;

  m_renderArea->sendMessage(msg.write(jsonxx::JSON));
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

  RenderAreaListener::onClosedConnection(renderArea, connectionId);
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
    auto formationId = (int)params.get<jsonxx::Number>("formationId");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableFormation(formationId, enabled);
  }
  if (cmd == "EnableAllFormations")
  {
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableAllFormations(enabled);
  }
  else if (cmd == "EnableSurface")
  {
    auto surfaceId = (int)params.get<jsonxx::Number>("surfaceId");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableSurface(surfaceId, enabled);
  }
  else if (cmd == "EnableAllSurfaces")
  {
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableAllSurfaces(enabled);
  }
  else if (cmd == "EnableReservoir")
  {
    auto reservoirId = (int)params.get<jsonxx::Number>("reservoirId");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableReservoir(reservoirId, enabled);
  }
  else if (cmd == "EnableAllReservoirs")
  {
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableAllReservoirs(enabled);
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
    //auto collection = params.get<std::string>("collection");
    //auto name = params.get<std::string>("name");
    auto faultId = params.get<jsonxx::Number>("faultId");
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableFault((int)faultId, enabled);
  }
  else if (cmd == "EnableAllFaults")
  {
    auto enabled = params.get<bool>("enabled");

    m_sceneGraphManager.enableAllFaults(enabled);
  }
  else if (cmd == "SetProperty")
  {
    auto propertyId = (int)params.get<jsonxx::Number>("propertyId");

    m_sceneGraphManager.setProperty(propertyId);
  }
  else if (cmd == "SetVerticalScale")
  {
    auto scale = params.get<jsonxx::Number>("scale");

    m_sceneGraphManager.setVerticalScale((float)scale);
  }
  else if (cmd == "SetTransparency")
  {
    auto transparency = params.get<jsonxx::Number>("transparency");

    m_sceneGraphManager.setTransparency((float)transparency);
  }
  else if (cmd == "SetRenderStyle")
  {
    auto drawFaces = params.get<bool>("drawFaces");
    auto drawEdges = params.get<bool>("drawEdges");

    m_sceneGraphManager.setRenderStyle(drawFaces, drawEdges);
  }
  else if (cmd == "ShowCoordinateGrid")
  {
    auto showGrid = params.get<bool>("show");

    m_sceneGraphManager.showCoordinateGrid(showGrid);
  }
  else if (cmd == "ShowTraps")
  {
    auto showTraps = params.get<bool>("show");

    m_sceneGraphManager.showTraps(showTraps);
  }
  else if (cmd == "ShowTrapOutlines")
  {
    auto showOutlines = params.get<bool>("show");

    m_sceneGraphManager.showTrapOutlines(showOutlines);
  }
  else if (cmd == "ShowFlowDirection")
  {
    auto typeStr = params.get<std::string>("type");

    SceneGraphManager::FlowVizType type = SceneGraphManager::FlowVizNone;

    if (typeStr == "FlowVizLines")
      type = SceneGraphManager::FlowVizLines;
    else if (typeStr == "FlowVizVectors")
      type = SceneGraphManager::FlowVizVectors;

    m_sceneGraphManager.showFlowDirection(type);
  }
  else if (cmd == "ShowDrainageAreaOutline")
  {
    auto typeStr = params.get<std::string>("type");

    SceneGraphManager::DrainageAreaType type = SceneGraphManager::DrainageAreaNone;

    if (typeStr == "DrainageAreaFluid")
      type = SceneGraphManager::DrainageAreaFluid;
    else if (typeStr == "DrainageAreaGas")
      type = SceneGraphManager::DrainageAreaGas;

    m_sceneGraphManager.showDrainageAreaOutlines(type);
  }
  else if (cmd == "SetProjection")
  {
    auto typeStr = params.get<std::string>("type");
    auto mode = (typeStr == "Perspective")
      ? SceneInteractor::PERSPECTIVE
      : SceneInteractor::ORTHOGRAPHIC;

    m_examiner->setCameraMode(mode);
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
  else if (cmd == "SetViewPreset")
  {
    auto preset = params.get<std::string>("preset");

    static const std::string presetKeys[] = 
    {
      "Top", "Left", "Front",
      "Bottom", "Right", "Back"
    };

    static const ViewPreset presetValues[] =
    {
      ViewPreset_Top, ViewPreset_Left, ViewPreset_Front,
      ViewPreset_Bottom, ViewPreset_Right, ViewPreset_Back
    };

    for (int i = 0; i < 6; ++i)
    {
      if (preset == presetKeys[i])
      {
        setViewPreset(m_examiner->getCameraInteractor()->getCamera(), presetValues[i]);
        break;
      }
    }
  }
  else if (cmd == "SetStillQuality")
  {
    auto quality = params.get<jsonxx::Number>("quality");

    renderArea->getSettings()->setStillCompressionQuality((float)quality);
  }
  else if (cmd == "SetInteractiveQuality")
  {
    auto quality = params.get<jsonxx::Number>("quality");

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
  //std::cout << "," << std::flush;

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

