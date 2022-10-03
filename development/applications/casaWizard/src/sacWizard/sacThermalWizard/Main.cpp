//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// SAC themal wizard user interface
#include "control/ControllerThermal.h"

#include <QApplication>
#include <QDebug>

#include <exception>

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  casaWizard::sac::thermal::ControllerThermal controller;

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

