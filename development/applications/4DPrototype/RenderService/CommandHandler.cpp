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

#include <Visualization/SceneGraphManager.h>
#include <Visualization/CameraUtil.h>

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

namespace
{
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
      << "numIHiRes" << projectInfo.dimensions.numCellsIHiRes
      << "numJHiRes" << projectInfo.dimensions.numCellsJHiRes
      << "formations" << formations
      << "surfaces" << surfaces
      << "reservoirs" << reservoirs
      << "faultCollections" << faultCollections
      << "properties" << properties;

    return projectInfoObject;
  }
}

void CommandHandler::onEnableFormation(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto formationId = (int)params.get<jsonxx::Number>("formationId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFormation(formationId, enabled);
}

void CommandHandler::onEnableAllFormations(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFormations(enabled);
}

void CommandHandler::onEnableSurface(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto surfaceId = (int)params.get<jsonxx::Number>("surfaceId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableSurface(surfaceId, enabled);
}

void CommandHandler::onEnableAllSurfaces(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllSurfaces(enabled);
}

void CommandHandler::onEnableReservoir(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto reservoirId = (int)params.get<jsonxx::Number>("reservoirId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableReservoir(reservoirId, enabled);
}

void CommandHandler::onEnableAllReservoirs(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllReservoirs(enabled);
}

void CommandHandler::onEnableSlice(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto slice = params.get<jsonxx::Number>("slice");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableSlice((int)slice, enabled);
}

void CommandHandler::onSetSlicePosition(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto slice = params.get<jsonxx::Number>("slice");
  auto position = params.get<jsonxx::Number>("position");

  m_sceneGraphManager->setSlicePosition((int)slice, (int)position);
}

void CommandHandler::onEnableFault(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto faultId = params.get<jsonxx::Number>("faultId");
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableFault((int)faultId, enabled);
}

void CommandHandler::onEnableAllFaults(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto enabled = params.get<bool>("enabled");

  m_sceneGraphManager->enableAllFaults(enabled);
}

void CommandHandler::onSetProperty(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto propertyId = (int)params.get<jsonxx::Number>("propertyId");

  m_sceneGraphManager->setProperty(propertyId);
}

void CommandHandler::onSetVerticalScale(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto scale = params.get<jsonxx::Number>("scale");

  m_sceneGraphManager->setVerticalScale((float)scale);
}

void CommandHandler::onSetTransparency(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto transparency = params.get<jsonxx::Number>("transparency");

  m_sceneGraphManager->setTransparency((float)transparency);
}

void CommandHandler::onSetRenderStyle(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto drawFaces = params.get<bool>("drawFaces");
  auto drawEdges = params.get<bool>("drawEdges");

  m_sceneGraphManager->setRenderStyle(drawFaces, drawEdges);
}

void CommandHandler::onShowCoordinateGrid(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showGrid = params.get<bool>("show");

  m_sceneGraphManager->showCoordinateGrid(showGrid);
}
 
void CommandHandler::onShowCompass(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showCompass = params.get<bool>("show");

  m_sceneGraphManager->showCompass(showCompass);
}

void CommandHandler::onShowText(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showText = params.get<bool>("show");

  m_sceneGraphManager->showText(showText);
}

void CommandHandler::onShowTraps(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showTraps = params.get<bool>("show");

  m_sceneGraphManager->showTraps(showTraps);
}

void CommandHandler::onShowTrapOutlines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto showOutlines = params.get<bool>("show");

  m_sceneGraphManager->showTrapOutlines(showOutlines);
}

void CommandHandler::onShowFlowDirection(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");

  SceneGraphManager::FlowVizType type = SceneGraphManager::FlowVizNone;

  if (typeStr == "FlowVizLines")
    type = SceneGraphManager::FlowVizLines;
  else if (typeStr == "FlowVizVectors")
    type = SceneGraphManager::FlowVizVectors;

  m_sceneGraphManager->showFlowDirection(type);
}

void CommandHandler::onShowDrainageAreaOutlines(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");

  SceneGraphManager::DrainageAreaType type = SceneGraphManager::DrainageAreaNone;

  if (typeStr == "DrainageAreaFluid")
    type = SceneGraphManager::DrainageAreaFluid;
  else if (typeStr == "DrainageAreaGas")
    type = SceneGraphManager::DrainageAreaGas;

  m_sceneGraphManager->showDrainageAreaOutlines(type);
}

void CommandHandler::onSetProjection(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto typeStr = params.get<std::string>("type");
  auto mode = (typeStr == "Perspective")
    ? SceneInteractor::PERSPECTIVE
    : SceneInteractor::ORTHOGRAPHIC;

  m_examiner->setCameraMode(mode);
}

void CommandHandler::onSetCurrentSnapshot(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* /*renderArea*/,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto index = params.get<jsonxx::Number>("index");

  m_sceneGraphManager->setCurrentSnapshot((int)index);
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

void CommandHandler::onSetStillQuality(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto quality = params.get<jsonxx::Number>("quality");

  renderArea->getSettings()->setStillCompressionQuality((float)quality);
}

void CommandHandler::onSetInteractiveQuality(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto quality = params.get<jsonxx::Number>("quality");

  renderArea->getSettings()->setInteractiveCompressionQuality((float)quality);
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

void CommandHandler::onSetWidth(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto width = params.get<jsonxx::Number>("width");
  auto height = renderArea->getSceneManager()->getSize()[1];

  renderArea->resize((int)width, (int)height);
}

void CommandHandler::onSetHeight(
  const jsonxx::Object& params,
  RemoteViz::Rendering::RenderArea* renderArea,
  RemoteViz::Rendering::Connection* /*connection*/)
{
  auto width = renderArea->getSceneManager()->getSize()[0];
  auto height = params.get<jsonxx::Number>("height");

  renderArea->resize((int)width, (int)height);
}

void CommandHandler::registerHandlers()
{
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
  m_handlers["SetProperty"] = &CommandHandler::onSetProperty;
  m_handlers["SetVerticalScale"] = &CommandHandler::onSetVerticalScale;
  m_handlers["SetTransparency"] = &CommandHandler::onSetTransparency;
  m_handlers["SetRenderStyle"] = &CommandHandler::onSetRenderStyle;
  m_handlers["ShowCoordinateGrid"] = &CommandHandler::onShowCoordinateGrid;
  m_handlers["ShowCompass"] = &CommandHandler::onShowCompass;
  m_handlers["ShowText"] = &CommandHandler::onShowText;
  m_handlers["ShowTraps"] = &CommandHandler::onShowTraps;
  m_handlers["ShowTrapOutlines"] = &CommandHandler::onShowTrapOutlines;
  m_handlers["ShowFlowDirection"] = &CommandHandler::onShowFlowDirection;
  m_handlers["ShowDrainageAreaOutline"] = &CommandHandler::onShowDrainageAreaOutlines;
  m_handlers["SetProjection"] = &CommandHandler::onSetProjection;
  m_handlers["SetCurrentSnapshot"] = &CommandHandler::onSetCurrentSnapshot;
  m_handlers["ViewAll"] = &CommandHandler::onViewAll;
  m_handlers["SetViewPreset"] = &CommandHandler::onSetViewPreset;
  m_handlers["SetStillQuality"] = &CommandHandler::onSetStillQuality;
  m_handlers["SetInteractiveQuality"] = &CommandHandler::onSetInteractiveQuality;
  m_handlers["SetBandwidth"] = &CommandHandler::onSetBandwidth;
  m_handlers["SetMaxFPS"] = &CommandHandler::onSetMaxFPS;
  m_handlers["SetWidth"] = &CommandHandler::onSetWidth;
  m_handlers["SetHeight"] = &CommandHandler::onSetHeight;
}

CommandHandler::CommandHandler()
  : m_sceneGraphManager(nullptr)
  , m_examiner(nullptr)
{
}

void CommandHandler::setup(SceneGraphManager* mgr, SceneExaminer* examiner)
{
  m_sceneGraphManager = mgr;
  m_examiner = examiner;

  registerHandlers();
}

void CommandHandler::sendProjectInfo(
  RemoteViz::Rendering::RenderArea* renderArea,
  const Project::ProjectInfo& projectInfo) const
{
  jsonxx::Object msg;
  msg << "projectInfo" << toJSON(projectInfo);

  std::cout << msg.write(jsonxx::JSON) << std::endl;

  renderArea->sendMessage(msg.write(jsonxx::JSON));
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

  auto iter = m_handlers.find(cmd);
  if (iter != m_handlers.end())
  {
    auto fn = iter->second;
    ((this)->*(fn))(params, renderArea, sender);
  }
}
