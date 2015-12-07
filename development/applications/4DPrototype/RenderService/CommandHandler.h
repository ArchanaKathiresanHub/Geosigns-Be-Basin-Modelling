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

#include <Visualization/Project.h>

#include <string>

class CommandHandler
{
  SceneGraphManager* m_sceneGraphManager;
  SceneExaminer* m_examiner;

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
