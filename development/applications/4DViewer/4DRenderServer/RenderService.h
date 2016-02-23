//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef RENDERSERVICE_H_INCLUDED
#define RENDERSERVICE_H_INCLUDED

#ifdef USE_QTSERVICE
#include <QtService/qtservice.h>
#include <QtCore/QCoreApplication>
#endif

class RenderService
#ifdef USE_QTSERVICE
 : public QtService<QCoreApplication>
#endif
{
protected:

  virtual void start();

  virtual void stop();

  virtual void pause();

  virtual void resume();

  virtual void processCommand(int code);

public:

  RenderService(int argc, char** argv);

  virtual ~RenderService();
};

#endif