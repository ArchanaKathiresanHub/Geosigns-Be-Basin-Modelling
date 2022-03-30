//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "ManipulatedTargetCreator.h"

namespace casaWizard
{

class ScaledTargetCreator : public ManipulatedTargetCreator
{
public:
   explicit ScaledTargetCreator(const double scalingFactor);

   QVector<CalibrationTarget> createManipulatedTargets(QVector<const CalibrationTarget *> oldTargets, const QString&) override;
   QString metaDataMessage(const QString& property) override;

private:
   double m_scalingFactor;
};

} // namespace casaWizard

