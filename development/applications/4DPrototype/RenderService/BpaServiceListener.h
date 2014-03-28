#pragma once

#include "SceneExaminer.h"

#include <RemoteViz/Rendering/ServiceListener.h>

class BpaRenderAreaListener;
class SceneGraph;

using namespace RemoteViz::Rendering;

class BpaServiceListener : public ServiceListener
{

public:

	BpaServiceListener();

	~BpaServiceListener();

  virtual bool onPendingCreateRenderArea(
    const std::string& renderAreaId, 
    unsigned int& width, 
    unsigned int& height, 
    Device* device, 
    ConnectionParameters* parameters);

  virtual bool onPendingShareRenderArea(RenderArea* renderArea, Device* device, ConnectionParameters* parameters);

	virtual void onInstantiatedRenderArea(RenderArea *renderArea);

  virtual void onDisposedRenderArea(const std::string& renderAreaId);

  virtual void onConnectedDevice(const std::string& deviceId);

  virtual void onDisconnectedDevice(const std::string& deviceId);

  virtual void onMissingLicense(const std::string& renderAreaId, ConnectionParameters* parameters);

};

