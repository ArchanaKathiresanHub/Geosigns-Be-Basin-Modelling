//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "saveTargetsThread.h"

#include "model/output/calibrationTargetSaver.h"

namespace casaWizard
{

SaveTargetsThread::SaveTargetsThread(const CalibrationTargetManager& calibrationTargetManager, const QString& fileName, QObject* parent) :
  QThread(parent),
  fileName_{fileName},
  calibrationTargetManager_{calibrationTargetManager}
{
}

void SaveTargetsThread::run()
{
  CalibrationTargetSaver targetSaver(calibrationTargetManager_);
  targetSaver.saveToExcel(fileName_);
}

} // namespace casaWizard
