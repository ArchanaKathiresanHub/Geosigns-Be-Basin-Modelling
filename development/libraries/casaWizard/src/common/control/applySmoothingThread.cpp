//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "applySmoothingThread.h"
#include "model/calibrationTargetManager.h"

namespace casaWizard
{

ApplySmoothingThread::ApplySmoothingThread(CalibrationTargetManager& calibrationTargetManager,
                                           const double radius,
                                           const QStringList& selectedProperties,
                                           QObject* parent) :
  QThread(parent),
  calibrationTargetManager_{calibrationTargetManager},
  radius_(radius),
  selectedProperties_(selectedProperties)
{
}

void ApplySmoothingThread::run()
{
  calibrationTargetManager_.smoothenData(selectedProperties_, radius_);
}

} // namespace casaWizard
