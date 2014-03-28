#include "BpaServiceListener.h"

#include <Visualization/SceneGraph.h>

#include <MeshVizInterface/mapping/MoMeshviz.h>

#include <RemoteViz/Rendering/Service.h>
#include <RemoteViz/Rendering/ServiceSettings.h>

#include <iostream>

using namespace RemoteViz::Rendering;

int main(int argc, char* argv[])
{
  MoMeshViz::init();
  BpaVizInit();

	ServiceSettings settings;
	settings.setIP("127.0.0.1");
	settings.setPort(8080);
  settings.setUsedExtensions(ServiceSettings::MESHVIZXLM | ServiceSettings::MESHVIZ);

	BpaServiceListener serviceListener;
	Service::instance()->addListener(&serviceListener);

	// Open the service by using the settings
	Service::instance()->open(&settings);

	std::cout << "The BPA RenderService is running. Press Enter to stop." << std::endl;

	//wait, until 'Enter' is pressed.
	std::string line;
	std::getline(std::cin, line);

	// Close the service
	Service::instance()->close();

	return 0;
}

