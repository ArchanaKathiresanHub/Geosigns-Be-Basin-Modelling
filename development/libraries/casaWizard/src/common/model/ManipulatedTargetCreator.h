//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QString>
#include <QVector>

#include "calibrationTarget.h"

namespace casaWizard
{

class ManipulatedTargetCreator
{
public:
   virtual QVector<CalibrationTarget> createManipulatedTargets(QVector<const CalibrationTarget*> oldTargets, const QString& property) = 0;
   virtual QString metaDataMessage(const QString& property) = 0;
};

} // namespace casaWizard
