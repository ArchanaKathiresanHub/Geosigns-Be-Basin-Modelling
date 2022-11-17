//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
  commands_.push_back({command, relativeDirectory});
}

QVector<RunCommand> RunScript::commands() const
{
  return commands_;
}

QString RunScript::baseDirectory() const
{
   return baseDirectory_;
}

bool RunScript::scriptShouldCancelWhenFailureIsEncountered() const
{
   return true;
}

} // namespace casaWizard
