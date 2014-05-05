#ifndef RENDERSERVICE_H_INCLUDED
#define RENDERSERVICE_H_INCLUDED

#include <QtService/qtservice.h>
#include <QtCore/QCoreApplication>

class RenderService : public QtService<QCoreApplication>
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
