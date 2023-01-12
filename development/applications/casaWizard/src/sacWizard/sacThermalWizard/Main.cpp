//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// SAC themal wizard user interface
#include "control/ThermalController.h"
#include "ThermalGuiTester.h"

#include <QApplication>
#include <QDebug>

#include <exception>

int main(int argc, char* argv[])
{
   QApplication a(argc, argv);
   casaWizard::sac::thermal::ThermalController controller;

   for (int i = 1; i < argc; i++)
   {
      if (std::string(argv[i]) == "-testGUI")
      {
         casaWizard::sac::thermal::ThermalGuiTester tester(controller);
         return tester.run();
      }
   }

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

