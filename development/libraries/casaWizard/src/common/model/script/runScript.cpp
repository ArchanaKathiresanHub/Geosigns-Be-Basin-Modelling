#include "runScript.h"

#include <QDir>

namespace casaWizard
{

RunScript::RunScript(const QString& baseDirectory) :
  baseDirectory_{baseDirectory}
{
}

bool RunScript::killAsync() const
{
  return false;
}

void RunScript::addCommand(const QString& command, const QString& relativeDirectory)
{
  commands_.append({command, relativeDirectory});
}

QVector<RunCommand> RunScript::commands() const
{
  return commands_;
}

QString RunScript::baseDirectory() const
{
  return baseDirectory_;
}

} // namespace casaWizard
