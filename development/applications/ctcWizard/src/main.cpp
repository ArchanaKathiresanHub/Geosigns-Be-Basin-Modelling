#include "control/controller.h"

#include <QApplication>
#include <QDebug>

#include <exception>

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  ctcWizard::Controller controller;
  controller.showUI();

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

