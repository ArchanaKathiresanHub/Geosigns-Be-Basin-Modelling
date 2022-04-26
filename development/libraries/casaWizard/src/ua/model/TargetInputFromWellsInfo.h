//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVector>
#include <QString>

namespace casaWizard
{

namespace ua
{

struct TargetInputFromWellsInfo
{

   TargetInputFromWellsInfo():
      temperatureTargetsSelected(false),
      vreTargetsSelected(false)
   {}

   QVector<bool> wellSelectionStates;
   QVector<bool> surfaceSelectionStates;
   bool temperatureTargetsSelected;
   bool vreTargetsSelected;
   QString depthInput;

};

} // namespace ua

} // namespace casaWizard
