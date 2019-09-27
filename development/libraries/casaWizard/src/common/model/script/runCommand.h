// Simple struct to define a run command
#pragma once

#include <QString>

namespace casaWizard
{

struct RunCommand
{
  QString command;
  QString relativeDirectory;

  RunCommand(const QString& command = "", const QString& relativeDirectory = "") :
    command{command},
    relativeDirectory{relativeDirectory}
  {
  }
};

} // namespace casaWizard
