//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RenderService.h"
#include "BpaServiceListener.h"

#include <Visualization/SceneGraphManager.h>

#include <MeshVizXLM/mapping/MoMeshViz.h>

//#include <RemoteViz/Rendering/Service.h>
//#include <RemoteViz/Rendering/ServiceSettings.h>
#include <Service.h>
#include <ServiceSettings.h>

void RenderService::start()
{
  //logMessage("Starting RenderService", QtServiceBase::Information);

  MoMeshViz::init();

  ServiceSettings settings;
  //settings.setIP("127.0.0.1");
  settings.setPort(8081);
  settings.setUsedExtensions(ServiceSettings::MESHVIZXLM | ServiceSettings::MESHVIZ);

  std::shared_ptr<ServiceListener> serviceListener(new BpaServiceListener(this));
  Service::instance()->addListener(serviceListener);

  // Open the service by using the settings
  Service::instance()->open(&settings);

  //logMessage("RenderService started", QtServiceBase::Information);
}

void RenderService::stop()
{
  //logMessage("Stopping RenderService", QtServiceBase::Information);

  // Close the service
  Service::instance()->close();

  MoMeshViz::finish();
}

void RenderService::pause()
{
}

void RenderService::resume()
{
}

void RenderService::processCommand(int /*code*/)
{
}

RenderService::RenderService(int /*argc*/, char** /*argv*/)
#ifdef USE_QTSERVICE
  : QtService(argc, argv, "RenderService")
#endif
{
}

RenderService::~RenderService()
{
}
