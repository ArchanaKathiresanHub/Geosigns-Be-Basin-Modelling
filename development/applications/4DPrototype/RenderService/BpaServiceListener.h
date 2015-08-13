//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef BPASERVICELISTENER_H_INCLUDED
#define BPASERVICELISTENER_H_INCLUDED

#include "SceneExaminer.h"

#include <RemoteViz/Rendering/ServiceListener.h>

class BpaRenderAreaListener;
class SceneGraph;
class Device;
class RenderService;

using namespace RemoteViz::Rendering;

class BpaServiceListener : public ServiceListener
{
  RenderService* m_renderService;

public:

	explicit BpaServiceListener(RenderService* renderService=0);

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

#endif
