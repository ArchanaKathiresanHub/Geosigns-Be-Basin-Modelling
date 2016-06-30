//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef JSONUTIL_H_INCLUDED
#define JSONUTIL_H_INCLUDED

#include "jsonxx.h"

#include <Project.h>
#include <Seismic.h>
#include <SceneGraphManager.h>

namespace RemoteViz
{
  namespace Rendering
  {
    class RenderAreaSettings;
  }
}

jsonxx::Object toJSON(const RemoteViz::Rendering::RenderAreaSettings& settings);

jsonxx::Object toJSON(const SeismicScene::ViewState& state);

jsonxx::Object toJSON(const SceneGraphManager::ViewState& state);

jsonxx::Object toJSON(const SceneGraphManager::PickResult& pickResult);

jsonxx::Object toJSON(const Project::ProjectInfo& projectInfo);

jsonxx::Object toJSON(const SeismicScene::Info& info);

#endif
