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

class CasaScenario;

class LoadTargetsThread : public QThread
{
  Q_OBJECT
public:
  LoadTargetsThread(CasaScenario& casaScenario, casaWizard::CalibrationTargetManager& calibrationTargetManager, const QString& fileName, QObject* parent = nullptr);
  void run() override;

private:
  CasaScenario& casaScenario_;
  const QString& fileName_;
  CalibrationTargetManager& calibrationTargetManager_;

};

} // namespace casaWizard

