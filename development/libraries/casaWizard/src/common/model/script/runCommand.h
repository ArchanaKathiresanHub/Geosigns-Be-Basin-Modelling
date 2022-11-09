// Simple struct to define a run command
#pragma once

#include <QString>
#include <QStringList>

namespace casaWizard
{

struct RunCommand
{
  QString command;
  QStringList commandArguments;
  QString relativeDirectory;

  RunCommand(const QString& command = "", const QStringList commandArguments = {}, const QString& relativeDirectory = "") :
    command{command},
    commandArguments{commandArguments},
    relativeDirectory{relativeDirectory}
  {
  }
};

} // namespace casaWizard
