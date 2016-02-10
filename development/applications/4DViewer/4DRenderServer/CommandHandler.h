//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef COMMANDHANDLER_H_INCLUDED
#define COMMANDHANDLER_H_INCLUDED

class SceneGraphManager;
class SceneExaminer;

namespace RemoteViz
{
  namespace Rendering
  {
    class RenderArea;
    class Connection;
  }
}

namespace jsonxx
{
  class Object;
}

#include <Project.h>

#include <string>
#include <map>

class CommandHandler
{
  SceneGraphManager* m_sceneGraphManager;
  SceneExaminer* m_examiner;

  typedef void (CommandHandler::*FunctionPtr)(
    const jsonxx::Object&,
    RemoteViz::Rendering::RenderArea*,
    RemoteViz::Rendering::Connection*);

  std::map<std::string, FunctionPtr> m_handlers;

  void onPick(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableFormation(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableAllFormations(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableSurface(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableAllSurfaces(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableReservoir(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableAllReservoirs(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableSlice(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetSlicePosition(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableFault(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableAllFaults(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableFlowLines(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onEnableAllFlowLines(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetProperty(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetVerticalScale(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetTransparency(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetRenderStyle(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onShowCoordinateGrid(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onShowCompass(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onShowText(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onShowTraps(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onShowTrapOutlines(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onShowFluidContacts(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetFlowLinesStep(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetFlowLinesThreshold(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onShowDrainageAreaOutlines(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetProjection(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetCurrentSnapshot(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onViewAll(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetViewPreset(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetStillQuality(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetInteractiveQuality(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetBandwidth(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetMaxFPS(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetWidth(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void onSetHeight(
    const jsonxx::Object& params,
    RemoteViz::Rendering::RenderArea* renderArea,
    RemoteViz::Rendering::Connection* connection);

  void registerHandlers();

public:

  CommandHandler();
  
  void setup(SceneGraphManager* mgr, SceneExaminer* examiner);

  void sendProjectInfo(
    RemoteViz::Rendering::RenderArea* renderArea,
    const Project::ProjectInfo& projectInfo) const;
    
  void onReceivedMessage(
    RemoteViz::Rendering::RenderArea* renderArea, 
    RemoteViz::Rendering::Connection* sender, 
    const std::string& message);
};

#endif
