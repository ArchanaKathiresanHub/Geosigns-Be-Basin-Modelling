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

#ifdef USE_H264
#include <ServiceListener.h>
#else
#include <RemoteViz/Rendering/ServiceListener.h>
#endif

class BpaRenderAreaListener;
class SceneGraph;
class Device;
class RenderService;

using namespace RemoteViz::Rendering;

class BpaServiceListener : public ServiceListener
{
  RenderService* m_renderService;
  std::string m_datadir;

public:

  explicit BpaServiceListener(RenderService* renderService=0);

  ~BpaServiceListener();

  void setDataDir(const std::string& dir);

  bool onPendingCreateRenderArea(
    const std::string& renderAreaId, 
    unsigned int& width, 
    unsigned int& height, 
    Client* client, 
    ConnectionParameters* parameters) override;

  bool onPendingShareRenderArea(RenderArea* renderArea, Client* client, ConnectionParameters* parameters) override;

  void onInstantiatedRenderArea(RenderArea *renderArea) override;

  void onDisposedRenderArea(const std::string& renderAreaId) override;

  void onConnectedClient(const std::string& clientId) override;

  void onDisconnectedClient(const std::string& clientId) override;

  void onMissingLicense(const std::string& renderAreaId, ConnectionParameters* parameters) override;

};

#endif
