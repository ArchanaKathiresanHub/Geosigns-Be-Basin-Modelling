//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "loadTargetsThread.h"

#include "control/importWellPopupController.h"
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
  try
  {
    calibrationTargetCreator_.createFromFile();
  }
  catch (std::runtime_error e)
  {
    emit exceptionThrown(e.what());
  }
}

} // namespace casaWizard
