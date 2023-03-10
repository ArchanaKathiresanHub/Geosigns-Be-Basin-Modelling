//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Abstract base class for writing a casa script
#pragma once

#include "runCommand.h"

#include <QString>
#include <QVector>

namespace casaWizard
{

class RunScript
{
public:
  RunScript(const QString& baseDirectory);
  virtual ~RunScript() = default;

  virtual bool generateCommands() = 0;
  virtual bool killAsync() const;
  virtual bool scriptShouldCancelWhenFailureIsEncountered() const;

  void addCommand(const QString& command, const QString& relativeDirectory = "");
  QVector<RunCommand> commands() const;
  QString baseDirectory() const;

private:
  QVector<RunCommand> commands_;
  QString baseDirectory_;
};

} // namespace casaWizard
