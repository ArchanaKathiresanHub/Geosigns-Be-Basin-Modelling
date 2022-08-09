//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/TargetInputInfo.h"
#include "model/calibrationTargetManager.h"

namespace casaWizard{
namespace ua {
class TargetInputInfoFromWells : public TargetInputInfo
{


public:
   TargetInputInfoFromWells(const CalibrationTargetManager& calibrationTargetManager,
                            QVector<bool> wellSelectionStates,
                            QVector<bool> surfaceSelectionStates,
                            bool temperatureTargetsSelected = false,
                            bool vreTargetsSelected = false,
                            QString depthInput = "");

   QVector<XYName> getTargetLocations() const override;

   QVector<bool> getWellSelectionStates();
   void setWellSelectionStates(QVector<bool> states);

private:
   QVector<bool> m_wellSelectionStates;
   const CalibrationTargetManager& m_calibrationTargetManager;

};
} //namespace ua
} //namespace casaWizard
