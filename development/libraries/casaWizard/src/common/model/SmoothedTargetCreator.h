//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "ManipulatedTargetCreator.h"

namespace casaWizard
{

class SmoothedTargetCreator : public ManipulatedTargetCreator
{
public:
   explicit SmoothedTargetCreator(const double radius);

   QVector<CalibrationTarget> createManipulatedTargets(QVector<const CalibrationTarget *> oldTargets, const QString& /*property*/) override;
   QString metaDataMessage(const QString& property) override;

private:
   double m_radius;
};

} // namespace casaWizard

