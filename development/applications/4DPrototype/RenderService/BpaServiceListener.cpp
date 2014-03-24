#include "BpaServiceListener.h"
#include "BpaRenderAreaListener.h"
#include "SceneExaminer.h"

#include <Visualization/SceneGraph.h>

#include <RemoteViz/Rendering/RenderArea.h>

#include <Inventor/SoType.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/actions/SoAction.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoGradientBackground.h>
#include <Inventor/gestures/recognizers/SoScaleGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoRotateGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoLongTapGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoScaleGestureRecognizer.h>
#include <Inventor/gestures/recognizers/SoDoubleTapGestureRecognizer.h>

#include <Interface/ProjectHandle.h>

#include <iostream>

namespace di = DataAccess::Interface;

BpaServiceListener::BpaServiceListener()
  : m_sceneGraph(0)
  , m_renderAreaListener(new BpaRenderAreaListener)
{
}

BpaServiceListener::~BpaServiceListener()
{
	delete m_renderAreaListener;
}

bool BpaServiceListener::onPendingCreateRenderArea(
  const std::string& renderAreaId, 
  unsigned int& width, 
  unsigned int& height, 
  Device* device, 
  ConnectionParameters* parameters)
{
  std::cout << "Accepting connection, renderAreaId = '" << renderAreaId << "' (" << width << "x" << height << ")" << std::endl;
  return true; // accept connection
}

bool BpaServiceListener::onPendingShareRenderArea(RenderArea* renderArea, Device* device, ConnectionParameters* parameters)
{
  std::cout << "Accepting connection, shared renderAreaId = '" << renderArea->getId() << std::endl;
  return true; // accept connection
}
  
void BpaServiceListener::onInstantiatedRenderArea(RenderArea *renderArea)
{
	// Add the renderAreaListener instance as renderArea listener
	renderArea->addListener(m_renderAreaListener);

	// Add recognizers for gesture events
  SoScaleGestureRecognizer *zoomRecognizer = new SoScaleGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(zoomRecognizer);
  SoRotateGestureRecognizer* rotateRecognizer = new SoRotateGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(rotateRecognizer);
  SoDoubleTapGestureRecognizer* doubleTapRecognizer = new SoDoubleTapGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(doubleTapRecognizer);
  SoLongTapGestureRecognizer* longTapRecognizer = new SoLongTapGestureRecognizer();
  renderArea->getTouchManager()->addRecognizer(longTapRecognizer);
	
	// Instantiate a sceneExaminer to interact with the camera
  createSceneGraph();

	SceneExaminer *examiner = new SceneExaminer();
	examiner->addChild(m_sceneGraph);
	examiner->addChild(new SoGradientBackground());

	// Apply the sceneExaminer node as renderArea scene graph
	renderArea->getSceneManager()->setSceneGraph(examiner);

	// viewall
	examiner->viewAll(renderArea->getSceneManager()->getViewportRegion());
}

void BpaServiceListener::onDisposedRenderArea(const std::string& renderAreaId)
{
  std::cout << "Render area '" << renderAreaId << "' disposed" << std::endl;
}

void BpaServiceListener::onConnectedDevice(const std::string& deviceId)
{
  std::cout << "Device '" << deviceId << "' connected" << std::endl;
}

void BpaServiceListener::onDisconnectedDevice(const std::string& deviceId)
{
  std::cout << "Device '" << deviceId << "' disconnected" << std::endl;
}

void BpaServiceListener::onMissingLicense(const std::string& renderAreaId, ConnectionParameters* parameters)
{
  std::cout << "License missing (renderAreaId = '" << renderAreaId << "')" << std::endl;
}

void callback(void* data, SoAction* action)
{
  std::cout << "callback action: " << action->getTypeId().getName().getString() << std::endl;
}

void BpaServiceListener::createSceneGraph()
{
  std::cout << "Loading scenegraph..."<< std::endl;

  const char* filename = "C:/Users/Serge.Van-Der-Ree/Downloads/output_cauldron/Project.project3d";
  di::ProjectHandle* projectHandle = di::OpenCauldronProject(filename, "r");

  m_sceneGraph = new SceneGraph;
  m_sceneGraph->setup(projectHandle);
  m_sceneGraph->RenderMode = SnapshotNode::RenderMode_Skin;

  std::cout << "...done" << std::endl;
}
