//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller to list the calibration targets grouped by wells. Wells can be activated or deactivated.
#pragma once

#include "calibrationTargetController.h"


namespace casaWizard
{

class CalibrationTargetTable;
class CasaScenario;

class CalibrationTargetWellPrepController : public CalibrationTargetController
{
  Q_OBJECT

public:
  CalibrationTargetWellPrepController(CalibrationTargetTable* calibTable,
                                      CasaScenario& casaScenario,
                                      QObject* parent);

  const CalibrationTargetManager& calibrationTargetManager() const override;
  CalibrationTargetManager& calibrationTargetManager() override;

public slots:
  void slotDeleteSelectedWells();

};

}  // namespace casaWizard
