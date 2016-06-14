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
#include "jsonxx.h"

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

namespace
{
  jsonxx::Object toJSON(const SceneGraphManager::PickResult& pickResult)
  {
    jsonxx::Object obj;

    std::string typeStr;
    switch (pickResult.type)
    {
    case SceneGraphManager::PickResult::Formation: typeStr = "formation"; break;
    case SceneGraphManager::PickResult::Surface: typeStr = "surface"; break;
    case SceneGraphManager::PickResult::Reservoir: typeStr = "reservoir"; break;
    case SceneGraphManager::PickResult::Trap: typeStr = "trap"; break;
    }

    obj
      << "type" << typeStr
      << "positionX" << pickResult.position[0]
      << "positionY" << pickResult.position[1]
      << "positionZ" << pickResult.position[2]
      << "i" << pickResult.i
      << "j" << pickResult.j
      << "k" << pickResult.k
      << "name" << pickResult.name
      << "propertyValue" << pickResult.propertyValue
      << "trapID" << pickResult.trapID
      << "persistentTrapID" << pickResult.persistentTrapID;

    return obj;
  }

  jsonxx::Object toJSON(const Project::ProjectInfo& projectInfo)
  {
    // Add formation names
    jsonxx::Array formations;
    for (auto formation : projectInfo.formations)
      formations << formation.name;

    // Add surface names
    jsonxx::Array surfaces;
    for (auto surface : projectInfo.surfaces)
      surfaces << surface.name;

    // Add reservoir names
    jsonxx::Array reservoirs;
    for (auto reservoir : projectInfo.reservoirs)
      reservoirs << reservoir.name;

    // Add fault collections
    int collectionId = 0;
    jsonxx::Array faultCollections;
    for (auto faultCollection : projectInfo.faultCollections)
    {
      jsonxx::Array faults;
      for (auto fault : projectInfo.faults)
      {
        if (fault.collectionId == collectionId)
          faults << fault.name;
      }

      jsonxx::Object collection;
      collection << "name" << faultCollection.name;
      collection << "faults" << faults;

      faultCollections << collection;

      collectionId++;
    }

    // Add flow lines
    jsonxx::Array flowlines;
    for (auto lines : projectInfo.flowLines)
      flowlines << lines.name;

    // Add properties
    jsonxx::Array properties;
    for (auto property : projectInfo.properties)
      properties << property.name;

    // Assemble complete projectInfo structure
    jsonxx::Object projectInfoObject;
    projectInfoObject
      << "snapshotCount" << (int)projectInfo.snapshotCount
      << "numI" << projectInfo.dimensions.numCellsI
      << "numJ" << projectInfo.dimensions.numCellsJ
      << "numK" << projectInfo.dimensions.numCellsK
      << "numIHiRes" << projectInfo.dimensions.numCellsIHiRes
      << "numJHiRes" << projectInfo.dimensions.numCellsJHiRes
      << "formations" << formations
      << "surfaces" << surfaces
      << "reservoirs" << reservoirs
      << "faultCollections" << faultCollections
      << "flowLines" << flowlines
      << "properties" << properties;

    return projectInfoObject;
  }
}

void CommandHandler::onPick(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto x = (int)params.get<jsonxx::Number>("x");
  auto y = (int)params.get<jsonxx::Number>("y");

  unsigned int width = renderArea->getWidth();
  unsigned int height = renderArea->getHeight();
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

      renderArea->sendMessage(msg.write(jsonxx::JSON));
    }
  }
}

void CommandHandler::onEnableFormation(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto formationId = (int)params.get<jsonxx::Number>("formationId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFormation(formationId, enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "formationEnabled", params);
}

void CommandHandler::onEnableAllFormations(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFormations(enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "allFormationsEnabled", params);
}

void CommandHandler::onEnableSurface(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto surfaceId = (int)params.get<jsonxx::Number>("surfaceId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableSurface(surfaceId, enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "surfaceEnabled", params);
}

void CommandHandler::onEnableAllSurfaces(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllSurfaces(enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "allSurfacesEnabled", params);
}

void CommandHandler::onEnableReservoir(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto reservoirId = (int)params.get<jsonxx::Number>("reservoirId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableReservoir(reservoirId, enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "reservoirEnabled", params);
}

void CommandHandler::onEnableAllReservoirs(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllReservoirs(enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "allReservoirsEnabled", params);
}

void CommandHandler::onEnableSlice(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto slice = params.get<jsonxx::Number>("slice");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableSlice((int)slice, enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "sliceEnabled", params);
}

void CommandHandler::onSetSlicePosition(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto slice = params.get<jsonxx::Number>("slice");
  auto position = params.get<jsonxx::Number>("position");

  m_sceneGraphManager->setSlicePosition((int)slice, (int)position);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "slicePositionChanged", params);
}

void CommandHandler::onEnableFault(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto faultId = params.get<jsonxx::Number>("faultId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFault((int)faultId, enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "faultEnabled", params);
}

void CommandHandler::onEnableAllFaults(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFaults(enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "allFaultsEnabled", params);
}

void CommandHandler::onEnableFlowLines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto flowLinesId = (int)params.get<jsonxx::Number>("flowLinesId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFlowLines(flowLinesId, enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "flowLinesEnabled", params);
}

void CommandHandler::onEnableAllFlowLines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFlowLines(enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "allFlowLinesEnabled", params);
}

void CommandHandler::onEnableFence(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto fenceId = (int)params.get<jsonxx::Number>("fenceId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFence(fenceId, enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "fenceEnabled", params);
}

void CommandHandler::onSetProperty(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto propertyId = (int)params.get<jsonxx::Number>("propertyId");

  m_sceneGraphManager->setProperty(propertyId);

  sendEvent(renderArea, "currentPropertyChanged", params);
}

void CommandHandler::onSetVerticalScale(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto scale = params.get<jsonxx::Number>("scale");

  m_sceneGraphManager->setVerticalScale((float)scale);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "verticalScaleChanged", params);
}

void CommandHandler::onSetTransparency(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto transparency = params.get<jsonxx::Number>("transparency");

  m_sceneGraphManager->setTransparency((float)transparency);

  sendEvent(renderArea, "transparencyChanged", params);
}

void CommandHandler::onSetRenderStyle(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto drawFaces = params.get<bool>("drawFaces");
  auto drawEdges = params.get<bool>("drawEdges");

  m_sceneGraphManager->setRenderStyle(drawFaces, drawEdges);

  sendEvent(renderArea, "renderStyleChanged", params);
}

void CommandHandler::onShowCoordinateGrid(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showGrid = params.get<bool>("show");

  m_sceneGraphManager->showCoordinateGrid(showGrid);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "coordinateGridEnabled", params);
}
 
void CommandHandler::onShowCompass(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showCompass = params.get<bool>("show");

  m_sceneGraphManager->showCompass(showCompass);

  sendEvent(renderArea, "compassEnabled", params);
}

void CommandHandler::onShowText(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showText = params.get<bool>("show");

  m_sceneGraphManager->showText(showText);

  sendEvent(renderArea, "textEnabled", params);
}

void CommandHandler::onShowTraps(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showTraps = params.get<bool>("show");

  m_sceneGraphManager->showTraps(showTraps);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "trapsEnabled", params);
}

void CommandHandler::onShowTrapOutlines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showOutlines = params.get<bool>("show");

  m_sceneGraphManager->showTrapOutlines(showOutlines);

  sendEvent(renderArea, "trapOutlinesEnabled", params);
}

void CommandHandler::onSetFlowLinesStep(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");
  auto type = (typeStr == "FlowLinesExpulsion")
    ? SceneGraphManager::FlowLinesExpulsion
    : SceneGraphManager::FlowLinesLeakage;

  auto step = (int)params.get<jsonxx::Number>("step");

  m_sceneGraphManager->setFlowLinesStep(type, step);

  sendEvent(renderArea, "flowLinesStepChanged", params);
}

void CommandHandler::onSetFlowLinesThreshold(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");
  auto type = (typeStr == "FlowLinesExpulsion")
    ? SceneGraphManager::FlowLinesExpulsion
    : SceneGraphManager::FlowLinesLeakage;

  auto threshold = (double)params.get<jsonxx::Number>("threshold");

  m_sceneGraphManager->setFlowLinesThreshold(type, threshold);

  sendEvent(renderArea, "flowLinesThresholdChanged", params);
}

void CommandHandler::onShowDrainageAreaOutlines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");

  SceneGraphManager::DrainageAreaType type = SceneGraphManager::DrainageAreaNone;

  if (typeStr == "DrainageAreaFluid")
    type = SceneGraphManager::DrainageAreaFluid;
  else if (typeStr == "DrainageAreaGas")
    type = SceneGraphManager::DrainageAreaGas;

  m_sceneGraphManager->showDrainageAreaOutlines(type);

  sendEvent(renderArea, "drainageAreaOutlinesEnabled", params);
}

void CommandHandler::onSetProjection(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");
  auto mode = (typeStr == "Perspective")
    ? SceneInteractor::PERSPECTIVE
    : SceneInteractor::ORTHOGRAPHIC;

  m_examiner->setCameraMode(mode);

  sendEvent(renderArea, "projectionChanged", params);
}

void CommandHandler::onSetCurrentSnapshot(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto index = params.get<jsonxx::Number>("index");

  m_sceneGraphManager->setCurrentSnapshot((int)index);

  sendEvent(renderArea, "currentSnapshotChanged", params);
}

void CommandHandler::onViewAll(
  const jsonxx::Object& /*params*/,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  SbViewportRegion vpregion = renderArea->getSceneManager()->getViewportRegion();
  m_examiner->viewAll(vpregion);
}

void CommandHandler::onSetViewPreset(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
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
  RemoteViz::Rendering::RenderArea* renderArea,
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

  sendEvent(renderArea, "colorScaleParamsChanged", params);
}

void CommandHandler::onEnableCellFilter(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  bool enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableCellFilter(enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "cellFilterEnabled", params);
}

void CommandHandler::onSetCellFilterRange(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  double minValue = params.get<jsonxx::Number>("minval");
  double maxValue = params.get<jsonxx::Number>("maxval");

  m_sceneGraphManager->setCellFilterRange(minValue, maxValue);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "cellFilterRangeChanged", params);
}

void CommandHandler::onEnableSeismicSlice(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
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
    adjustClippingPlanes(renderArea);

	sendEvent(renderArea, "seismicSliceEnabled", params);
  }
}

void CommandHandler::onSetSeismicSlicePosition(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
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
    adjustClippingPlanes(renderArea);

	sendEvent(renderArea, "seismicSlicePositionChanged", params);
  }
}

void CommandHandler::onEnableInterpolatedSurface(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (!m_seismicScene)
    return;

  bool enabled = params.get<bool>("enabled");
  m_seismicScene->enableInterpolatedSurface(enabled);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "interpolatedSurfaceEnabled", params);
}

void CommandHandler::onSetInterpolatedSurfacePosition(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (!m_seismicScene)
    return;

  float position = (float)params.get<jsonxx::Number>("position");
  m_seismicScene->setInterpolatedSurfacePosition(position);
  adjustClippingPlanes(renderArea);

  sendEvent(renderArea, "interpolatedSurfacePositionChanged", params);
}

void CommandHandler::onSetSeismicDataRange(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  if (!m_seismicScene)
    return;

  float minValue = (float)params.get<jsonxx::Number>("minValue");
  float maxValue = (float)params.get<jsonxx::Number>("maxValue");

  m_seismicScene->setDataRange(minValue, maxValue);

  sendEvent(renderArea, "seismicDataRangeChanged", params);
}

void CommandHandler::onSetStillQuality(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto quality = params.get<jsonxx::Number>("quality");

  renderArea->getSettings()->setStillCompressionQuality((float)quality);

  sendEvent(renderArea, "stillQualityChanged", params);
}

void CommandHandler::onSetInteractiveQuality(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto quality = params.get<jsonxx::Number>("quality");

  renderArea->getSettings()->setInteractiveCompressionQuality((float)quality);

  sendEvent(renderArea, "interactiveQualityChanged", params);
}

void CommandHandler::onSetBandwidth(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
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
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* connection)
{
  auto maxFPS = (unsigned int)params.get<jsonxx::Number>("maxFPS");
  connection->getSettings()->setMaxSendingFPS(maxFPS);
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
}

void CommandHandler::adjustClippingPlanes(RemoteViz::Rendering::RenderArea* renderArea)
{
  SbViewportRegion vpregion(renderArea->getWidth(), renderArea->getHeight());
  m_examiner->getCameraInteractor()->adjustClippingPlanes(m_examiner, vpregion);
}

void CommandHandler::sendEvent(RemoteViz::Rendering::RenderArea* renderArea, const std::string& type, const jsonxx::Object& params)
{
  jsonxx::Object event;
  event << "type" << type;
  event << "params" << params;

  jsonxx::Object msg;
  msg << "event" << event;

  renderArea->sendMessage(msg.write(jsonxx::JSON));
}

CommandHandler::CommandHandler()
  : m_sceneGraphManager(nullptr)
  , m_examiner(nullptr)
{
}

void CommandHandler::setup(SceneGraphManager* mgr, SeismicScene* seismic, SceneExaminer* examiner)
{
  m_sceneGraphManager = mgr;
  m_seismicScene = seismic;
  m_examiner = examiner;
  
  registerHandlers();
}

void CommandHandler::sendProjectInfo(
  RemoteViz::Rendering::Connection* connection,
  const Project::ProjectInfo& projectInfo) const
{
  jsonxx::Object msg;
  msg << "projectInfo" << toJSON(projectInfo);
  connection->sendMessage(msg.write(jsonxx::JSON));
}

void CommandHandler::sendSeismicInfo(
  RemoteViz::Rendering::Connection* connection,
  const SbVec3i32& size, 
  const SbBox3f& extent) const
{
  jsonxx::Array sizeArray;
  sizeArray << size[0] << size[1] << size[2];

  jsonxx::Array extentMinArray;
  SbVec3f min = extent.getMin();
  extentMinArray << min[0] << min[1] << min[2];

  jsonxx::Array extentMaxArray;
  SbVec3f max = extent.getMax();
  extentMaxArray << max[0] << max[1] << max[2];

  jsonxx::Object extentObj;
  extentObj << "min" << extentMinArray;
  extentObj << "max" << extentMaxArray;

  jsonxx::Object seismicInfo;
  seismicInfo << "size" << sizeArray;
  seismicInfo << "extent" << extentObj;

  jsonxx::Object msg;
  msg << "seismicInfo" << seismicInfo;

  connection->sendMessage(msg.write(jsonxx::JSON));
}

void CommandHandler::sendFenceAddedEvent(
  RemoteViz::Rendering::RenderArea* renderArea,
  int fenceId)
{
  jsonxx::Object params;
  params << "fenceId" << fenceId;

  sendEvent(renderArea, "fenceAdded", params);
}

void CommandHandler::onReceivedMessage(
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* sender,
  const std::string& message)
{
  jsonxx::Object jsonObj;
  if (!jsonObj.parse(message))
    return;

  auto cmd = jsonObj.get<std::string>("cmd");
  auto params = jsonObj.get<jsonxx::Object>("params");

  BOOST_LOG_TRIVIAL(trace) << "received message \"" << cmd << "\" on render area " << renderArea->getId() << std::flush;

  auto iter = m_handlers.find(cmd);
  if (iter != m_handlers.end())
  {
    auto fn = iter->second;
    ((this)->*(fn))(params, renderArea, sender);
  }
}
