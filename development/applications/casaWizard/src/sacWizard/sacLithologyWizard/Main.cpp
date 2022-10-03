//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// SAC lithology wizard user interface
#include "control/controllerLithology.h"

#include <QApplication>
#include <QDebug>

#include <exception>

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  casaWizard::sac::lithology::ControllerLithology controller;

  try
  {
    return a.exec();
  }
  catch (const std::exception& error)
  {
    qDebug() << error.what();
    return 1;
  }
}

