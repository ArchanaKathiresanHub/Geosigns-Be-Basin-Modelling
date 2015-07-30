#include "RenderService.h"
#include "BpaServiceListener.h"

//#include <windows.h>
#include <signal.h>

#include <Visualization/SceneGraph.h>

#include <MeshVizInterface/mapping/MoMeshViz.h>

#include <RemoteViz/Rendering/Service.h>
#include <RemoteViz/Rendering/ServiceSettings.h>

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

void sighandler(int sig)
{
  running = false;
}
//#define COMPILE_AS_SERVICE

int main(int argc, char* argv[])
{
#ifdef COMPILE_AS_SERVICE

  RenderService service(argc, argv);
  return service.exec();

#else

  MoMeshViz::init();
  BpaVizInit();

  ServiceSettings settings;
  settings.setIP("127.0.0.1");
  settings.setPort(8081);
  settings.setUsedExtensions(ServiceSettings::MESHVIZXLM | ServiceSettings::MESHVIZ);

  std::shared_ptr<ServiceListener> serviceListener(new BpaServiceListener);
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

  BpaVizFinish();
  MoMeshViz::finish();

  return 0;

#endif
}

