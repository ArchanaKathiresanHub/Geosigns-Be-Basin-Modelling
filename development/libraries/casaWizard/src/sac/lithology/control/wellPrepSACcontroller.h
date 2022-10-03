//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/wellPrepController.h"

#include <QObject>

namespace casaWizard
{

class CalibrationTargetWellPrepControllertroller;
class CasaScenario;
class ScriptRunController;
class WellPrepTab;

namespace sac
{

class SacLithologyScenario;

class WellPrepSACcontroller : public WellPrepController
{
  Q_OBJECT

public:
  explicit WellPrepSACcontroller(WellPrepTab* wellPrepTab,
                                 SacLithologyScenario& scenario,
                                 ScriptRunController& scriptRunController,
                                 QObject* parent);

private slots:
  void slotToSAC();

protected:
  virtual bool checkTabID(int TabID) const override;

signals:
  void switchToTab(int);

private:
  SacLithologyScenario& scenario_;
};

} // namespace sac

} // namespace casaWizard


