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

#ifndef _WIN32
#include <unistd.h> // for usleep
#endif

#include <signal.h>

#include <MeshVizXLM/mapping/MoMeshViz.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>

#ifdef USE_H264
#include <Service.h>
#include <ServiceSettings.h>
#else
#include <RemoteViz/Rendering/Service.h>
#include <RemoteViz/Rendering/ServiceSettings.h>
#endif

#include <iostream>

using namespace RemoteViz::Rendering;

bool running; 

void sleepms(unsigned int time) // milliseconds
{ 
#if defined(_WIN32)
  Sleep(time);
#else
  usleep(time * 1000);
#endif
}

void sighandler(int /*sig*/)
{
  running = false;
}
//#define COMPILE_AS_SERVICE

int main(int /*argc*/, char* /*argv*/[])
{
#ifdef COMPILE_AS_SERVICE

  RenderService service(argc, argv);
  return service.exec();

#else

  MoMeshViz::init();
  SoVolumeRendering::init();

  ServiceSettings settings;
  settings.setPort(8081);
  settings.setUsedExtensions(
    ServiceSettings::MESHVIZXLM | 
    ServiceSettings::MESHVIZ |
    ServiceSettings::VOLUMEVIZ |
    ServiceSettings::VOLUMEVIZLDM);

  auto serviceListener = std::make_shared<BpaServiceListener>();
  Service::instance()->addListener(serviceListener);

  // Open the service by using the settings
  if(Service::instance()->open(&settings))
  {
    std::cout << "IP : " << settings.getIP() << std::endl;
    std::cout << "Hostname : " << settings.getHostname() << std::endl;
    std::cout << "Port : " << settings.getPort() << std::endl;
    std::cout << "The BPA RenderService is running. Press Ctrl+C to stop." << std::endl;

    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGINT, &sighandler);

    running = true;

    while (running)
    {
      Service::instance()->dispatch();
      sleepms(1);
    }

    // Close the service
    Service::instance()->close();
  }
  else
  {
    std::cout << "Error starting service" << std::endl;
    sleepms(5000);
  }

  MoMeshViz::finish();

  return 0;

#endif
}

