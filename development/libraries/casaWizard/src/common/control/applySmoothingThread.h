//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QThread>

class QString;

namespace casaWizard
{

class CalibrationTargetManager;

class ApplySmoothingThread : public QThread
{
  Q_OBJECT
public:
  ApplySmoothingThread(CalibrationTargetManager& calibrationTargetManager, const double radius, const QStringList& selectedproperties, QObject* parent = nullptr);
  void run() override;

private:
  CalibrationTargetManager& calibrationTargetManager_;
  const double radius_;
  const QStringList& selectedProperties_;
};

} // namespace casaWizard

