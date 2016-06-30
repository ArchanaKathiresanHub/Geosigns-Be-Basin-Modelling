//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CommandHandler.h"
#include "SceneExaminer.h"
#include "JSONUtil.h"

#include <SceneGraphManager.h>
#include <Seismic.h>
#include <CameraUtil.h>

#ifdef USE_H264
#include <RenderArea.h>
#include <Connection.h>
#include <ConnectionSettings.h>
#include <RenderAreaSettings.h>
#else
#include <RemoteViz/Rendering/RenderArea.h>
#include <RemoteViz/Rendering/Connection.h>
#include <RemoteViz/Rendering/ConnectionSettings.h>
#include <RemoteViz/Rendering/RenderAreaSettings.h>
#endif

#include <Inventor/SoSceneManager.h>
#include <Inventor/ViewerComponents/SoCameraInteractor.h>
#include <Inventor/actions/SoRayPickAction.h>

#include <boost/log/trivial.hpp>

void CommandHandler::onPick(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto x = (int)params.get<jsonxx::Number>("x");
  auto y = (int)params.get<jsonxx::Number>("y");

  unsigned int width = m_renderArea->getWidth();
  unsigned int height = m_renderArea->getHeight();
  SbViewportRegion vpregion((short)width, (short)height);

  SoRayPickAction action(vpregion);
  action.setPoint(SbVec2s((short)x, (short)(height - y - 1)));
  action.apply(m_examiner);
  SoPickedPoint* p = action.getPickedPoint();
  if (p)
  {
    auto pickResult = m_sceneGraphManager->processPickedPoint(p);
    if (pickResult.type != SceneGraphManager::PickResult::Unknown)
    {
      jsonxx::Object msg;
      msg << "pickResult" << toJSON(pickResult);

      m_renderArea->sendMessage(msg.write(jsonxx::JSON));
    }
  }
}

void CommandHandler::onEnableFormation(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto formationId = (int)params.get<jsonxx::Number>("formationId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFormation(formationId, enabled);
  adjustClippingPlanes();

  sendEvent("formationEnabled", params);
}

void CommandHandler::onEnableAllFormations(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFormations(enabled);
  adjustClippingPlanes();

  sendEvent("allFormationsEnabled", params);
}

void CommandHandler::onEnableSurface(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto surfaceId = (int)params.get<jsonxx::Number>("surfaceId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableSurface(surfaceId, enabled);
  adjustClippingPlanes();

  sendEvent("surfaceEnabled", params);
}

void CommandHandler::onEnableAllSurfaces(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllSurfaces(enabled);
  adjustClippingPlanes();

  sendEvent("allSurfacesEnabled", params);
}

void CommandHandler::onEnableReservoir(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto reservoirId = (int)params.get<jsonxx::Number>("reservoirId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableReservoir(reservoirId, enabled);
  adjustClippingPlanes();

  sendEvent("reservoirEnabled", params);
}

void CommandHandler::onEnableAllReservoirs(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllReservoirs(enabled);
  adjustClippingPlanes();

  sendEvent("allReservoirsEnabled", params);
}

void CommandHandler::onEnableSlice(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto slice = params.get<jsonxx::Number>("slice");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableSlice((int)slice, enabled);
  adjustClippingPlanes();

  sendEvent("sliceEnabled", params);
}

void CommandHandler::onSetSlicePosition(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto slice = params.get<jsonxx::Number>("slice");
  auto position = params.get<jsonxx::Number>("position");

  m_sceneGraphManager->setSlicePosition((int)slice, (int)position);
  adjustClippingPlanes();

  sendEvent("slicePositionChanged", params);
}

void CommandHandler::onEnableFault(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto faultId = params.get<jsonxx::Number>("faultId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFault((int)faultId, enabled);
  adjustClippingPlanes();

  sendEvent("faultEnabled", params);
}

void CommandHandler::onEnableAllFaults(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFaults(enabled);
  adjustClippingPlanes();

  sendEvent("allFaultsEnabled", params);
}

void CommandHandler::onEnableFlowLines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto flowLinesId = (int)params.get<jsonxx::Number>("flowLinesId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFlowLines(flowLinesId, enabled);
  adjustClippingPlanes();

  sendEvent("flowLinesEnabled", params);
}

void CommandHandler::onEnableAllFlowLines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFlowLines(enabled);
  adjustClippingPlanes();

  sendEvent("allFlowLinesEnabled", params);
}

void CommandHandler::onEnableFence(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto fenceId = (int)params.get<jsonxx::Number>("fenceId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFence(fenceId, enabled);
  adjustClippingPlanes();

  sendEvent("fenceEnabled", params);
}

void CommandHandler::onSetProperty(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto propertyId = (int)params.get<jsonxx::Number>("propertyId");

  m_sceneGraphManager->setProperty(propertyId);

  sendEvent("currentPropertyChanged", params);
}

void CommandHandler::onSetVerticalScale(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto scale = params.get<jsonxx::Number>("scale");

  m_sceneGraphManager->setVerticalScale((float)scale);
  adjustClippingPlanes();

  sendEvent("verticalScaleChanged", params);
}

void CommandHandler::onSetTransparency(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto transparency = params.get<jsonxx::Number>("transparency");

  m_sceneGraphManager->setTransparency((float)transparency);

  sendEvent("transparencyChanged", params);
}

void CommandHandler::onSetRenderStyle(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto drawFaces = params.get<bool>("drawFaces");
  auto drawEdges = params.get<bool>("drawEdges");

  m_sceneGraphManager->setRenderStyle(drawFaces, drawEdges);

  sendEvent("renderStyleChanged", params);
}

void CommandHandler::onShowCoordinateGrid(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showGrid = params.get<bool>("show");

  m_sceneGraphManager->showCoordinateGrid(showGrid);
  adjustClippingPlanes();

  sendEvent("coordinateGridEnabled", params);
}
 
void CommandHandler::onShowCompass(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showCompass = params.get<bool>("show");

  m_sceneGraphManager->showCompass(showCompass);

  sendEvent("compassEnabled", params);
}

void CommandHandler::onShowText(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showText = params.get<bool>("show");

  m_sceneGraphManager->showText(showText);

  sendEvent("textEnabled", params);
}

void CommandHandler::onShowTraps(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showTraps = params.get<bool>("show");

  m_sceneGraphManager->showTraps(showTraps);
  adjustClippingPlanes();

  sendEvent("trapsEnabled", params);
}

void CommandHandler::onShowTrapOutlines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showOutlines = params.get<bool>("show");

  m_sceneGraphManager->showTrapOutlines(showOutlines);

  sendEvent("trapOutlinesEnabled", params);
}

void CommandHandler::onSetFlowLinesStep(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");
  auto type = (typeStr == "FlowLinesExpulsion")
    ? SceneGraphManager::FlowLinesExpulsion
    : SceneGraphManager::FlowLinesLeakage;

  auto step = (int)params.get<jsonxx::Number>("step");

  m_sceneGraphManager->setFlowLinesStep(type, step);

  sendEvent("flowLinesStepChanged", params);
}

void CommandHandler::onSetFlowLinesThreshold(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");
  auto type = (typeStr == "FlowLinesExpulsion")
    ? SceneGraphManager::FlowLinesExpulsion
    : SceneGraphManager::FlowLinesLeakage;

  auto threshold = (double)params.get<jsonxx::Number>("threshold");

  m_sceneGraphManager->setFlowLinesThreshold(type, threshold);

  sendEvent("flowLinesThresholdChanged", params);
}

void CommandHandler::onShowDrainageAreaOutlines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");

  SceneGraphManager::DrainageAreaType type = SceneGraphManager::DrainageAreaNone;

  if (typeStr == "DrainageAreaFluid")
    type = SceneGraphManager::DrainageAreaFluid;
  else if (typeStr == "DrainageAreaGas")
    type = SceneGraphManager::DrainageAreaGas;

  m_sceneGraphManager->showDrainageAreaOutlines(type);

  sendEvent("drainageAreaOutlinesEnabled", params);
}

void CommandHandler::onSetProjection(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");
  auto mode = (typeStr == "Perspective")
    ? SceneInteractor::PERSPECTIVE
    : SceneInteractor::ORTHOGRAPHIC;

  m_examiner->setCameraMode(mode);

  sendEvent("projectionChanged", params);
}

void CommandHandler::onSetCurrentSnapshot(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto index = params.get<jsonxx::Number>("index");

  m_sceneGraphManager->setCurrentSnapshot((int)index);
  adjustClippingPlanes();

  sendEvent("currentSnapshotChanged", params);
}

void CommandHandler::onViewAll(
  const jsonxx::Object& /*params*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  SbViewportRegion vpregion = m_renderArea->getSceneManager()->getViewportRegion();
  m_examiner->viewAll(vpregion);
}

void CommandHandler::onSetViewPreset(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
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

void CommandHandler::onSetColorScaleParams(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto mappingStr = params.get<std::string>("mapping");
  auto rangeStr = params.get<std::string>("range");

  SceneGraphManager::ColorScaleParams colorScaleParams;
  colorScaleParams.mapping = (mappingStr == "linear")
    ? SceneGraphManager::ColorScaleParams::Linear
    : SceneGraphManager::ColorScaleParams::Logarithmic;

  colorScaleParams.range = (rangeStr == "auto")
    ? SceneGraphManager::ColorScaleParams::Automatic
    : SceneGraphManager::ColorScaleParams::Manual;

  colorScaleParams.minValue = (double)params.get<jsonxx::Number>("minval");
  colorScaleParams.maxValue = (double)params.get<jsonxx::Number>("maxval");

  m_sceneGraphManager->setColorScaleParams(colorScaleParams);

  sendEvent("colorScaleParamsChanged", params);
}

void CommandHandler::onEnableCellFilter(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  bool enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableCellFilter(enabled);
  adjustClippingPlanes();

  sendEvent("cellFilterEnabled", params);
}

void CommandHandler::onSetCellFilterRange(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  double minValue = params.get<jsonxx::Number>("minval");
  double maxValue = params.get<jsonxx::Number>("maxval");

  m_sceneGraphManager->setCellFilterRange(minValue, maxValue);
  adjustClippingPlanes();

  sendEvent("cellFilterRangeChanged", params);
}

void CommandHandler::onEnableSeismicSlice(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (m_seismicScene)
  {
    int index = (int)params.get<jsonxx::Number>("index");
    bool enabled = params.get<bool>("enabled");

    SeismicScene::SliceType type[] =
    {
      SeismicScene::SliceInline,
      SeismicScene::SliceCrossline
    };

    m_seismicScene->enableSlice(type[index], enabled);
    adjustClippingPlanes();
    
    sendEvent("seismicSliceEnabled", params);
  }
}

void CommandHandler::onSetSeismicSlicePosition(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (m_seismicScene)
  {
    int index = (int)params.get<jsonxx::Number>("index");
    float position = (float)params.get<jsonxx::Number>("position");

    SeismicScene::SliceType type[] =
    {
      SeismicScene::SliceInline,
      SeismicScene::SliceCrossline
    };

    m_seismicScene->setSlicePosition(type[index], position);
    adjustClippingPlanes();

    sendEvent("seismicSlicePositionChanged", params);
  }
}

void CommandHandler::onEnableInterpolatedSurface(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (!m_seismicScene)
    return;

  bool enabled = params.get<bool>("enabled");
  m_seismicScene->enableInterpolatedSurface(enabled);
  adjustClippingPlanes();

  sendEvent("interpolatedSurfaceEnabled", params);
}

void CommandHandler::onSetInterpolatedSurfacePosition(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (!m_seismicScene)
    return;

  float position = (float)params.get<jsonxx::Number>("position");
  m_seismicScene->setInterpolatedSurfacePosition(position);
  adjustClippingPlanes();

  sendEvent("interpolatedSurfacePositionChanged", params);
}

void CommandHandler::onSetSeismicDataRange(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (!m_seismicScene)
    return;

  float minValue = (float)params.get<jsonxx::Number>("minValue");
  float maxValue = (float)params.get<jsonxx::Number>("maxValue");

  m_seismicScene->setDataRange(minValue, maxValue);

  sendEvent("seismicDataRangeChanged", params);
}

void CommandHandler::onSetStillQuality(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto quality = params.get<jsonxx::Number>("quality");

  m_renderArea->getSettings()->setStillCompressionQuality((float)quality);

  sendEvent("stillQualityChanged", params);
}

void CommandHandler::onSetInteractiveQuality(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto quality = params.get<jsonxx::Number>("quality");

  m_renderArea->getSettings()->setInteractiveCompressionQuality((float)quality);

  sendEvent("interactiveQualityChanged", params);
}

void CommandHandler::onSetBandwidth(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* connection)
{
#ifdef USE_H264
  auto bandwidth = params.get<jsonxx::Number>("bandwidth");

  connection->getSettings()->setBitsPerSecond((int)bandwidth);
#else
  (void)params;
  (void)connection;
#endif
}

void CommandHandler::onSetMaxFPS(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* connection)
{
  auto maxFPS = (unsigned int)params.get<jsonxx::Number>("maxFPS");
  connection->getSettings()->setMaxSendingFPS(maxFPS);
}

void CommandHandler::onSetWidth(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto width = params.get<jsonxx::Number>("width");
  auto height = m_renderArea->getSceneManager()->getSize()[1];

  m_renderArea->resize((int)width, (int)height);
}

void CommandHandler::onSetHeight(
  const jsonxx::Object& params,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto width = m_renderArea->getSceneManager()->getSize()[0];
  auto height = params.get<jsonxx::Number>("height");

  m_renderArea->resize((int)width, (int)height);
}

void CommandHandler::registerHandlers()
{
  m_handlers["Pick"] = &CommandHandler::onPick;
  m_handlers["EnableFormation"] = &CommandHandler::onEnableFormation;
  m_handlers["EnableAllFormations"] = &CommandHandler::onEnableAllFormations;
  m_handlers["EnableSurface"] = &CommandHandler::onEnableSurface;
  m_handlers["EnableAllSurfaces"] = &CommandHandler::onEnableAllSurfaces;
  m_handlers["EnableReservoir"] = &CommandHandler::onEnableReservoir;
  m_handlers["EnableAllReservoirs"] = &CommandHandler::onEnableAllReservoirs;
  m_handlers["EnableSlice"] = &CommandHandler::onEnableSlice;
  m_handlers["SetSlicePosition"] = &CommandHandler::onSetSlicePosition;
  m_handlers["EnableFault"] = &CommandHandler::onEnableFault;
  m_handlers["EnableAllFaults"] = &CommandHandler::onEnableAllFaults;
  m_handlers["EnableFlowLines"] = &CommandHandler::onEnableFlowLines;
  m_handlers["EnableAllFlowLines"] = &CommandHandler::onEnableAllFlowLines;
  m_handlers["EnableFence"] = &CommandHandler::onEnableFence;
  m_handlers["SetProperty"] = &CommandHandler::onSetProperty;
  m_handlers["SetVerticalScale"] = &CommandHandler::onSetVerticalScale;
  m_handlers["SetTransparency"] = &CommandHandler::onSetTransparency;
  m_handlers["SetRenderStyle"] = &CommandHandler::onSetRenderStyle;
  m_handlers["ShowCoordinateGrid"] = &CommandHandler::onShowCoordinateGrid;
  m_handlers["ShowCompass"] = &CommandHandler::onShowCompass;
  m_handlers["ShowText"] = &CommandHandler::onShowText;
  m_handlers["ShowTraps"] = &CommandHandler::onShowTraps;
  m_handlers["ShowTrapOutlines"] = &CommandHandler::onShowTrapOutlines;
  m_handlers["SetFlowLinesStep"] = &CommandHandler::onSetFlowLinesStep;
  m_handlers["SetFlowLinesThreshold"] = &CommandHandler::onSetFlowLinesThreshold;
  m_handlers["ShowDrainageAreaOutline"] = &CommandHandler::onShowDrainageAreaOutlines;
  m_handlers["SetProjection"] = &CommandHandler::onSetProjection;
  m_handlers["SetCurrentSnapshot"] = &CommandHandler::onSetCurrentSnapshot;
  m_handlers["ViewAll"] = &CommandHandler::onViewAll;
  m_handlers["SetViewPreset"] = &CommandHandler::onSetViewPreset;
  m_handlers["SetColorScaleParams"] = &CommandHandler::onSetColorScaleParams;
  m_handlers["EnableCellFilter"] = &CommandHandler::onEnableCellFilter;
  m_handlers["SetCellFilterRange"] = &CommandHandler::onSetCellFilterRange;
  m_handlers["EnableSeismicSlice"] = &CommandHandler::onEnableSeismicSlice;
  m_handlers["SetSeismicSlicePosition"] = &CommandHandler::onSetSeismicSlicePosition;
  m_handlers["EnableInterpolatedSurface"] = &CommandHandler::onEnableInterpolatedSurface;
  m_handlers["SetInterpolatedSurfacePosition"] = &CommandHandler::onSetInterpolatedSurfacePosition;
  m_handlers["SetSeismicDataRange"] = &CommandHandler::onSetSeismicDataRange;
  m_handlers["SetStillQuality"] = &CommandHandler::onSetStillQuality;
  m_handlers["SetInteractiveQuality"] = &CommandHandler::onSetInteractiveQuality;
  m_handlers["SetBandwidth"] = &CommandHandler::onSetBandwidth;
  m_handlers["SetMaxFPS"] = &CommandHandler::onSetMaxFPS;
  m_handlers["SetWidth"] = &CommandHandler::onSetWidth;
  m_handlers["SetHeight"] = &CommandHandler::onSetHeight;
}

void CommandHandler::adjustClippingPlanes()
{
  SbViewportRegion vpregion(m_renderArea->getWidth(), m_renderArea->getHeight());
  m_examiner->getCameraInteractor()->adjustClippingPlanes(m_examiner, vpregion);
}

void CommandHandler::sendEvent(const std::string& type, const jsonxx::Object& params)
{
  jsonxx::Object event;
  event << "type" << type;
  event << "params" << params;

  jsonxx::Object msg;
  msg << "event" << event;

  m_renderArea->sendMessage(msg.write(jsonxx::JSON));
}

CommandHandler::CommandHandler(RemoteViz::Rendering::RenderArea* renderArea)
  : m_sceneGraphManager(nullptr)
  , m_examiner(nullptr)
  , m_renderArea(renderArea)
{
}

void CommandHandler::setup(SceneGraphManager* mgr, SeismicScene* seismic, SceneExaminer* examiner)
{
  m_sceneGraphManager = mgr;
  m_seismicScene = seismic;
  m_examiner = examiner;
  
  registerHandlers();
}

void CommandHandler::onReceivedMessage(
  RemoteViz::Rendering::Connection* sender,
  const std::string& message)
{
  jsonxx::Object jsonObj;
  if (!jsonObj.parse(message))
    return;

  auto cmd = jsonObj.get<std::string>("cmd");
  auto params = jsonObj.get<jsonxx::Object>("params");

  BOOST_LOG_TRIVIAL(trace) << "received message \"" << cmd << "\" on render area " << m_renderArea->getId() << std::flush;

  auto iter = m_handlers.find(cmd);
  if (iter != m_handlers.end())
  {
    auto fn = iter->second;
    ((this)->*(fn))(params, sender);
  }
}
