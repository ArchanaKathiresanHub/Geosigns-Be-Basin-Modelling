#include "RenderService.h"
#include "BpaServiceListener.h"

#include <Visualization/SceneGraph.h>

#include <MeshVizInterface/mapping/MoMeshviz.h>

#include <RemoteViz/Rendering/Service.h>
#include <RemoteViz/Rendering/ServiceSettings.h>

void RenderService::start()
{
  logMessage("Starting RenderService", QtServiceBase::Information);

  MoMeshViz::init();
  BpaVizInit();

	ServiceSettings settings;
	//settings.setIP("127.0.0.1");
	settings.setPort(8081);
  settings.setUsedExtensions(ServiceSettings::MESHVIZXLM | ServiceSettings::MESHVIZ);

	std::tr1::shared_ptr<ServiceListener> serviceListener(new BpaServiceListener(this));
	Service::instance()->addListener(serviceListener);

	// Open the service by using the settings
	Service::instance()->open(&settings);

  logMessage("RenderService started", QtServiceBase::Information);
}

void RenderService::stop()
{
  logMessage("Stopping RenderService", QtServiceBase::Information);

	// Close the service
	Service::instance()->close();

  BpaVizFinish();
  MoMeshViz::finish();
}

void RenderService::pause()
{
}

void RenderService::resume()
{
}

void RenderService::processCommand(int code)
{
}

RenderService::RenderService(int argc, char** argv)
  : QtService(argc, argv, "RenderService")
{
}

RenderService::~RenderService()
{
}
