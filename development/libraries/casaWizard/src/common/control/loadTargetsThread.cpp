//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "loadTargetsThread.h"

#include "model/input/calibrationTargetCreator.h"

namespace casaWizard
{

LoadTargetsThread::LoadTargetsThread(CalibrationTargetCreator& targetCreator, QObject* parent) :
  QThread(parent),
  calibrationTargetCreator_{targetCreator}
{
}

void LoadTargetsThread::run()
{
  calibrationTargetCreator_.createFromFile();
}

} // namespace casaWizard
