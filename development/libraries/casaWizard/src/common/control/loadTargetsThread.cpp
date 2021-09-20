//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "loadTargetsThread.h"

#include "model/input/calibrationTargetCreator.h"
#include "model/casaScenario.h"

namespace casaWizard
{

LoadTargetsThread::LoadTargetsThread(CasaScenario& casaScenario, CalibrationTargetManager& calibrationTargetManager, const QString& fileName, QObject* parent) :
  QThread(parent),
  casaScenario_{casaScenario},
  fileName_{fileName},
  calibrationTargetManager_{calibrationTargetManager}
{
}

void LoadTargetsThread::run()
{
  CalibrationTargetCreator targetCreator(casaScenario_, calibrationTargetManager_);
  targetCreator.createFromExcel(fileName_);
}

} // namespace casaWizard
