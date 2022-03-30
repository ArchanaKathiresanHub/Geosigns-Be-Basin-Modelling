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

class CutOffTargetCreator : public ManipulatedTargetCreator
{
public:
   explicit CutOffTargetCreator(const QMap<QString, QPair<double, double>>& propertiesWithCutOffRanges);

   QVector<CalibrationTarget> createManipulatedTargets(QVector<const CalibrationTarget *> oldTargets, const QString& property) override;
   QString metaDataMessage(const QString& property) override;

private:
   int m_numberOfTargetsCutOff;
   const QMap<QString, QPair<double, double>>& m_propertiesWithCutOffRanges;
};

} // namespace casaWizard

