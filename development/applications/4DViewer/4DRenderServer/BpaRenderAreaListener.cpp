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

#include <SceneGraphManager.h>
#include <Seismic.h>
#include <CameraUtil.h>

#include "jsonxx.h"

#ifdef USE_H264
#include <RenderArea.h>
#include <Connection.h>
#include <ConnectionSettings.h>
#include <RenderAreaSettings.h>
#else
#include <RemoteViz/Rendering/RenderArea.h>
#include <RemoteViz/Rendering/Connection.h>
#include <RemoteViz/Rendering/RenderAreaSettings.h>
#endif

#include <Inventor/SoSceneManager.h>
#include <Inventor/nodes/SoGradientBackground.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>

#include <string>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

void BpaRenderAreaListener::loadSeismic()
{
  // Try to find seismic data
  boost::filesystem::path ldmFile;
  boost::filesystem::directory_iterator iter(m_projectdir);
  boost::filesystem::directory_iterator end;
  while (iter != end && iter->path().extension() != ".ldm")
    ++iter;

  if (iter != end)
    ldmFile = *iter;

  if (!ldmFile.empty())
  {
    BOOST_LOG_TRIVIAL(trace) << "found LDM file " << ldmFile;

    std::string ldmFileStr = ldmFile.string();
    auto dim = m_project->getProjectInfo().dimensions;
    m_seismicScene = std::make_shared<SeismicScene>(ldmFileStr.c_str(), dim);
    m_sceneGraphManager->addSeismicScene(m_seismicScene);
  }
}

void BpaRenderAreaListener::createSceneGraph()
{
  assert(m_project);

  m_sceneGraphManager = std::make_shared<SceneGraphManager>();
  m_sceneGraphManager->setup(m_project);

  m_examiner = new SceneExaminer(m_sceneGraphManager);
  m_examiner->setFenceAddedCallback(std::bind(&BpaRenderAreaListener::onFenceAdded, this, std::placeholders::_1));

  loadSeismic();

  m_commandHandler.setup(
    m_sceneGraphManager.get(),
    m_seismicScene.get(),
    m_examiner.ptr());

  m_renderArea->getSceneManager()->setSceneGraph(m_examiner.ptr());
  m_examiner->viewAll(m_renderArea->getSceneManager()->getViewportRegion());
}

void BpaRenderAreaListener::setupProject(const std::string& id)
{
  BOOST_LOG_TRIVIAL(trace) << "Loading project, id = " << id;

  boost::filesystem::path projectFile(m_rootdir);
  projectFile.append(id);

  m_projectdir = projectFile.parent_path().string();

  try
  {
    m_project = Project::load(projectFile.string());
    m_projectInfo = m_project->getProjectInfo();

    createSceneGraph();
  }
  catch (std::runtime_error& e)
  {
    BOOST_LOG_TRIVIAL(trace) << "Failed to load project " << projectFile << " (" << e.what() << ")";
  }
}

void BpaRenderAreaListener::onFenceAdded(int fenceId)
{
  m_commandHandler.sendFenceAddedEvent(m_renderArea, fenceId);
}

void BpaRenderAreaListener::onConnectionCountChanged()
{
  unsigned int count = m_renderArea->getNumConnections();
  jsonxx::Object params;
  params << "count" << count;

  jsonxx::Object event;
  event << "type" << "connectionCountChanged";
  event << "params" << params;

  jsonxx::Object msg;
  msg << "event" << event;

  m_renderArea->sendMessage(msg.write(jsonxx::JSON));
}

void BpaRenderAreaListener::sendProjectInfo(Connection* connection)
{
}

namespace
{
  template<class T>
  jsonxx::Array toJSON(const std::vector<T>& v)
  {
    jsonxx::Array result;
    for (T x : v)
      result << x;

    return result;
  }

  template<class T>
  jsonxx::Array toJSON(const T* v, int n)
  {
    jsonxx::Array result;
    for(int i=0; i < n; ++i)
      result << v[i];

    return result;
  }
}

void BpaRenderAreaListener::sendViewState(Connection* connection)
{
  auto state = m_sceneGraphManager->getViewState();
  auto mode = m_examiner->getCameraMode();
  auto renderAreaSettings = m_renderArea->getSettings();

  SeismicScene::ViewState seismicState;
  if (m_seismicScene)
	seismicState = m_seismicScene->getViewState();

  jsonxx::Object vs;
  vs
	<< "currentSnapshotIndex" << state.currentSnapshotIndex
	<< "currentPropertyId" << state.currentPropertyId

	// display settings
	<< "showFaces" << state.showFaces
	<< "showEdges" << state.showEdges
	<< "showGrid" << state.showGrid
	<< "showCompass" << state.showCompass
	<< "showText" << state.showText
	<< "showPerspective" << (mode == SceneInteractor::PERSPECTIVE)
	<< "verticalScale" << state.verticalScale
	<< "transparency" << state.transparency

	// traps & flow
	<< "showTraps" << state.showTraps
	<< "showTrapOutlines" << state.showTrapOutlines
	<< "drainageAreaType" << (int)state.drainageAreaType
	<< "flowLinesExpulsionStep" << state.flowLinesExpulsionStep
	<< "flowLinesLeakageStep" << state.flowLinesLeakageStep
	<< "flowLinesExpulsionThreshold" << state.flowLinesExpulsionThreshold
	<< "flowLinesLeakageThreshold" << state.flowLinesLeakageThreshold

	// elements
	<< "formationVisibility" << toJSON(state.formationVisibility)
	<< "surfaceVisibility" << toJSON(state.surfaceVisibility)
	<< "reservoirVisibility" << toJSON(state.reservoirVisibility)
	<< "faultVisibility" << toJSON(state.faultVisibility)
	<< "flowLinesVisibility" << toJSON(state.flowLinesVisibility)

	// slices
	<< "slicePosition" << toJSON(state.slicePosition, 3)
	<< "sliceEnabled" << toJSON(state.sliceEnabled, 3)

	// cell filter
	<< "cellFilterEnabled" << state.cellFilterEnabled
	<< "cellFilterMinValue" << state.cellFilterMinValue
	<< "cellFilterMaxValue" << state.cellFilterMaxValue

	// color scale
	<< "colorScaleMapping" << (int)state.colorScaleParams.mapping
	<< "colorScaleRange" << (int)state.colorScaleParams.range
	<< "colorScaleMinValue" << state.colorScaleParams.minValue
	<< "colorScaleMaxValue" << state.colorScaleParams.maxValue

	// jpeg quality
	<< "stillQuality" << renderAreaSettings->getStillCompressionQuality()
	<< "interactiveQuality" << renderAreaSettings->getInteractiveCompressionQuality()

	// seismic
	<< "seismicInlineSliceEnabled" << seismicState.inlineSliceEnabled
	<< "seismicInlineSlicePosition" << seismicState.inlineSlicePosition
	<< "seismicCrosslineSliceEnabled" << seismicState.crosslineSliceEnabled
	<< "seismicCrosslineSlicePosition" << seismicState.crosslineSlicePosition
	<< "seismicInterpolatedSurfaceEnabled" << seismicState.interpolatedSurfaceEnabled
	<< "seismicInterpolatedSurfacePosition" << seismicState.interpolatedSurfacePosition;

  jsonxx::Object msg;
  msg << "viewstate" << vs;

  connection->sendMessage(msg.write(jsonxx::JSON));
}

BpaRenderAreaListener::BpaRenderAreaListener(RenderArea* renderArea)
: m_renderArea(renderArea)
, m_examiner(0)
, m_drawFaces(true)
, m_drawEdges(true)
, m_logEvents(true)
{
}

BpaRenderAreaListener::~BpaRenderAreaListener()
{
}

void BpaRenderAreaListener::setDataDir(const std::string& dir)
{
  m_rootdir = dir;
}

void BpaRenderAreaListener::onOpenedConnection(RenderArea* renderArea, Connection* connection)
{
  if (m_logEvents)
    BOOST_LOG_TRIVIAL(trace) << "new connection opened on render area " << renderArea->getId() << ", id = " << connection->getId();

  if(!m_project)
    setupProject(renderArea->getId());

  m_commandHandler.sendProjectInfo(connection, m_projectInfo);

  onConnectionCountChanged();

  if (m_seismicScene)
  {
    SbBox3f extent = m_seismicScene->getExtent();
    SbVec3i32 size = m_seismicScene->getDimensions();

    m_commandHandler.sendSeismicInfo(connection, size, extent);
  }

  sendViewState(connection);

  RemoteViz::Rendering::RenderAreaListener::onOpenedConnection(renderArea, connection);
}

void BpaRenderAreaListener::onClosedConnection(RenderArea* renderArea, const std::string& connectionId)
{
  if (m_logEvents)
    BOOST_LOG_TRIVIAL(trace) << "connection " << connectionId << " closed";

  onConnectionCountChanged();

  RenderAreaListener::onClosedConnection(renderArea, connectionId);
}

void BpaRenderAreaListener::onReceivedMessage(RenderArea* renderArea, Connection* sender, const std::string& message)
{
  m_commandHandler.onReceivedMessage(renderArea, sender, message);

  RemoteViz::Rendering::RenderAreaListener::onReceivedMessage(renderArea, sender, message);
}

bool BpaRenderAreaListener::onPreRender(RenderArea* renderArea, bool& clearWindow, bool& clearZBuffer)
{
  //BOOST_LOG_TRIVIAL(trace) << "prerender renderArea " << renderArea->getId();
  return RemoteViz::Rendering::RenderAreaListener::onPreRender(renderArea, clearWindow, clearZBuffer);
}

void BpaRenderAreaListener::onPostRender(RenderArea* renderArea)
{
  //BOOST_LOG_TRIVIAL(trace) << "postrender renderArea " << renderArea->getId();
  RemoteViz::Rendering::RenderAreaListener::onPostRender(renderArea);
}

void BpaRenderAreaListener::onRequestedSize(RenderArea* renderArea, Connection* sender, unsigned int width, unsigned int height)
{
  if(m_logEvents)
    BOOST_LOG_TRIVIAL(trace) << "requested resize render area " << renderArea->getId() << " to " << width << " x " << height << " (DENIED)";
}
