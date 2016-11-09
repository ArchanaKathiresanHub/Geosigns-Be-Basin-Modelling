//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MainWindow.h"

#include <Interface/ObjectFactory.h>
#include <Interface/ProjectHandle.h>

#include <QtWidgets/QApplication>

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  MainWindow window;
  window.resize(1280, 960);
  window.show();

  return app.exec();
}