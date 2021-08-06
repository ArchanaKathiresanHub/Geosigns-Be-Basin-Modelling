//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "loadTargetsThread.h"

#include "model/input/calibrationTargetCreator.h"
#include "model/sacScenario.h"

namespace casaWizard
{

namespace sac
{

LoadTargetsThread::LoadTargetsThread(SACScenario& casaScenario, const QString& fileName, QObject* parent) :
  QThread(parent),
  casaScenario_{casaScenario},
  fileName_{fileName}
{
}

void LoadTargetsThread::run()
{
  CalibrationTargetCreator targetCreator(casaScenario_);
  targetCreator.createFromExcel(fileName_);
}

} // namespace sac

} // namespace casaWizard